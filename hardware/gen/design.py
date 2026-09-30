"""Single source of truth for the Pocket E-Ink 4.26 main board.

Every part lists: reference, footprint (lib:name), value, LCSC part number,
placement (x, y in mm from the board's top-left corner, rotation in degrees,
KiCad convention: positive = counter-clockwise on screen) and a pad map
pad -> (pin name, net). build_pcb.py and build_sch.py both read this file,
so the schematic and the board can never drift apart.

Board frame (portrait, seen from the front / panel side, y grows downward):
  y 0.0 .. 26.3  : under the e-paper panel (parts must stay <= 1.6 mm tall)
  y 26.3 .. 45.0 : strip below the panel (battery/charger/LDO, 3 buttons, USB-C)
Pin map follows the Xteink X4 (ESP32-C3 + SSD1677 800x480) so the CrossPoint /
FreeInk SDK drivers apply with a small board profile (see firmware/).
"""

BOARD_W = 62.0
BOARD_H = 45.0
PANEL_EDGE_Y = 26.3  # bottom edge of the GDEY0426T82 active glass over the PCB
# The panel's FPC (about 24 mm past the glass, per the Waveshare outline of
# 129.33 mm vs 105.33 mm glass) folds 180 degrees behind the panel and runs back
# up to this connector opening. Adjust after measuring a real panel.
FPC_OPENING_Y = 10.0

# Antenna keep-out (no copper on any layer): top-right corner.
ANT_KEEPOUT = (51.0, 0.0, 62.0, 7.5)  # x0, y0, x1, y1

# Net classes: name -> (track width, clearance) in mm
NETCLASSES = {
    "Default": (0.127, 0.127),
    "Power": (0.25, 0.15),
    "HV": (0.25, 0.15),  # e-paper gate/source rails, up to about +-22 V; FPC pads sit 0.2 mm apart
    "RF": (0.30, 0.20),
}
NET_CLASS_OF = {
    "GND": "Power", "+3V3": "Power", "VBUS": "Power", "VBAT": "Power",
    "BAT_RAW-": "Power", "SD_VDD": "Power",
    "PREVGH": "HV", "PREVGL": "HV", "VSH1": "HV", "VSH2": "HV", "VSL": "HV",
    "VCOM": "HV", "EPD_SW": "HV", "EPD_NEG": "HV", "GDR": "HV",
    "RF_ANT": "RF", "RF_LNA": "RF", "RF_MID": "RF",
}

LIB_POCKET = "pocket"  # hardware/lib/pocket.pretty (footprints from EasyEDA/LCSC)

R0402 = "Resistor_SMD:R_0402_1005Metric"
C0402 = "Capacitor_SMD:C_0402_1005Metric"
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
    "1": ("LNA_IN", "RF_LNA"),
    "2": ("VDD3P3", "+3V3"),
    "3": ("VDD3P3", "+3V3"),
    "4": ("GPIO0/ADC", "BAT_SENSE"),
    "5": ("GPIO1/ADC", "BTN_ADC"),
    "6": ("GPIO2", "SD_PWR_N"),
    "7": ("CHIP_EN", "EN"),
    "8": ("GPIO3", "PWR_BTN"),
    "9": ("GPIO4", "EPD_DC"),
    "10": ("GPIO5", "EPD_RST"),
    "11": ("VDD3P3_RTC", "+3V3"),
    "12": ("GPIO6", "EPD_BUSY"),
    "13": ("GPIO7", "SD_MISO"),
    "14": ("GPIO8", "SPI_SCLK"),
    "15": ("GPIO9/BOOT", "BOOT"),
    "16": ("GPIO10", "SPI_MOSI"),
    "17": ("VDD3P3_CPU", "+3V3"),
    "18": ("VDD_SPI", "VDD_SPI"),
    "19": ("GPIO12/SPIHD", "SD_CS"),
    "20": ("GPIO13/SPIWP", "NC_GPIO13"),
    "21": ("SPICS0", "FLASH_CS"),
    "22": ("SPICLK", "FLASH_CLK"),
    "23": ("SPID", "FLASH_DI"),
    "24": ("SPIQ", "FLASH_DO"),
    "25": ("GPIO18/USB_D-", "USB_DM"),
    "26": ("GPIO19/USB_D+", "USB_DP"),
    "27": ("GPIO20/U0RXD", "USB_DET"),
    "28": ("GPIO21/U0TXD", "EPD_CS"),
    "29": ("XTAL_N", "XTAL_N"),
    "30": ("XTAL_P", "XTAL_P_CHIP"),
    "31": ("VDDA", "+3V3"),
    "32": ("VDDA", "+3V3"),
    "33": ("GND/EP", "GND"),
}
part("U1", "Package_DFN_QFN:QFN-32-1EP_5x5mm_P0.5mm_EP3.7x3.7mm_ThermalVias", "ESP32-C3", "C2838500",
     46, 12, -90, ESP_PINS, desc="Wi-Fi/BLE MCU, RISC-V 160 MHz")
part("U2", "Package_SON:WSON-8-1EP_6x5mm_P1.27mm_EP3.4x4.3mm", "W25Q128JVPIQ", "C190862",
     48, 20.5, 0, {
         "1": ("/CS", "FLASH_CS"), "2": ("DO", "FLASH_DO"), "3": ("/WP", "VDD_SPI"),
         "4": ("GND", "GND"), "5": ("DI", "FLASH_DI"), "6": ("CLK", "FLASH_CLK"),
         "7": ("/HOLD", "VDD_SPI"), "8": ("VCC", "VDD_SPI"), "9": ("EP", "GND")},
     desc="16 MB QSPI NOR, used in DIO mode like the X4")
part("Y1", "Crystal:Crystal_SMD_3225-4Pin_3.2x2.5mm", "40MHz", "C5380316", 52.2, 12.4, 90, {
    "1": ("X1", "XTAL_P"), "2": ("GND", "GND"), "3": ("X2", "XTAL_N"), "4": ("GND", "GND")})
part("R1", R0402, "0R", "C17168", 50.4, 9, 0, two("XTAL_P", "XTAL_P_CHIP"),
     desc="XTAL_P series, Espressif tuning position")
part("C1", C0402, "12pF", "C1547", 55, 10.8, 90, two("XTAL_P", "GND"))
part("C2", C0402, "12pF", "C1547", 55, 14, 90, two("XTAL_N", "GND"))
# RF: fitted C-L-C-L matching network then chip antenna
part("C3", C0402, "1.5pF", "C1552", 48.4, 7.6, 90, two("RF_LNA", "GND"),
     desc="RF match shunt 1")
part("L2", "Inductor_SMD:L_0402_1005Metric", "2.7nH", "C77108", 49.2, 6.2, 0,
     two("RF_LNA", "RF_MID"), desc="RF match series 1, LQG15HS2N7S02D")
part("C4", C0402, "1.5pF", "C1552", 51, 6, 90, two("RF_MID", "GND"),
     desc="RF match shunt 2")
part("L3", "Inductor_SMD:L_0402_1005Metric", "6.8nH", "C77110", 52.6, 5.2, 0,
     two("RF_MID", "RF_ANT"), desc="RF match series 2, LQG15HS6N8J02D")
part("AE1", f"{LIB_POCKET}:FILTER-SMD_1206-2P-L3.2-W1.6-L", "RFANT3216120A5T", "C127629",
     57.5, 3.0, 0, {"1": ("FEED", "RF_ANT"), "2": ("NC", "ANT_NC")},
     desc="2.4 GHz chip antenna, keep-out zone around it")
# Supply decoupling near the MCU
part("C5", C0603, "10uF", "C19702", 41.2, 16.4, 90, two("+3V3", "GND"))
part("C6", C0402, "100nF", "C1525", 46.6, 7.6, 0, two("+3V3", "GND"), desc="VDD3P3 pins 2/3")
part("C7", C0402, "100nF", "C1525", 41.8, 11.2, 90, two("+3V3", "GND"), desc="VDD3P3_RTC")
part("C8", C0402, "100nF", "C1525", 43.6, 16.6, 0, two("+3V3", "GND"), desc="VDD3P3_CPU")
part("C9", C0402, "1uF", "C14445", 45.6, 16.6, 0, two("VDD_SPI", "GND"), desc="VDD_SPI -> flash")
part("C10", C0402, "100nF", "C1525", 50.0, 14.5, 90, two("+3V3", "GND"), desc="VDDA 31/32")
part("C11", C0402, "100nF", "C1525", 51.8, 19, 90, two("VDD_SPI", "GND"), desc="flash VCC")
part("R3", R0402, "10k", "C25744", 43.4, 7.4, 90, two("+3V3", "EN"))
part("C12", C0402, "1uF", "C14445", 42.2, 7.4, 90, two("EN", "GND"), desc="EN RC delay")
part("R4", R0402, "10k", "C25744", 41.8, 13.4, 90, two("+3V3", "BOOT"), desc="GPIO9 strap high")
part("R5", R0402, "10k", "C25744", 40.6, 13.4, 90, two("+3V3", "SPI_SCLK"), desc="GPIO8 strap high")

# ------------------------------------------------------------ battery ------
part("TP1", "TestPoint:TestPoint_Pad_1.0x1.0mm", "BAT+", "", 2.5, 29, 0,
     {"1": ("BAT+", "VBAT")}, desc="battery red wire")
part("TP2", "TestPoint:TestPoint_Pad_1.0x1.0mm", "BAT-", "", 2.5, 31.5, 0,
     {"1": ("BAT-", "BAT_RAW-")}, desc="battery black wire")
# DW01A + FS8205A protection (same topology as IS7V4N ESP32_E-Reader Rev B)
part("U3", "Package_TO_SOT_SMD:SOT-23-6", "DW01A", "C351410", 6.5, 29.5, 0, {
    "1": ("OD", "PROT_OD"), "2": ("CS", "PROT_CS"), "3": ("OC", "PROT_OC"),
    "4": ("TD", "PROT_TD"), "5": ("VCC", "PROT_VCC"), "6": ("GND", "BAT_RAW-")})
part("Q1", "Package_TO_SOT_SMD:SOT-23-6", "FS8205A", "C2830320", 11, 29.5, 0, {
    "1": ("S1", "BAT_RAW-"), "2": ("D1", "Q1_D"), "3": ("S2", "GND"),
    "4": ("G2", "PROT_OC"), "5": ("D2", "Q1_D"), "6": ("G1", "PROT_OD")},
    desc="dual N-MOSFET, drains common")
part("R6", R0402, "470R", "C25117", 6.5, 32.4, 0, two("VBAT", "PROT_VCC"))
part("C13", C0402, "100nF", "C1525", 8.8, 32.4, 0, two("PROT_VCC", "BAT_RAW-"))
part("R7", R0402, "2.7k", "C25885", 11, 32.4, 0, two("PROT_CS", "GND"))
part("C14", C0603, "10uF", "C19702", 14, 29.5, 90, two("VBAT", "GND"))
# Battery sense 1M/1M (~2 uA) into GPIO0
part("R8", R0402, "1M", "C26083", 38.4, 19.5, 90, two("VBAT", "BAT_SENSE"))
part("R9", R0402, "1M", "C26083", 39.6, 19.5, 90, two("BAT_SENSE", "GND"))
part("C15", C0402, "100nF", "C1525", 40.8, 19.5, 90, two("BAT_SENSE", "GND"))

# ------------------------------------------------------------ charger ------
part("U4", "Package_TO_SOT_SMD:SOT-23-5", "MCP73831-2", "C424093", 20, 29.8, 0, {
    "1": ("STAT", "CHG_STAT"), "2": ("VSS", "GND"), "3": ("VBAT", "VBAT"),
    "4": ("VDD", "VBUS"), "5": ("PROG", "CHG_PROG")}, desc="LiPo charger, 256 mA")
part("R10", R0402, "3.9k", "C51721", 20, 32.6, 0, two("CHG_PROG", "GND"), desc="Ichg = 1000/3.9k")
part("C16", C0603, "4.7uF", "C19666", 17.2, 29.8, 90, two("VBUS", "GND"))
part("C17", C0603, "4.7uF", "C19666", 22.8, 29.8, 90, two("VBAT", "GND"))
part("D1", "LED_SMD:LED_0603_1608Metric", "RED", "C2286", 17.2, 33.4, 0,
     {"1": ("K", "LED_K"), "2": ("A", "VBUS")}, desc="charging indicator")
part("R11", R0402, "1k", "C11702", 19.8, 34.4, 0, two("LED_K", "CHG_STAT"))

# ----------------------------------------------------------- 3V3 LDO -------
part("U5", "Package_TO_SOT_SMD:SOT-23-5", "RT9080-33GJ5", "C841192", 28, 29.8, 0, {
    "1": ("VIN", "VBAT"), "2": ("GND", "GND"), "3": ("EN", "VBAT"), "4": ("NC", "LDO_NC"),
    "5": ("VOUT", "+3V3")}, desc="600 mA, 2 uA Iq")
part("C18", C0603, "1uF", "C15849", 25.2, 29.8, 90, two("VBAT", "GND"))
part("C19", C0603, "10uF", "C19702", 30.8, 29.8, 90, two("+3V3", "GND"))

# ----------------------------------------------------------- microSD -------
part("J1", "Connector_Card:microSD_HC_Hirose_DM3AT-SF-PEJM5", "microSD", "C114218", 9.0, 12, -90, {
    "1": ("DAT2", "SD_DAT2"), "2": ("CD/DAT3", "SD_CS"), "3": ("CMD", "SPI_MOSI"),
    "4": ("VDD", "SD_VDD"), "5": ("CLK", "SPI_SCLK"), "6": ("VSS", "GND"),
    "7": ("DAT0", "SD_MISO"), "8": ("DAT1", "SD_DAT1"), "9": ("DET", "GND"),
    "10": ("DET_SW", "SD_DET_NC"), "11": ("SHIELD", "GND")},
    desc="card inserted from the left edge")
part("Q2", "Package_TO_SOT_SMD:SOT-23", "AO3401A", "C15127", 15.6, 23.0, 0, {
    "1": ("G", "SD_PWR_N"), "2": ("S", "+3V3"), "3": ("D", "SD_VDD")},
    desc="SD power switch, GPIO2 low = on")
part("R12", R0402, "10k", "C25744", 18.4, 23.0, 90, two("+3V3", "SD_PWR_N"), desc="off at boot, keeps GPIO2 strap high")
part("C20", C0603, "1uF", "C15849", 12.6, 23.0, 90, two("SD_VDD", "GND"))
part("R13", R0402, "10k", "C25744", 16.0, 3.0, 90, two("SD_VDD", "SD_CS"))
part("R14", R0402, "10k", "C25744", 14.6, 3.0, 90, two("SD_VDD", "SD_MISO"))
part("R15", R0402, "10k", "C25744", 13.2, 3.0, 90, two("SD_VDD", "SD_DAT1"))
part("R16", R0402, "10k", "C25744", 11.8, 3.0, 90, two("SD_VDD", "SD_DAT2"))

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
part("C32", C0603, "4.7uF", "C19666", 18.0, 20.0, 180, two("+3V3", "GND"),
     desc="boost input bulk, next to L1 pin 1")
part("Q3", "Package_TO_SOT_SMD:SOT-323_SC-70", "Si1308EDL", "C469327", 25.8, 16, 0, {
    "1": ("G", "GDR"), "2": ("S", "RESE"), "3": ("D", "EPD_SW")})
part("R17", R0603, "2.2R", "C22939", 25.8, 18.6, 0, two("RESE", "GND"), desc="current sense")
part("R18", R0402, "1M", "C26083", 25.8, 20.4, 0, two("GDR", "GND"))
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
BTN_Y = 38.3
KEY = f"{LIB_POCKET}:KEY-SMD_4P-L4.2-W3.2-P2.20-LS4.6"
part("R19", R0402, "10k", "C25744", 20.0, 43.4, 0, two("+3V3", "BTN_ADC"), desc="ladder pull-up")
part("C30", C0402, "100nF", "C1525", 22.4, 43.4, 0, two("BTN_ADC", "GND"))
part("SW1", KEY, "PREV", "C139797", 13.0, BTN_Y, 0, {
    "1": ("A", "BTN_L"), "2": ("A", "SW1_NC2"), "3": ("B", "SW1_NC3"), "4": ("B", "BTN_ADC")},
    desc="left front key: previous page")
part("R20", R0402, "15k", "C25756", 13.0, 41.8, 0, two("BTN_L", "GND"), desc="~1.98 V")
part("SW2", KEY, "REFRESH/MENU", "C139797", 31.0, BTN_Y, 0, {
    "1": ("A", "GND"), "2": ("A", "SW2_NC2"), "3": ("B", "SW2_NC3"), "4": ("B", "BTN_ADC")},
    desc="center key: short = full refresh, long = back/menu")
part("SW3", KEY, "NEXT", "C139797", 49.0, BTN_Y, 0, {
    "1": ("A", "BTN_R"), "2": ("A", "SW3_NC2"), "3": ("B", "SW3_NC3"), "4": ("B", "BTN_ADC")},
    desc="right front key: next page")
part("R21", R0402, "6.8k", "C25917", 49.0, 41.8, 0, two("BTN_R", "GND"), desc="~1.34 V")
part("SW4", "Button_Switch_SMD:SW_SPST_EVQP7C", "POWER", "C388883", 59.9, 20.0, 90, {
    "1": ("A", "PWR_BTN"), "2": ("B", "GND")}, desc="side power key, right edge")
part("R22", R0402, "10k", "C25744", 57.0, 23.4, 90, two("+3V3", "PWR_BTN"))
part("C31", C0402, "100nF", "C1525", 55.6, 23.4, 90, two("PWR_BTN", "GND"))

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
part("R23", R0402, "5.1k", "C25905", 48.4, 43.6, 0, two("CC1", "GND"))
part("R24", R0402, "5.1k", "C25905", 46.2, 43.6, 0, two("CC2", "GND"))
part("U6", "Package_TO_SOT_SMD:SOT-23-6", "USBLC6-2SC6", "C7519", 53.0, 28.6, 0, {
    "1": ("IO1", "USB_DM"), "2": ("GND", "GND"), "3": ("IO2", "USB_DP"),
    "4": ("IO2", "USB_DP"), "5": ("VBUS", "VBUS"), "6": ("IO1", "USB_DM")})
part("R25", R0402, "100k", "C25741", 42.0, 30.0, 90, two("VBUS", "USB_DET"))
part("R26", R0402, "100k", "C25741", 43.4, 30.0, 90, two("USB_DET", "GND"))

# ------------------------------------------------------------ test pads ----
part("TP3", "TestPoint:TestPoint_Pad_1.0x1.0mm", "3V3", "", 4.5, 41.2, 0, {"1": ("3V3", "+3V3")})
part("TP4", "TestPoint:TestPoint_Pad_1.0x1.0mm", "GND", "", 7, 41.2, 0, {"1": ("GND", "GND")})
part("TP5", "TestPoint:TestPoint_Pad_1.0x1.0mm", "BOOT", "", 38.0, 27.8, 0, {"1": ("GPIO9", "BOOT")})
part("TP6", "TestPoint:TestPoint_Pad_1.0x1.0mm", "EN", "", 7, 43.6, 0, {"1": ("EN", "EN")})


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
