"""Write a KiCad schematic (label-connected, one block per part) from design.py.

  python3 build_sch.py OUT_DIR   -> OUT_DIR/pocket-eink.kicad_sch

Every pin gets a net label with the design.py net name, and single-pin nets get
a no-connect flag, so `kicad-cli sch erc` checks the same netlist the board uses.
"""
import os
import sys
import uuid

import design as D

NAME = "pocket-eink"
G = 2.54
NS = uuid.UUID("7d3b2f64-3c1e-4c55-9a8e-2b8f5e0d9a11")
ROOT = str(uuid.uuid5(NS, "root"))
SECTION_BREAKS = {"U1": "ESP32-C3 module", "TP1": "Protected battery", "U4": "Charger",
                  "U5": "3V3 LDO", "J2": "E-paper FPC + charge pump",
                  "R19": "Buttons", "J3": "USB-C", "TP3": "Test pads"}


def uid(*parts):
    return str(uuid.uuid5(NS, "/".join(parts)))


def q(s):
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"') + '"'


def font(size=1.0):
    return f"(effects (font (size {size} {size})))"


def pin_rows(p):
    pads = sorted(p["pads"].items(), key=lambda kv: (len(kv[0]), kv[0]))
    left = pads[: (len(pads) + 1) // 2]
    right = pads[(len(pads) + 1) // 2:]
    return left, right


def body_width(p):
    longest = max((len(name) for name, _ in p["pads"].values()), default=2)
    return max(4, (longest * 2 + 7) // 5 * 2) * G / 2


def lib_symbol(p):
    left, right = pin_rows(p)
    rows = max(len(left), len(right))
    w = body_width(p)
    top = (rows - 1) * G / 2 + G
    out = [f"(symbol {q('pocket:' + p['ref'])} (in_bom yes) (on_board yes)",
           f"(property \"Reference\" {q(p['ref'][0])} (at 0 {top + 1.27:.2f} 0) {font()})",
           f"(property \"Value\" {q(p['value'])} (at 0 {-top - 1.27:.2f} 0) {font()})",
           f"(symbol {q(p['ref'] + '_0_1')}",
           f"(rectangle (start {-w:.2f} {top:.2f}) (end {w:.2f} {-top:.2f}) "
           "(stroke (width 0.254) (type default)) (fill (type background)))",
           ")", f"(symbol {q(p['ref'] + '_1_1')}"]
    for side, pads in (("L", left), ("R", right)):
        for i, (num, (name, _)) in enumerate(pads):
            y = (rows - 1) * G / 2 - i * G
            x, ang = (-w - G, 0) if side == "L" else (w + G, 180)
            out.append(f"(pin passive line (at {x:.2f} {y:.2f} {ang}) (length {G}) "
                       f"(name {q(name)} {font()}) (number {q(num)} {font()}))")
    out += [")", ")"]
    return "\n".join(out), rows


def pin_points(p, ox, oy):
    """Sheet coordinates of each pin's connection point (schematic y grows down)."""
    left, right = pin_rows(p)
    rows = max(len(left), len(right))
    w = body_width(p)
    pts = []
    for side, pads in (("L", left), ("R", right)):
        for i, (num, (_, net)) in enumerate(pads):
            y = (rows - 1) * G / 2 - i * G
            x = -w - G if side == "L" else w + G
            pts.append((num, net, side, ox + x, oy - y))
    return pts


def build(out_dir):
    os.makedirs(out_dir, exist_ok=True)
    single = set(D.single_pad_nets())
    libs, body = [], []
    col_x, y, col, col_w, max_y = 30 * G, 20 * G, 0, 44 * G, 300 * G
    for p in D.PARTS:
        sym, rows = lib_symbol(p)
        libs.append(sym)
        h = (rows + 3) * G
        if p["ref"] in SECTION_BREAKS or y + h > max_y:
            if y > 20 * G:
                col += 1
                y = 20 * G
            if p["ref"] in SECTION_BREAKS:
                body.append(f"(text {q(SECTION_BREAKS[p['ref']])} (exclude_from_sim no) "
                            f"(at {col_x + col * col_w - 10 * G:.2f} {y - 2 * G:.2f} 0) "
                            f"(effects (font (size 2 2) (bold yes)) (justify left)) (uuid {uid('t', p['ref'])}))")
        ox = col_x + col * col_w
        oy = y + (rows - 1) * G / 2 + G
        y += h
        ref, sid = p["ref"], uid("s", p["ref"])
        props = [("Reference", ref), ("Value", p["value"]), ("Footprint", p["fp"]),
                 ("Datasheet", ""), ("LCSC", p["lcsc"]), ("Description", p["desc"])]
        s = [f"(symbol (lib_id {q('pocket:' + ref)}) (at {ox:.2f} {oy:.2f} 0) (unit 1) "
             f"(exclude_from_sim no) (in_bom {'no' if p['dnp'] else 'yes'}) (on_board yes) "
             f"(dnp {'yes' if p['dnp'] else 'no'}) (uuid {sid})"]
        top = (rows - 1) * G / 2 + G
        for i, (k, v) in enumerate(props):
            hide = " (hide yes)" if i >= 2 else ""
            yy = oy - top - 1.27 if k == "Reference" else oy + top + 1.27 + (i - 1) * 1.6
            s.append(f"(property {q(k)} {q(v)} (at {ox:.2f} {yy:.2f} 0) "
                     f"(effects (font (size 1 1)){hide}))")
        for num in p["pads"]:
            s.append(f"(pin {q(num)} (uuid {uid('p', ref, num)}))")
        s.append(f"(instances (project {q(NAME)} (path \"/{ROOT}\" (reference {q(ref)}) (unit 1))))")
        s.append(")")
        body.append("\n".join(s))
        for num, net, side, px, py in pin_points(p, ox, oy):
            if net in single:
                body.append(f"(no_connect (at {px:.2f} {py:.2f}) (uuid {uid('nc', ref, num)}))")
                continue
            ang, just = (180, "right") if side == "L" else (0, "left")
            body.append(f"(label {q(net)} (at {px:.2f} {py:.2f} {ang}) "
                        f"(effects (font (size 1 1)) (justify {just})) (uuid {uid('l', ref, num)}))")

    text = "\n".join([
        "(kicad_sch (version 20231120) (generator \"pocket_gen\") (generator_version \"1.0\")",
        f"(uuid {ROOT}) (paper \"A0\")",
        "(title_block (title \"Pocket E-Ink 4.26 main board\") (rev \"B\") "
        "(comment 1 \"Generated from hardware/gen/design.py - edit that file, not this one\"))",
        "(lib_symbols", *libs, ")", *body,
        "(sheet_instances (path \"/\" (page \"1\")))", ")"])
    path = os.path.join(out_dir, f"{NAME}.kicad_sch")
    with open(path, "w") as f:
        f.write(text + "\n")
    print(f"wrote {path}: {len(D.PARTS)} symbols")


if __name__ == "__main__":
    build(sys.argv[1])
