"""JLCPCB assembly files from design.py and the KiCad position export.

  python3 gen_jlc.py POS_CSV OUT_DIR   -> OUT_DIR/bom-jlc.csv, OUT_DIR/cpl-jlc.csv
"""
import csv
import os
import sys
from collections import OrderedDict

import design as D


def main(pos_csv, out_dir):
    placed = {p["ref"]: p for p in D.PARTS if not p["dnp"] and p["lcsc"]}
    groups = OrderedDict()
    for p in placed.values():
        key = (p["value"], p["fp"].split(":", 1)[1], p["lcsc"])
        groups.setdefault(key, []).append(p["ref"])
    with open(os.path.join(out_dir, "bom-jlc.csv"), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["Comment", "Designator", "Footprint", "LCSC Part #"])
        for (value, fp, lcsc), refs in groups.items():
            w.writerow([value, ",".join(refs), fp, lcsc])

    rows = []
    with open(pos_csv, newline="") as f:
        for r in csv.DictReader(f):
            if r["Ref"] in placed:
                rows.append([r["Ref"], f"{float(r['PosX']):.3f}mm", f"{float(r['PosY']):.3f}mm",
                             "Top" if r["Side"].lower().startswith("top") else "Bottom", r["Rot"]])
    missing = sorted(set(placed) - {r[0] for r in rows})
    if missing:
        raise SystemExit("parts missing from the position file: " + ", ".join(missing))
    with open(os.path.join(out_dir, "cpl-jlc.csv"), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["Designator", "Mid X", "Mid Y", "Layer", "Rotation"])
        w.writerows(rows)
    print(f"BOM lines: {len(groups)}, placements: {len(rows)}")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
