#!/usr/bin/env python3
"""Render a UTF-8 text book and send Pocket426 protocol v1 pages."""

from __future__ import annotations

import argparse
import json
import sys
import urllib.parse
import urllib.request
import zlib
from dataclasses import dataclass
from pathlib import Path
from typing import Final

import anyio
from anyio.streams.memory import MemoryObjectReceiveStream, MemoryObjectSendStream
from bleak import BleakClient, BleakScanner
from PIL import Image, ImageDraw, ImageFont

SERVICE_UUID: Final = "7b1e0001-8f4c-4d6a-9c3e-2f5a6b7c8d90"
CONTROL_UUID: Final = "7b1e0002-8f4c-4d6a-9c3e-2f5a6b7c8d90"
DATA_UUID: Final = "7b1e0003-8f4c-4d6a-9c3e-2f5a6b7c8d90"
WIDTH: Final = 800
HEIGHT: Final = 480
FONT_CANDIDATES: Final = (
    Path("/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc"),
    Path("/usr/share/fonts/truetype/noto/NotoSansKR-Regular.ttf"),
    Path("/usr/share/fonts/truetype/nanum/NanumGothic.ttf"),
    Path("/Library/Fonts/NotoSansKR-Regular.otf"),
    Path("/Library/Fonts/NanumGothic.ttf"),
)


class BookToolError(RuntimeError):
    """A user-actionable book transfer error."""


@dataclass(frozen=True, slots=True)
class Layout:
    font_path: Path
    font_size: int
    margin: int
    line_spacing: int


@dataclass(frozen=True, slots=True)
class PackedPage:
    number: int
    image: Image.Image
    raw: bytes
    compressed: bytes


def find_font(explicit: Path | None) -> Path:
    if explicit is not None:
        if explicit.is_file():
            return explicit
        raise BookToolError(f"font not found: {explicit}")
    for candidate in FONT_CANDIDATES:
        if candidate.is_file():
            return candidate
    raise BookToolError("Noto Sans KR/Nanum font not found; pass --font PATH")


def _wrap_paragraph(
    paragraph: str, draw: ImageDraw.ImageDraw, font: ImageFont.FreeTypeFont, width: int
) -> list[str]:
    if not paragraph:
        return [""]
    lines: list[str] = []
    current = ""
    for word in paragraph.split(" "):
        candidate = word if not current else f"{current} {word}"
        if draw.textlength(candidate, font=font) <= width:
            current = candidate
            continue
        if current:
            lines.append(current)
            current = ""
        for character in word:
            candidate = current + character
            if current and draw.textlength(candidate, font=font) > width:
                lines.append(current)
                current = character
            else:
                current = candidate
    if current:
        lines.append(current)
    return lines


def render_pages(text: str, layout: Layout) -> list[Image.Image]:
    font = ImageFont.truetype(str(layout.font_path), layout.font_size)
    probe = Image.new("1", (WIDTH, HEIGHT), 1)
    draw = ImageDraw.Draw(probe)
    line_height = font.getbbox("가Ag")[3] + layout.line_spacing
    usable_width = WIDTH - 2 * layout.margin
    lines: list[str] = []
    for paragraph in text.expandtabs(4).splitlines():
        lines.extend(_wrap_paragraph(paragraph, draw, font, usable_width))
    lines_per_page = max(1, (HEIGHT - 2 * layout.margin) // line_height)
    pages: list[Image.Image] = []
    for start in range(0, max(1, len(lines)), lines_per_page):
        image = Image.new("1", (WIDTH, HEIGHT), 1)
        canvas = ImageDraw.Draw(image)
        for row, line in enumerate(lines[start : start + lines_per_page]):
            canvas.text(
                (layout.margin, layout.margin + row * line_height),
                line,
                font=font,
                fill=0,
            )
        pages.append(image)
    return pages


def pack_page(image: Image.Image) -> bytes:
    monochrome = image.convert("1")
    pixels = monochrome.load()
    packed = bytearray(WIDTH * HEIGHT // 8)
    for y in range(HEIGHT):
        row = y * (WIDTH // 8)
        for byte_x in range(WIDTH // 8):
            value = 0
            for bit in range(8):
                if pixels[byte_x * 8 + bit, y] == 0:
                    value |= 1 << (7 - bit)
            packed[row + byte_x] = value
    return bytes(packed)


def compress_page(raw: bytes) -> bytes:
    compressor = zlib.compressobj(level=9, wbits=-15)
    return compressor.compress(raw) + compressor.flush()


def prepare_pages(text: str, layout: Layout) -> list[PackedPage]:
    return [
        PackedPage(number, image, raw := pack_page(image), compress_page(raw))
        for number, image in enumerate(render_pages(text, layout))
    ]


def write_previews(pages: list[PackedPage], output: Path) -> None:
    output.mkdir(parents=True, exist_ok=True)
    for page in pages:
        page.image.save(output / f"page-{page.number:04d}.png")
        (output / f"p{page.number}.bin").write_bytes(page.compressed)


def parse_window(value: str | None, page_count: int) -> tuple[int, int]:
    if value is None:
        return 0, page_count - 1
    try:
        first_text, last_text = value.split(":", 1)
        first, last = int(first_text), int(last_text)
    except ValueError as error:
        raise BookToolError("--window must be FROM:TO") from error
    if first < 0 or last < first or last >= page_count:
        raise BookToolError("--window is outside the rendered page range")
    return first, last


async def _receive_json(
    receiver: MemoryObjectReceiveStream[dict[str, str | int | bool]],
) -> dict[str, str | int | bool]:
    while True:
        message = await receiver.receive()
        if "ev" not in message:
            return message


async def _ble_command(
    client: BleakClient,
    receiver: MemoryObjectReceiveStream[dict[str, str | int | bool]],
    command: dict[str, str | int | bool],
) -> dict[str, str | int | bool]:
    await client.write_gatt_char(
        CONTROL_UUID, json.dumps(command, ensure_ascii=False).encode(), response=True
    )
    return await _receive_json(receiver)


async def _send_ble_pages(
    client: BleakClient,
    receiver: MemoryObjectReceiveStream[dict[str, str | int | bool]],
    book_id: str,
    pages: list[PackedPage],
) -> None:
    chunk_size = max(20, client.mtu_size - 3)
    for page in pages:
        command = {
            "op": "begin_page",
            "book": book_id,
            "n": page.number,
            "len": len(page.compressed),
            "crc32": zlib.crc32(page.compressed),
        }
        await client.write_gatt_char(
            CONTROL_UUID, json.dumps(command).encode(), response=True
        )
        for offset in range(0, len(page.compressed), chunk_size):
            await client.write_gatt_char(
                DATA_UUID, page.compressed[offset : offset + chunk_size], response=False
            )
        ack = await _receive_json(receiver)
        if ack.get("ok") is not True or ack.get("n") != page.number:
            raise BookToolError(f"page {page.number} rejected: {ack}")


def _http_post(url: str, body: bytes, content_type: str) -> dict[str, str | int | bool]:
    request = urllib.request.Request(
        url, data=body, method="POST", headers={"Content-Type": content_type}
    )
    with urllib.request.urlopen(request, timeout=15) as response:  # noqa: S310
        return json.loads(response.read())


async def _send_wifi_pages(base_url: str, book_id: str, pages: list[PackedPage]) -> None:
    for page in pages:
        query = urllib.parse.urlencode(
            {
                "book": book_id,
                "n": page.number,
                "crc32": zlib.crc32(page.compressed),
            }
        )
        ack = await anyio.to_thread.run_sync(
            _http_post,
            f"{base_url}/page?{query}",
            page.compressed,
            "application/octet-stream",
        )
        if ack.get("ok") is not True or ack.get("n") != page.number:
            raise BookToolError(f"page {page.number} rejected: {ack}")


async def send_book(args: argparse.Namespace, pages: list[PackedPage]) -> None:
    device = await BleakScanner.find_device_by_name(args.device, timeout=10)
    if device is None:
        raise BookToolError(f"BLE device not found: {args.device}")
    sender, receiver = anyio.create_memory_object_stream[dict[str, str | int | bool]](20)

    def on_notify(_: object, data: bytearray) -> None:
        sender.send_nowait(json.loads(bytes(data)))

    first, last = parse_window(args.window, len(pages))
    selected = pages[first : last + 1]
    async with BleakClient(device) as client:
        await client.start_notify(CONTROL_UUID, on_notify)
        begin = {"op": "begin_book", "id": args.book_id, "title": args.title, "pages": len(pages)}
        if args.wifi:
            wifi = await _ble_command(client, receiver, {"op": "wifi", "on": True})
            print(f"Join Wi-Fi SSID {wifi['ssid']} with password {wifi['pass']}")
            await anyio.to_thread.run_sync(input, "Press Enter after joining the AP: ")
            base = f"http://{wifi['ip']}:{wifi['port']}"
            await anyio.to_thread.run_sync(
                _http_post, f"{base}/op", json.dumps(begin).encode(), "application/json"
            )
            if args.window:
                window = {"op": "window", "book": args.book_id, "from": first, "to": last}
                await anyio.to_thread.run_sync(
                    _http_post, f"{base}/op", json.dumps(window).encode(), "application/json"
                )
            await _send_wifi_pages(base, args.book_id, selected)
        else:
            await _ble_command(client, receiver, begin)
            if args.window:
                await _ble_command(
                    client,
                    receiver,
                    {"op": "window", "book": args.book_id, "from": first, "to": last},
                )
            await _send_ble_pages(client, receiver, args.book_id, selected)
        await _ble_command(client, receiver, {"op": "end_book", "id": args.book_id})
        await _ble_command(client, receiver, {"op": "open", "id": args.book_id, "page": first})


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("text", type=Path)
    parser.add_argument("--font", type=Path)
    parser.add_argument("--font-size", type=int, default=22)
    parser.add_argument("--margin", type=int, default=36)
    parser.add_argument("--line-spacing", type=int, default=6)
    parser.add_argument("--book-id", default="book")
    parser.add_argument("--title")
    parser.add_argument("--device", default="Pocket426")
    parser.add_argument("--window", metavar="FROM:TO")
    parser.add_argument("--wifi", action="store_true")
    parser.add_argument("--dry-run", type=Path)
    return parser


def main() -> int:
    args = build_parser().parse_args()
    try:
        args.title = args.title or args.text.stem
        layout = Layout(find_font(args.font), args.font_size, args.margin, args.line_spacing)
        pages = prepare_pages(args.text.read_text(encoding="utf-8"), layout)
        if args.dry_run:
            write_previews(pages, args.dry_run)
            print(f"Wrote {len(pages)} pages to {args.dry_run}")
        else:
            anyio.run(send_book, args, pages)
    except (BookToolError, OSError, UnicodeError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
