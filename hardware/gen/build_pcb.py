"""Build the KiCad board from design.py (needs KiCad 9's pcbnew Python module).

  python3 build_pcb.py place  OUT_DIR   -> OUT_DIR/pocket-eink.kicad_pcb + .kicad_pro + .dsn
  python3 build_pcb.py import OUT_DIR   -> reads OUT_DIR/pocket-eink.ses, fills zones, saves

Routing between the two steps is done by Freerouting (see .github/workflows/hardware.yml).
"""
import json
import os
import sys

import pcbnew

import design as D

NAME = "pocket-eink"
ORIGIN = (20.0, 20.0)  # board top-left on the KiCad sheet, mm
HERE = os.path.dirname(os.path.abspath(__file__))
POCKET_LIB = os.path.join(HERE, "..", "lib", "pocket.pretty")
SYS_FP = os.environ.get("KICAD9_FOOTPRINT_DIR", "/usr/share/kicad/footprints")


def mm(v):
    return pcbnew.FromMM(v)


def pt(x, y):
    return pcbnew.VECTOR2I(mm(ORIGIN[0] + x), mm(ORIGIN[1] + y))


def load_fp(spec):
    lib, name = spec.split(":", 1)
    path = POCKET_LIB if lib == D.LIB_POCKET else os.path.join(SYS_FP, lib + ".pretty")
    fp = pcbnew.FootprintLoad(path, name)
    if fp is None:
        raise SystemExit(f"footprint not found: {spec} in {path}")
    return fp


def add_outline(board):
    corners = [(0, 0), (D.BOARD_W, 0), (D.BOARD_W, D.BOARD_H), (0, D.BOARD_H)]
    for (x0, y0), (x1, y1) in zip(corners, corners[1:] + corners[:1]):
        seg = pcbnew.PCB_SHAPE(board)
        seg.SetShape(pcbnew.SHAPE_T_SEGMENT)
        seg.SetStart(pt(x0, y0))
        seg.SetEnd(pt(x1, y1))
        seg.SetLayer(pcbnew.Edge_Cuts)
        seg.SetWidth(mm(0.1))
        board.Add(seg)


def rect_outline(zone, x0, y0, x1, y1):
    outline = zone.Outline()
    outline.NewOutline()
    for x, y in [(x0, y0), (x1, y0), (x1, y1), (x0, y1)]:
        p = pt(x, y)
        outline.Append(p.x, p.y)


def add_zone(board, net, layer, priority=0):
    z = pcbnew.ZONE(board)
    z.SetLayer(layer)
    z.SetNet(net)
    z.SetAssignedPriority(priority)
    z.SetLocalClearance(mm(0.2))
    z.SetMinThickness(mm(0.2))
    z.SetPadConnection(pcbnew.ZONE_CONNECTION_FULL)
    z.SetIslandRemovalMode(pcbnew.ISLAND_REMOVAL_MODE_ALWAYS)
    rect_outline(z, 0.3, 0.3, D.BOARD_W - 0.3, D.BOARD_H - 0.3)
    board.Add(z)


def add_antenna_keepout(board):
    z = pcbnew.ZONE(board)
    z.SetIsRuleArea(True)
    z.SetLayerSet(pcbnew.LSET.AllCuMask())
    (getattr(z, "SetDoNotAllowZoneFills", None) or z.SetDoNotAllowCopperPour)(True)
    z.SetDoNotAllowVias(True)
    z.SetDoNotAllowTracks(False)  # the feed trace has to reach the antenna pad
    z.SetDoNotAllowPads(False)
    z.SetDoNotAllowFootprints(False)
    rect_outline(z, *D.ANT_KEEPOUT)
    board.Add(z)


def write_project(out_dir):
    classes = []
    for name, (width, clearance) in D.NETCLASSES.items():
        classes.append({
            "name": name, "clearance": clearance, "track_width": width,
            "via_diameter": 0.5, "via_drill": 0.3,
            "microvia_diameter": 0.3, "microvia_drill": 0.1,
            "diff_pair_width": 0.2, "diff_pair_gap": 0.2, "diff_pair_via_gap": 0.25,
            "wire_width": 6, "bus_width": 12, "line_style": 0, "pcb_color": "rgba(0, 0, 0, 0.000)",
            "schematic_color": "rgba(0, 0, 0, 0.000)", "priority": 2147483647 if name == "Default" else 0,
        })
    patterns = [{"netclass": cls, "pattern": net} for net, cls in D.NET_CLASS_OF.items()]
    project = {
        "board": {"design_settings": {
            "defaults": {"board_outline_line_width": 0.1, "copper_line_width": 0.2},
            "rules": {
                "min_clearance": 0.127, "min_track_width": 0.127, "min_via_diameter": 0.45,
                "min_via_annular_width": 0.1, "min_through_hole_diameter": 0.25,
                "min_hole_to_hole": 0.25, "min_copper_edge_clearance": 0.3,
                "min_silk_clearance": 0.0, "min_hole_clearance": 0.25,
            },
            "track_widths": [0.15, 0.25, 0.35],
            "via_dimensions": [{"diameter": 0.5, "drill": 0.3}],
        }},
        "net_settings": {"classes": classes, "meta": {"version": 3}, "netclass_patterns": patterns},
        "meta": {"filename": f"{NAME}.kicad_pro", "version": 1},
    }
    with open(os.path.join(out_dir, f"{NAME}.kicad_pro"), "w") as f:
        json.dump(project, f, indent=2)


def place(out_dir):
    os.makedirs(out_dir, exist_ok=True)
    board = pcbnew.BOARD()
    board.SetCopperLayerCount(4)
    board.GetDesignSettings().SetBoardThickness(mm(0.8))

    netinfo = {}
    for name in D.nets():
        ni = pcbnew.NETINFO_ITEM(board, name)
        board.Add(ni)
        netinfo[name] = ni

    problems = []
    for p in D.PARTS:
        fp = load_fp(p["fp"])
        fp.SetReference(p["ref"])
        fp.SetValue(p["value"])
        fp.SetPosition(pt(p["x"], p["y"]))
        fp.SetOrientationDegrees(p["rot"])
        if p["dnp"]:
            fp.SetDNP(True)
            fp.SetExcludedFromBOM(True)
            fp.SetExcludedFromPosFiles(True)
        fp.Reference().SetTextSize(pcbnew.VECTOR2I(mm(0.5), mm(0.5)))
        fp.Reference().SetTextThickness(mm(0.08))
        fp.Reference().SetVisible(p["ref"].startswith(("J", "SW", "TP", "U")))
        fp.Value().SetVisible(False)
        seen = set()
        for pad in fp.Pads():
            num = pad.GetNumber()
            if not num:
                continue
            if num not in p["pads"]:
                problems.append(f"{p['ref']} pad {num} has no net in design.py")
                continue
            pad.SetNet(netinfo[p["pads"][num][1]])
            seen.add(num)
        for num in p["pads"]:
            if num not in seen:
                problems.append(f"{p['ref']} pad {num} is in design.py but not in the footprint")
        board.Add(fp)
    if problems:
        raise SystemExit("\n".join(problems))

    add_outline(board)
    add_antenna_keepout(board)
    # Only the In1 GND plane goes to the router (GND pads get vias to it); the
    # other pours are added after routing so they cannot fragment into islands.
    add_zone(board, netinfo["GND"], pcbnew.In1_Cu)

    pcb_path = os.path.join(out_dir, f"{NAME}.kicad_pcb")
    pcbnew.SaveBoard(pcb_path, board)
    # SaveBoard writes a default .kicad_pro; overwrite it, then reload so the
    # net classes and rules are attached before the DSN export.
    write_project(out_dir)
    board = pcbnew.LoadBoard(pcb_path)
    pcbnew.ZONE_FILLER(board).Fill(board.Zones())
    if not pcbnew.ExportSpecctraDSN(board, os.path.join(out_dir, f"{NAME}.dsn")):
        raise SystemExit("Specctra DSN export failed")
    pcbnew.SaveBoard(pcb_path, board)
    print(f"placed {len(D.PARTS)} parts, {len(netinfo)} nets -> {pcb_path}")


def import_ses(out_dir):
    pcb_path = os.path.join(out_dir, f"{NAME}.kicad_pcb")
    board = pcbnew.LoadBoard(pcb_path)
    if not pcbnew.ImportSpecctraSES(board, os.path.join(out_dir, f"{NAME}.ses")):
        raise SystemExit("Specctra SES import failed")
    gnd = board.FindNet("GND")
    for layer in (pcbnew.F_Cu, pcbnew.In2_Cu, pcbnew.B_Cu):
        add_zone(board, gnd, layer)
    pcbnew.ZONE_FILLER(board).Fill(board.Zones())
    pcbnew.SaveBoard(pcb_path, board)
    tracks = sum(1 for t in board.GetTracks())
    print(f"imported routes: {tracks} track/via items")


if __name__ == "__main__":
    step, out = sys.argv[1], sys.argv[2]
    {"place": place, "import": import_ses}[step](out)
