from __future__ import annotations

import importlib.util
import sys
import zlib
from pathlib import Path

from PIL import ImageFont

MODULE_PATH = Path(__file__).with_name("send_book.py")
SPEC = importlib.util.spec_from_file_location("send_book", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
send_book = importlib.util.module_from_spec(SPEC)
sys.modules["send_book"] = send_book
SPEC.loader.exec_module(send_book)


def test_layout_pack_and_raw_deflate_round_trip(tmp_path: Path) -> None:
    font_path = next(
        path
        for path in (
            Path("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"),
            Path("/System/Library/Fonts/Helvetica.ttc"),
        )
        if path.is_file()
    )
    font = ImageFont.truetype(str(font_path), 22)
    assert font.path
    layout = send_book.Layout(font_path, 22, 36, 6)

    pages = send_book.prepare_pages("A deterministic page.\n" * 30, layout)
    output = tmp_path / "preview"
    send_book.write_previews(pages, output)

    assert len(pages) >= 2
    assert len(pages[0].raw) == 800 * 480 // 8
    assert zlib.decompress(pages[0].compressed, wbits=-15) == pages[0].raw
    assert (output / "page-0000.png").is_file()
    assert (output / "p0.bin").read_bytes() == pages[0].compressed


def test_protocol_bits_mark_black_pixels_as_one() -> None:
    image = send_book.Image.new("1", (800, 480), 1)
    image.putpixel((0, 0), 0)
    image.putpixel((7, 0), 0)

    packed = send_book.pack_page(image)

    assert packed[0] == 0b10000001
    assert packed[1] == 0


def test_window_parser_selects_inclusive_range() -> None:
    assert send_book.parse_window("20:150", 300) == (20, 150)
    assert send_book.parse_window(None, 300) == (0, 299)
