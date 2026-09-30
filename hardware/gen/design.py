"""Single source of truth for the Pocket E-Ink 4.26 main board.

Every part lists: reference, footprint (lib:name), value, LCSC part number,
placement (x, y in mm from the board's top-left corner, rotation in degrees,
KiCad convention: positive = counter-clockwise on screen) and a pad map
pad -> (pin name, net). build_pcb.py and build_sch.py both read this file,
so the schematic and the board can never drift apart.

Board frame (portrait, seen from the front / panel side, y grows downward):
  y 0.0 .. 26.3  : under the e-paper panel (parts must stay <= 1.6 mm tall)
  y 26.3 .. 45.0 : strip below the panel (battery/charger/LDO, 3 buttons, USB-C)
Rev B uses an ESP32-C3-WROOM-02-N4 module and keeps the Rev A GPIO assignment.
"""

BOARD_W = 62.0
BOARD_H = 45.0
PANEL_EDGE_Y = 26.3  # bottom edge of the GDEY0426T82 active glass over the PCB
# The panel's FPC (about 24 mm past the glass, per the Waveshare outline of
# 129.33 mm vs 105.33 mm glass) folds 180 degrees behind the panel and runs back
# up to this connector opening. Adjust after measuring a real panel.
FPC_OPENING_Y = 10.0

# Module antenna keep-out (no copper on any layer): left board edge.
ANT_KEEPOUT = (0.0, 26.2, 6.2, 44.3)  # x0, y0, x1, y1

# Net classes: name -> (track width, clearance) in mm
NETCLASSES = {
    "Default": (0.127, 0.127),
    "Power": (0.25, 0.15),
    "HV": (0.25, 0.15),  # e-paper gate/source rails, up to about +-22 V; FPC pads sit 0.2 mm apart
}
NET_CLASS_OF = {
    "GND": "Power", "+3V3": "Power", "VBUS": "Power", "VBAT": "Power",
    "PREVGH": "HV", "PREVGL": "HV", "VSH1": "HV", "VSH2": "HV", "VSL": "HV",
    "VCOM": "HV", "EPD_SW": "HV", "EPD_NEG": "HV", "GDR": "HV",
}

LIB_POCKET = "pocket"  # hardware/lib/pocket.pretty (footprints from EasyEDA/LCSC)

C0603 = "Capacitor_SMD:C_0603_1608Metric"
C0805 = "Capacitor_SMD:C_0805_2012Metric"
R0603 = "Resistor_SMD:R_0603_1608Metric"


def two(net1, net2, n1="1", n2="2"):
    return {"1": (n1, net1), "2": (n2, net2)}


PARTS = []


def part(ref, fp, value, lcsc, x, y, rot=0, pads=None, dnp=False, desc=""):
    PARTS.append(dict(ref=ref, fp=fp, value=value, lcsc=lcsc, x=x, y=y, rot=rot,
                      pads=pads or {}, dnp=dnp, desc=desc))


# ---------------------------------------------------------------- MCU ------
ESP_PINS = {
    "1": ("3V3", "+3V3"), "2": ("EN", "EN"),
    "3": ("IO4", "EPD_DC"), "4": ("IO5", "EPD_RST"), "5": ("IO6", "EPD_BUSY"),
    "6": ("IO7", "GPIO7_NC"), "7": ("IO8", "SPI_SCLK"), "8": ("IO9", "BOOT"),
    "9": ("GND", "GND"), "10": ("IO10", "SPI_MOSI"),
    "11": ("RXD/IO20", "USB_DET"), "12": ("TXD/IO21", "EPD_CS"),
    "13": ("IO18/USB_D-", "USB_DM"), "14": ("IO19/USB_D+", "USB_DP"),
    "15": ("IO3", "PWR_BTN"), "16": ("IO2", "GPIO2"),
    "17": ("IO1", "BTN_ADC"), "18": ("IO0", "BAT_SENSE"), "19": ("EP", "GND"),
}
part("U1", f"{LIB_POCKET}:ESP32-C3-WROOM-02-N4", "ESP32-C3-WROOM-02-N4", "C2934560",
     13.1, 35.2, 90, ESP_PINS, desc="Wi-Fi/BLE module with 4 MB flash and PCB antenna")
# Supply and boot support at the module's board-facing side.
part("C5", C0603, "10uF", "C19702", 27.0, 24.0, 90, two("+3V3", "GND"))
part("C6", C0603, "100nF", "C14663", 29.0, 24.0, 90, two("+3V3", "GND"))
part("R3", R0603, "10k", "C25804", 31.0, 24.0, 90, two("+3V3", "EN"))
part("C12", C0603, "1uF", "C15849", 33.0, 24.0, 90, two("EN", "GND"), desc="EN RC delay")
part("R4", R0603, "10k", "C25804", 35.0, 24.0, 90, two("+3V3", "GPIO2"), desc="GPIO2 strap high")
part("R5", R0603, "10k", "C25804", 37.0, 24.0, 90, two("+3V3", "SPI_SCLK"), desc="GPIO8 strap high")

# ------------------------------------------------------------ battery ------
part("TP1", "TestPoint:TestPoint_Pad_1.0x1.0mm", "BAT+", "", 2.5, 21.5, 0,
     {"1": ("BAT+", "VBAT")}, desc="battery red wire")
part("TP2", "TestPoint:TestPoint_Pad_1.0x1.0mm", "BAT-", "", 2.5, 24.5, 0,
     {"1": ("BAT-", "GND")}, desc="protected battery black wire")
part("C14", C0603, "10uF", "C19702", 4.8, 22.0, 90, two("VBAT", "GND"))
# Battery sense 1M/1M (~2 uA) into GPIO0
part("R8", R0603, "1M", "C22935", 7.0, 22.0, 90, two("VBAT", "BAT_SENSE"))
part("R9", R0603, "1M", "C22935", 9.0, 22.0, 90, two("BAT_SENSE", "GND"))
part("C15", C0603, "100nF", "C14663", 11.0, 22.0, 90, two("BAT_SENSE", "GND"))

# ------------------------------------------------------------ charger ------
part("U4", "Package_TO_SOT_SMD:SOT-23-5", "MCP73831-2", "C424093", 7.5, 17.0, 0, {
    "1": ("STAT", "CHG_STAT"), "2": ("VSS", "GND"), "3": ("VBAT", "VBAT"),
    "4": ("VDD", "VBUS"), "5": ("PROG", "CHG_PROG")}, desc="LiPo charger, 256 mA")
part("R10", R0603, "3.9k", "C22980", 7.5, 19.5, 0, two("CHG_PROG", "GND"), desc="Ichg = 1000/3.9k")
part("C16", C0603, "4.7uF", "C19666", 4.5, 17.0, 90, two("VBUS", "GND"))
part("C17", C0603, "4.7uF", "C19666", 10.5, 17.0, 90, two("VBAT", "GND"))
part("D1", "LED_SMD:LED_0603_1608Metric", "RED", "C2286", 13.0, 18.5, 0,
     {"1": ("K", "LED_K"), "2": ("A", "VBUS")}, desc="charging indicator")
part("R11", R0603, "1k", "C21190", 16.2, 18.5, 0, two("LED_K", "CHG_STAT"))

# ----------------------------------------------------------- 3V3 LDO -------
part("U5", "Package_TO_SOT_SMD:SOT-23-5", "RT9080-33GJ5", "C841192", 15.5, 14.0, 0, {
    "1": ("VIN", "VBAT"), "2": ("GND", "GND"), "3": ("EN", "VBAT"), "4": ("NC", "LDO_NC"),
    "5": ("VOUT", "+3V3")}, desc="600 mA, 2 uA Iq")
part("C18", C0603, "1uF", "C15849", 11.8, 14.0, 90, two("VBAT", "GND"))
part("C19", C0603, "10uF", "C19702", 19.5, 11.0, 90, two("+3V3", "GND"))

# --------------------------------------------- e-paper FPC + charge pump ---
# 24-pin Good Display interface, SSD1677 application circuit (same netlist as
# the IS7V4N reference board and the Xteink X4 family).
FPC_PINS = {
    "1": ("NC", "FPC_NC1"), "2": ("GDR", "GDR"), "3": ("RESE", "RESE"), "4": ("NC", "FPC_NC4"),
    "5": ("VSH2", "VSH2"), "6": ("TSCL", "FPC_NC6"), "7": ("TSDA", "FPC_NC7"), "8": ("BS1", "GND"),
    "9": ("BUSY", "EPD_BUSY"), "10": ("RES#", "EPD_RST"), "11": ("D/C#", "EPD_DC"),
    "12": ("CS#", "EPD_CS"), "13": ("SCL", "SPI_SCLK"), "14": ("SDA", "SPI_MOSI"),
    "15": ("VDDIO", "+3V3"), "16": ("VCI", "+3V3"), "17": ("VSS", "GND"), "18": ("VDD", "EPD_VDD"),
    "19": ("VPP", "FPC_NC19"), "20": ("VSH1", "VSH1"), "21": ("VGH", "PREVGH"),
    "22": ("VSL", "VSL"), "23": ("VGL", "PREVGL"), "24": ("VCOM", "VCOM"),
    "MP": ("MOUNT", "GND"),
}
FPC_BODY_FRONT = 4.9  # footprint local +y extent (insertion side)
part("J2", "Connector_FFC-FPC:Hirose_FH12-24S-0.5SH_1x24-1MP_P0.50mm_Horizontal", "FH12-24S-0.5SH",
     "C202112", 31.0, FPC_OPENING_Y - FPC_BODY_FRONT, 0, FPC_PINS,
     desc="under the panel, opening toward the panel's FPC edge; verify contact side and pin 1")
part("L1", f"{LIB_POCKET}:IND-SMD_L4.0-W4.0_FNR4012S", "47uH", "C167794", 21, 17, 0,
     two("+3V3", "EPD_SW"), desc="boost inductor, 1.2 mm tall")
part("C32", C0603, "4.7uF", "C19666", 20.5, 20.7, 90, two("+3V3", "GND"),
     desc="boost input bulk, next to L1 pin 1")
part("Q3", "Package_TO_SOT_SMD:SOT-323_SC-70", "Si1308EDL", "C469327", 25.2, 16, 0, {
    "1": ("G", "GDR"), "2": ("S", "RESE"), "3": ("D", "EPD_SW")})
part("R17", R0603, "2.2R", "C22939", 25.2, 18.6, 0, two("RESE", "GND"), desc="current sense")
part("R18", R0603, "1M", "C22935", 25.2, 20.4, 0, two("GDR", "GND"))
part("D2", "Diode_SMD:D_SOD-123", "MBR0530", "C77336", 30.5, 15.2, 0,
     {"1": ("K", "EPD_NEG"), "2": ("A", "PREVGL")})
part("D3", "Diode_SMD:D_SOD-123", "MBR0530", "C77336", 30.5, 17.6, 0,
     {"1": ("K", "GND"), "2": ("A", "EPD_NEG")})
part("D4", "Diode_SMD:D_SOD-123", "MBR0530", "C77336", 30.5, 20, 0,
     {"1": ("K", "PREVGH"), "2": ("A", "EPD_SW")})
part("C21", C0805, "4.7uF", "C98192", 32.5, 12.2, 90, two("EPD_NEG", "EPD_SW"), desc="pump flying cap")
part("C22", C0805, "4.7uF", "C98192", 22.5, 12.2, 90, two("PREVGH", "GND"))
part("C23", C0805, "4.7uF", "C98192", 24.5, 12.2, 90, two("PREVGL", "GND"))
part("C24", C0805, "4.7uF", "C98192", 26.5, 12.2, 90, two("VSH1", "GND"))
part("C25", C0805, "4.7uF", "C98192", 28.5, 12.2, 90, two("VSH2", "GND"))
part("C26", C0805, "4.7uF", "C98192", 30.5, 12.2, 90, two("VSL", "GND"))
part("C27", C0603, "1uF", "C15849", 34.3, 12.2, 90, two("EPD_VDD", "GND"))
part("C28", C0603, "1uF", "C15849", 35.9, 12.2, 90, two("VCOM", "GND"))
part("C29", C0603, "1uF", "C15849", 37.5, 12.2, 90, two("+3V3", "GND"), desc="VCI/VDDIO")

# ------------------------------------------------------------- buttons -----
# Single ADC ladder on GPIO1 (10k pull-up). Windows match the FreeInk SDK
# OnePageAdcLadder decoder: LEFT 1780-2140 mV, RIGHT 1140-1500 mV, CONFIRM 0-250 mV.
BTN_Y = 37.8
KEY = f"{LIB_POCKET}:KEY-SMD_4P-L4.2-W3.2-P2.20-LS4.6"
part("R19", R0603, "10k", "C25804", 22.5, 33.0, 0, two("+3V3", "BTN_ADC"), desc="ladder pull-up")
part("C30", C0603, "100nF", "C14663", 25.5, 33.0, 0, two("BTN_ADC", "GND"))
part("SW1", KEY, "PREV", "C139797", 24.0, BTN_Y, 0, {
    "1": ("A", "BTN_L"), "2": ("A", "SW1_NC2"), "3": ("B", "SW1_NC3"), "4": ("B", "BTN_ADC")},
    desc="left front key: previous page")
part("R20", R0603, "15k", "C22809", 24.0, 42.0, 0, two("BTN_L", "GND"), desc="~1.98 V")
part("SW2", KEY, "REFRESH/MENU", "C139797", 34.5, BTN_Y, 0, {
    "1": ("A", "GND"), "2": ("A", "SW2_NC2"), "3": ("B", "SW2_NC3"), "4": ("B", "BTN_ADC")},
    desc="center key: short = full refresh, long = back/menu")
part("SW3", KEY, "NEXT", "C139797", 45.0, BTN_Y, 0, {
    "1": ("A", "BTN_R"), "2": ("A", "SW3_NC2"), "3": ("B", "SW3_NC3"), "4": ("B", "BTN_ADC")},
    desc="right front key: next page")
part("R21", R0603, "6.8k", "C23212", 43.5, 42.0, 0, two("BTN_R", "GND"), desc="~1.34 V")
part("SW4", "Button_Switch_SMD:SW_SPST_EVQP7C", "POWER", "C388883", 59.9, 20.0, 90, {
    "1": ("A", "PWR_BTN"), "2": ("B", "GND")}, desc="side power key, right edge")
part("R22", R0603, "10k", "C25804", 57.0, 23.4, 90, two("+3V3", "PWR_BTN"))
part("C31", C0603, "100nF", "C14663", 54.8, 23.4, 90, two("PWR_BTN", "GND"))

# --------------------------------------------------------------- USB-C -----
USB_PADS = {
    "A1": ("GND", "GND"), "B12": ("GND", "GND"), "A12": ("GND", "GND"), "B1": ("GND", "GND"),
    "A4": ("VBUS", "VBUS_IN"), "B9": ("VBUS", "VBUS_IN"), "A9": ("VBUS", "VBUS_IN"), "B4": ("VBUS", "VBUS_IN"),
    "A5": ("CC1", "CC1"), "B5": ("CC2", "CC2"),
    "A6": ("D+", "USB_DP"), "B6": ("D+", "USB_DP"), "A7": ("D-", "USB_DM"), "B7": ("D-", "USB_DM"),
    "A8": ("SBU1", "SBU1_NC"), "B8": ("SBU2", "SBU2_NC"), "S1": ("SHIELD", "GND"),
}
part("J3", "Connector_USB:USB_C_Receptacle_HRO_TYPE-C-31-M-12", "USB-C", "C165948",
     BOARD_W - 4.15 + 0.6, 37.5, 90, USB_PADS, desc="right side, bottom strip")
part("F1", "Resistor_SMD:R_0603_1608Metric", "0R", "C21189", 50.4, 32, 0, two("VBUS_IN", "VBUS"),
     desc="VBUS link (fit a PTC here if wanted)")
part("R23", R0603, "5.1k", "C23186", 50.0, 43.2, 0, two("CC1", "GND"))
part("R24", R0603, "5.1k", "C23186", 46.5, 43.2, 0, two("CC2", "GND"))
part("R25", R0603, "100k", "C25803", 42.0, 30.0, 90, two("VBUS", "USB_DET"))
part("R26", R0603, "100k", "C25803", 44.0, 30.0, 90, two("USB_DET", "GND"))

# ------------------------------------------------------------ test pads ----
part("TP3", "TestPoint:TestPoint_Pad_1.0x1.0mm", "3V3", "", 3.0, 3.0, 0, {"1": ("3V3", "+3V3")})
part("TP4", "TestPoint:TestPoint_Pad_1.0x1.0mm", "GND", "", 6.0, 3.0, 0, {"1": ("GND", "GND")})
part("TP5", "TestPoint:TestPoint_Pad_1.0x1.0mm", "BOOT", "", 38.0, 27.8, 0, {"1": ("GPIO9", "BOOT")})
part("TP6", "TestPoint:TestPoint_Pad_1.0x1.0mm", "EN", "", 9.0, 3.0, 0, {"1": ("EN", "EN")})


def nets():
    """All net names used by pads, sorted, GND first."""
    names = {n for p in PARTS for (_, n) in p["pads"].values()}
    return sorted(names, key=lambda n: (n != "GND", n))


def single_pad_nets():
    """Nets with exactly one pad: allowed only for deliberate no-connects."""
    owners = {}
    for p in PARTS:
        for _, n in p["pads"].values():
            owners.setdefault(n, set()).add(p["ref"])
    return sorted(n for n, refs in owners.items() if len(refs) == 1)


if __name__ == "__main__":
    print(f"{len(PARTS)} parts, {len(nets())} nets")
    for n in single_pad_nets():
        print("single-pad net:", n)
