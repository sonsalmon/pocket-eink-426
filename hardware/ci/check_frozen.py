"""Fail when design.py no longer matches the frozen routed board."""
import os
import sys

import pcbnew

GEN = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "gen"))
sys.path.insert(0, GEN)
import design as D  # noqa: E402


def mm(value):
    return pcbnew.ToMM(value)


def fail(message):
    raise SystemExit("frozen route drift: " + message)


def main(path):
    board = pcbnew.LoadBoard(path)
    expected = {part["ref"]: part for part in D.PARTS}
    actual = {fp.GetReference(): fp for fp in board.GetFootprints()}
    if set(actual) != set(expected):
        fail(
            f"references differ; missing={sorted(set(expected) - set(actual))}, "
            f"extra={sorted(set(actual) - set(expected))}"
        )

    for ref, part in expected.items():
        fp = actual[ref]
        footprint_name = str(fp.GetFPID().GetLibItemName())
        expected_name = part["fp"].split(":", 1)[1]
        if footprint_name != expected_name:
            fail(f"{ref} footprint {footprint_name!r} != {expected_name!r}")

        position = fp.GetPosition()
        x = mm(position.x) - 20.0
        y = mm(position.y) - 20.0
        rotation = fp.GetOrientationDegrees() % 360
        expected_rotation = part["rot"] % 360
        if abs(x - part["x"]) > 0.001 or abs(y - part["y"]) > 0.001:
            fail(f"{ref} position ({x:.3f}, {y:.3f}) != ({part['x']}, {part['y']})")
        if abs(rotation - expected_rotation) > 0.001:
            fail(f"{ref} rotation {rotation:.3f} != {expected_rotation}")

        actual_pads = {}
        for pad in fp.Pads():
            number = pad.GetNumber()
            if number:
                actual_pads.setdefault(number, set()).add(pad.GetNetname())
        expected_pads = {
            number: {net_name}
            for number, (_, net_name) in part["pads"].items()
        }
        if actual_pads != expected_pads:
            fail(f"{ref} pad->net differs: actual={actual_pads}, expected={expected_pads}")

    if board.GetCopperLayerCount() != 4:
        fail(f"expected 4 copper layers, found {board.GetCopperLayerCount()}")
    if abs(mm(board.GetDesignSettings().GetBoardThickness()) - 0.8) > 0.001:
        fail("board thickness is not 0.8 mm")

    module = actual["U1"]
    center = module.GetPosition()
    ep_vias = []
    for item in board.GetTracks():
        if not isinstance(item, pcbnew.PCB_VIA):
            continue
        pos = item.GetPosition()
        distance = ((mm(pos.x - center.x)) ** 2 + (mm(pos.y - center.y)) ** 2) ** 0.5
        if distance <= 3.0:
            ep_vias.append(item)
    if len(ep_vias) != 9:
        fail(f"U1 EP needs 9 thermal vias, found {len(ep_vias)}")
    for via in ep_vias:
        if via.GetNetname() != "GND":
            fail("U1 EP via is not on GND")
        if abs(mm(via.GetWidth()) - 0.7) > 0.001 or abs(mm(via.GetDrillValue()) - 0.4) > 0.001:
            fail("U1 EP via is not 0.7 mm / 0.4 mm")
        if not via.IsTented(pcbnew.F_Cu) or via.IsTented(pcbnew.B_Cu):
            fail("U1 EP via must be tented on F.Cu and exposed on B.Cu")

    print(f"frozen route matches design.py: {len(expected)} footprints, 9 exposed EP vias")


if __name__ == "__main__":
    main(sys.argv[1])
