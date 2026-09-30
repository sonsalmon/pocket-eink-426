"""Summarize a kicad-cli ERC/DRC JSON report; exit 1 on any error-level item.

  python3 report.py erc|drc REPORT.json
"""
import json
import sys
from collections import Counter


def items(report, kind):
    if kind == "erc":
        for sheet in report.get("sheets", []):
            yield from sheet.get("violations", [])
    else:
        yield from report.get("violations", [])
        yield from report.get("unconnected_items", [])
        yield from report.get("schematic_parity", [])


def main(kind, path):
    with open(path) as f:
        report = json.load(f)
    found = [v for v in items(report, kind) if v.get("severity", "error") == "error"]
    by_type = Counter(v.get("type", "?") for v in found)
    print(f"{kind.upper()}: {len(found)} error(s)")
    for t, n in by_type.most_common():
        print(f"  {n:4d}  {t}")
    for v in found[:40]:
        where = "; ".join(i.get("description", "") for i in v.get("items", []))
        print(f"  - {v.get('type')}: {v.get('description')} [{where}]")
    sys.exit(1 if found else 0)


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
