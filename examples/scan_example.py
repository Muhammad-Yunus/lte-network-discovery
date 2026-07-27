#!/usr/bin/env python3
"""
Quick example: scan Band 8 and print operator info.

Usage:
    python3 scan_example.py
"""

import sys
sys.path.insert(0, ".")
from lte_discover import scan, lookup_operator

# Method 1: Full scan (needs SDR)
print("=== Full Scan ===")
try:
    cells = scan(band=8, gain=42)
    for c in cells:
        print(f"  EARFCN {c.earfcn} | PCI {c.pci} | {c.operator} | PLMN {c.plmn}")
except RuntimeError as e:
    print(f"  SDR not available: {e}")

# Method 2: Operator lookup only (no SDR needed)
print("\n=== Operator Lookup (no SDR) ===")
for earfcn in [3500, 3550, 3650, 3700]:
    op = lookup_operator(earfcn)
    if op:
        print(f"  EARFCN {earfcn} → {op['operator']} (MCC {op['mcc']}, MNC {op['mnc']})")
    else:
        print(f"  EARFCN {earfcn} → Unknown")
