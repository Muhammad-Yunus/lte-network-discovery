"""
LTE Network Discovery - Python wrapper

Usage:
    from lte_discover import scan
    
    results = scan(band=8)
    for cell in results:
        print(f"{cell['operator']}: MCC={cell['mcc']}, MNC={cell['mnc']}")
"""

import subprocess
import json
import shutil
from pathlib import Path
from dataclasses import dataclass
from typing import List, Optional

CLI_PATH = shutil.which("lte_scan_example") or str(
    Path(__file__).parent.parent / "build" / "lte-network-discovery" / "lte_scan_example"
)


@dataclass
class Cell:
    earfcn: int
    freq_mhz: float
    pci: int
    nof_prb: int
    nof_ports: int
    rsrp_dbm: float
    operator: str
    mcc: int
    mnc: int
    mnc_3digit: bool
    plmn: str
    source: str
    tac: Optional[int] = None
    cell_id: Optional[int] = None
    cfo_hz: float = 0.0


def scan(
    band: int = 8,
    gain: float = 42.0,
    earfcn_start: int = None,
    earfcn_end: int = None,
    timeout: int = 300,
) -> List[Cell]:
    """
    Scan LTE band and return detected cells.
    
    Args:
        band: LTE band number (3, 5, 8, 28, 40)
        gain: RX gain in dB
        earfcn_start: Start EARFCN (optional)
        earfcn_end: End EARFCN (optional)
        timeout: Max seconds to wait
    
    Returns:
        List of Cell objects
    """
    cmd = [CLI_PATH, "-b", str(band), "-g", str(gain), "-j", "-q"]
    if earfcn_start is not None:
        cmd.extend(["-s", str(earfcn_start)])
    if earfcn_end is not None:
        cmd.extend(["-e", str(earfcn_end)])

    result = subprocess.run(
        cmd, capture_output=True, text=True, timeout=timeout
    )

    if result.returncode == 2:
        raise RuntimeError(f"SDR error: {result.stderr.strip()}")

    try:
        data = json.loads(result.stdout)
    except json.JSONDecodeError:
        raise RuntimeError(f"Parse error: {result.stdout}{result.stderr}")

    cells = []
    for c in data.get("cells", []):
        cells.append(Cell(
            earfcn=c["earfcn"],
            freq_mhz=c["freq_mhz"],
            pci=c["pci"],
            nof_prb=c["nof_prb"],
            nof_ports=c["nof_ports"],
            rsrp_dbm=c["rsrp_dbm"],
            operator=c["operator"],
            mcc=c["mcc"],
            mnc=c["mnc"],
            mnc_3digit=c["mnc_3digit"],
            plmn=c["plmn"],
            source=c["source"],
            tac=c.get("tac"),
            cell_id=c.get("cell_id"),
            cfo_hz=c.get("cfo_hz", 0.0),
        ))
    return cells


def lookup_operator(earfcn: int) -> Optional[dict]:
    """
    Look up operator from EARFCN (no SDR needed).
    
    Returns dict with operator, mcc, mnc or None if not found.
    """
    # Import the operator table from the C library
    # This is a fallback - parses the table from lte_scan.h
    table = {
        (1200, 1399): ("Telkomsel", 510, 10),
        (1400, 1499): ("XL Axiata", 510, 11),
        (1500, 1599): ("XL Axiata (AXIS)", 510, 11),
        (1600, 1799): ("Indosat Ooredoo", 510, 21),
        (1800, 1949): ("Hutchison 3", 510, 89),
        (2400, 2499): ("Telkomsel", 510, 10),
        (2500, 2649): ("Smartfren", 510, 9),
        (3450, 3549): ("Telkomsel", 510, 10),
        (3550, 3649): ("XL Axiata", 510, 11),
        (3650, 3699): ("Indosat Ooredoo", 510, 21),
        (3700, 3799): ("Hutchison 3", 510, 89),
        (9000, 9149): ("Telkomsel", 510, 10),
        (9150, 9299): ("Indosat Ooredoo", 510, 21),
        (9300, 9449): ("XL Axiata", 510, 11),
        (9450, 9599): ("Hutchison 3", 510, 89),
        (38650, 38799): ("Telkomsel", 510, 10),
        (38800, 38949): ("XL Axiata", 510, 11),
        (38950, 39099): ("Indosat Ooredoo", 510, 21),
        (39100, 39249): ("Hutchison 3", 510, 89),
        (39250, 39649): ("Smartfren", 510, 9),
    }
    for (lo, hi), (op, mcc, mnc) in table.items():
        if lo <= earfcn <= hi:
            return {"operator": op, "mcc": mcc, "mnc": mnc, "earfcn": earfcn}
    return None
