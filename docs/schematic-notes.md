# nissan-z31 schematic — text extraction & firmware relevance notes

Source: `docs/hw-private-nissan-z31-a-schematic.pdf` (28 pages, A2, KiCad-generated, rev `a` 2026-05-14).

This board is the F7 sibling of [fw-uaefi-Honda-OBD1](https://github.com/rusefi/fw-uaefi-Honda-OBD1):
same mega-uaefi + Hellen-One module stack, but with the **mega-mcu100-f7/0.2** CPU module
(STM32F765VGT6) instead of the F4 mcu100. In firmware terms it is closest to
`ext/rusefi/firmware/config/boards/hellen/super-uaefi` (same F7 module stack).

## How this file was produced (repeatable process)

Same recipe as `fw-uaefi-Honda-OBD1/docs/schematic-notes.md` — only **poppler-utils**
(`pdftotext`, `pdfinfo`, `pdftoppm`) plus standard shell tools. The PDF is vector KiCad
output, all labels are real text, no OCR needed.

```bash
PDF=docs/hw-private-nissan-z31-a-schematic.pdf
pdfinfo $PDF                                   # 28 pages, A2
pdftotext -layout $PDF /tmp/schematic.txt      # full text, preserving 2D layout
for i in $(seq 1 28); do pdftotext -layout -f $i -l $i $PDF /tmp/page$i.txt; done
# classify sheets by title block
grep -iE "Sheet: /|Title:|Module:|Id: [0-9]+/" /tmp/page*.txt
```

Page 1 of this schematic is dense enough that `-layout` interleaves columns. Two extra
tricks were needed beyond the Honda recipe:

1. **bbox extraction** — `pdftotext -bbox -f 1 -l 1 $PDF page1.xml` gives every word an
   x/y coordinate; module pin names (a fixed x column, e.g. `x=822` for the mega-uaefi
   symbol, `x=1247` for the mcu100 symbol on page 2) can then be paired with the net
   label sitting just left of them (fixed y offset ≈ 3.4–3.9 pt) with a few lines of
   python. This resolves every module-pin ↔ net association unambiguously.
2. **regional rendering** — for anything still ambiguous (staggered dual-column pin
   numbers on the OEM connector and J1), render just that region and read it:
   `pdftoppm -f 1 -l 1 -r 300 -x <pt*300/72> -y ... -W ... -H ... -png $PDF out`.

Every association below was additionally cross-checked against the firmware sources of
truth:

```bash
# platform symbol → STM32 pin definitions (F7 mm100)
less ext/rusefi/firmware/config/boards/hellen_mm100_meta.h
# closest sibling board (same F7 module stack)
less ext/rusefi/firmware/config/boards/hellen/super-uaefi/board_configuration.cpp
# shared mega-uaefi defaults
less ext/rusefi/firmware/config/boards/hellen/uaefi121/mega-uaefi.cpp
```

## Page map and firmware relevance

The board is a stack: **top board (Z31 adapter) → mega-uaefi module → mcu100-f7 CPU
module + Hellen-One function modules**. Firmware only cares about connector-pin ↔
signal ↔ STM32 pin, plus populate-options that change configuration.

| Page | Sheet | Firmware relevance |
|---|---|---|
| **1** | **Top level `z31`** (hw-private-nissan-z31.kicad_sch) | **ESSENTIAL — the only page needed for pin mapping.** Z31 OEM ECU connector → signal, aux connector J1, USB-C J2, and signal → STM32 pin for the mcu100-f7 module. All tables below come from here. |
| **2** | mega-uaefi module top (uaefi.kicad_sch, `Module:mega-uaefi/0.5`) | High — routes module edge pins to mcu100 pins; source of the module-pin → STM32 mapping table. |
| 3 | /LS/ — low-side drivers | Low — VNLD5160 smart low-side driver internals. |
| 4 | /LS HOT/ — always-hot low side | Low — same driver for main-relay/fan class outputs (OUT_LS5_HOT, OUT_LS6_HOT). |
| 5 | /MAP/ — onboard MAP option | Medium — MPX4/MPXH6400 onboard MAP populate-option (feeds `INTERNAL_MAP` → PC1). |
| 6 | /Ignition/ | **Medium-high** — ignition outputs are **logic-level (smart coil) by default**; option: remove R870–R872/R876–R878 and populate Q841–Q846 (ISL9V3040 IGBTs) for dumb coils. The Z31 stock setup drives the OEM power transistor via a single logic-level output (IGN1). |
| 7 | /EGT/ | Low — MAX31855K thermocouple interface (SPI), populate option. |
| 8 | /Injector/ | Low — 3× VNLD5160 = 6 injector channels, driver internals only. |
| 9 | WBO (wideband) module | **Not main-firmware** — own STM32F042K6 running rusEFI WBO firmware, talks CAN. |
| 10 | (no extractable text — graphics-only sheet) | Not relevant. |
| 11 | Hellen-One Knock module | Medium — LMV321 op-amp conditioning feeding IN_KNOCK (PA2). |
| 12 | VR-discrete module ("outputs a pulse on rising edge") | Medium — present on board; **inputs not wired to any connector on this design** (see below). Threshold on OUT_PWM6/PD14. |
| 13 | VR MAX9924 module | Medium — present on board; inputs likewise not brought out. R843 populate-option turns VR− into a Hall input. |
| 14 | Motor-driver module (TLE9201SG) | Medium — DC1/DC2 H-bridges exist on the mega-uaefi; the OUT_DC1±/OUT_DC2± edge pins have **no labeled destination** on page 1 (no factory ETB on Z31). |
| 15–27 | mcu100-f7 CPU-module internals: op-amp buffers (p17), **STM32F765VGT6** (p19), 680k dividers (p20), TJA1051 CAN1 transceiver (p23), USB (p24, p26), **LPS22HB baro + LIS2DH12 accelerometer** (p27) | **Not needed** — fixed Hellen mm100-f7 platform; already encoded in `hellen_mm100_meta.h`. |
| 28 | Hellen-One CAN module (TJA1051T) | Not relevant beyond "the mega-uaefi carries a second CAN transceiver" (CAN2, PB12/PB13). |

**TL;DR: page 1 (plus page 2 for the module-internal routing) is what firmware work
needs; pages 15–28 are platform module internals you can ignore.**

## Z31 OEM ECU connector → board signal

Pin numbering as printed on the schematic connector symbol (Nissan ECCS three-row
header; `—` = not connected on this board).

16-pin power/injector block:

| Pin | Signal | Pin | Signal |
|---|---|---|---|
| 101 | Z31_INJ1 (cyl 1 injector) | 108 | 12 V |
| 102 | Z31_INJ2 (cyl 2) | 109 | Power ground |
| 103 | Z31_INJ3 (cyl 3) | 112 | Power ground |
| 104 | Z31_INJ4 (cyl 4) | 113 | Power ground |
| 105 | Z31_INJ5 (cyl 5) | 114 | 12 V |
| 106 | Z31_INJ6 (cyl 6) | 115 | — |
| 107 | Power ground | 1004/1005 | — |

20-pin block:

| Pin | Signal | Pin | Signal |
|---|---|---|---|
| 1 | — | 11–15 | — |
| 2 | OUT_IDLE (idle air valve solenoid) | 16 | Ground |
| 3, 4 | — | 17 | IN_CRANK (CAS crank signal) |
| 5 | OUT_IGN (OEM power transistor) | 18, 19 | — |
| 6 | OUT_MAIN_RELAY | 20 | OUT_FUEL_PUMP_RELAY |
| 7 | — | | |
| 8 | IN_CAM (CAS cam signal) | | |
| 9 | — | | |
| 10 | IN_CLUTCH | | |

16-pin sensor/power block:

| Pin | Signal | Pin | Signal |
|---|---|---|---|
| 21 | IN_KNOCK | 28 | Ground |
| 22 | — | 29 | IN_VSS (vehicle speed) |
| 23 | IN_CLT | 30 | — |
| 24, 25 | — | 31 | IN_MAF |
| 26 | Ground | 34 | +12V_FROM_KEY (ignition switch) |
| 27 | +12V_FROM_MAIN_RELAY | 35 | +12V_FROM_MAIN_RELAY |
| 1006/1007 | — | 36 | Ground |

## J1 aux connector (TE 6437288-2, 34-pos) → board signal

| Pin | Signal | Pin | Signal |
|---|---|---|---|
| 1 | GND | 18 | GND |
| 2 | CANL | 19 | WBO_Ip |
| 3 | CANH | 20 | WBO_Un |
| 4 | +5VP | 21 | GND |
| 5 | IN_BUTTON3 | 22 | IN_AUX_KNOCK |
| 6 | IN_BUTTON2 | 23 | IN_FLEX |
| 7 | OUT_LS3 | 24 | IN_IAT |
| 8 | OUT_LS1 | 25 | IGN_AUX_5 |
| 9 | IGN_AUX_3 | 26 | WBO_HTR− |
| 10–12 | GND | 27 | WBO_Vm |
| 13–16 | +5VP | 28 | WBO_Rtrim |
| 17 | IGN_AUX_4 | 29 | IN_PPS2 |
| | | 30 | IN_TPS2 |
| | | 31 | IN_MAP |
| | | 32 | IN_TPS1 |
| | | 33 | IN_PPS1 |
| | | 34 | OUT_12V (fused 12 V out) |

J2 is a USB-C connector (VBUS, D±, CC1/CC2, SBU).

## Signal → STM32 pin (net → mega-uaefi edge pin → mcu100-f7 pin)

Chain verified on page 1 (net → mega-uaefi symbol) and page 2 (mega-uaefi edge →
mcu100 pin label). `MM100_*` names from `hellen_mm100_meta.h`.

Outputs:

| Z31 net (source) | mega-uaefi pin | STM32 | MM100 symbol |
|---|---|---|---|
| Z31_INJ1 (101) | OUT_INJ1 | PD3 | MM100_INJ1 |
| Z31_INJ2 (102) | OUT_INJ2 | PA9 | MM100_INJ2 |
| Z31_INJ3 (103) | OUT_INJ3 | PD11 | MM100_INJ3 |
| **Z31_INJ4 (104)** | **OUT_INJ5** | **PD2** | **MM100_INJ5** |
| **Z31_INJ5 (105)** | **OUT_INJ4** | **PD10** | **MM100_INJ4** |
| Z31_INJ6 (106) | OUT_INJ6 | PA8 | MM100_INJ6 |
| OUT_IGN (5) | IGN1 | PC13 | MM100_IGN1 |
| IGN_AUX_3 (J1-9) | IGN3 | PE4 | MM100_IGN3 |
| IGN_AUX_4 (J1-17) | IGN4 | PE3 | MM100_IGN4 |
| IGN_AUX_5 (J1-25) | IGN5 | PE2 | MM100_IGN5 |
| OUT_LS1 (J1-8) | OUT_LS1 | PD15 | MM100_INJ7 |
| OUT_IDLE (2) | OUT_LS2 ("Idle Air Valve") | PD12 | MM100_INJ8 |
| OUT_LS3 (J1-7) | OUT_LS3 | PD13 | MM100_OUT_PWM1 |
| OUT_FUEL_PUMP_RELAY (20) | OUT_LS4 ("Fuel Pump Relay") | PC6 | MM100_OUT_PWM2 |
| OUT_MAIN_RELAY (6) | OUT_LS5_HOT ("Main Relay", driven by IGN7) | PB9 | MM100_IGN7 |
| — (not wired) | OUT_LS6_HOT ("Fan Relay", driven by IGN8) | PE6 | MM100_IGN8 |
| VR discrete threshold (internal) | — | PD14 | MM100_OUT_PWM6 |

⚠ Note the **injector 4/5 crossover**: OEM pin 104 (cyl 4) is wired to module channel
OUT_INJ5 and OEM pin 105 (cyl 5) to OUT_INJ4 — confirmed visually on page 1. Firmware
must set `injectionPins[3] = MM100_INJ5` and `injectionPins[4] = MM100_INJ4`.

Inputs:

| Z31 net (source) | mega-uaefi pin | STM32 | MM100 symbol / ADC |
|---|---|---|---|
| IN_CRANK (17, CAS crank) | IN_HALL2 | PE13 | MM100_IN_D2 |
| IN_CAM (8, CAS cam) | IN_HALL1 | PE12 | MM100_IN_D1 |
| IN_VSS (29) | IN_HALL3 | PE14 | MM100_IN_D3 |
| IN_CLUTCH (10) | IN_BUTTON1 | PB1 | MM100_IN_CRANK |
| IN_BUTTON2 (J1-6) | IN_BUTTON2 | PA6 | MM100_IN_CAM |
| IN_BUTTON3 (J1-5) | IN_BUTTON3 | PE15 | MM100_IN_D4 |
| IN_FLEX (J1-23) | IN_FLEX | PE11 | MM100_IN_VSS |
| IN_KNOCK (21) | IN_KNOCK | PA2 | (software knock, ADC3) |
| IN_CLT (23) | IN_CLT | PC2 | MM100_IN_CLT_ANALOG = EFI_ADC_12 |
| IN_IAT (J1-24) | IN_IAT | PC3 | MM100_IN_IAT_ANALOG = EFI_ADC_13 |
| IN_MAF (31) | IN_AUX2 | PA1 | MM100_IN_O2S2_ANALOG = EFI_ADC_1 |
| IN_AUX_KNOCK (J1-22) | IN_AUX1 | PA0 | MM100_IN_O2S_ANALOG = EFI_ADC_0 |
| IN_MAP (J1-31) | IN_MAP | PC0 | MM100_IN_MAP1_ANALOG = EFI_ADC_10 |
| onboard MAP option (page 5) | `INTERNAL_MAP` | PC1 | MM100_IN_MAP2_ANALOG = EFI_ADC_11 |
| IN_TPS1 (J1-32) | IN_TPS1 | PA4 | MM100_IN_TPS_ANALOG = EFI_ADC_4 |
| IN_TPS2 (J1-30) | IN_TPS2 | PB0 | MM100_IN_AUX1_ANALOG = EFI_ADC_8 |
| IN_PPS1 (J1-33) | IN_PPS1 | PA3 | MM100_IN_PPS_ANALOG = EFI_ADC_3 |
| IN_PPS2 (J1-29) | IN_PPS2 | PC4 | MM100_IN_AUX2_ANALOG = EFI_ADC_14 |
| — | IN_AUX3 | PA7 | MM100_IN_AUX3_ANALOG = EFI_ADC_7 |
| +12V_FROM_KEY (34) | power module → VIGN | PA5 | MM100_IN_VBATT = EFI_ADC_5 |

Comms & internal:

| Function | STM32 |
|---|---|
| CAN1 (onboard transceiver in mcu100-f7; WBO + J1 CAN bus) | PD0 RX / PD1 TX (MM100_CAN_RX/TX) |
| CAN2 (mega-uaefi TJA1051 transceiver, F7 CAN2) | PB12 RX / PB13 TX |
| VR MAX9924 module output (crank, if used) | PE1 (MM100_UART8_TX) |
| VR discrete module output (cam, if used) | PE0 (MM100_UART8_RX) |
| VR discrete threshold PWM | PD14 (MM100_OUT_PWM6) |
| SD card + LIS2DH12 accelerometer | SPI1 |
| LPS22HB baro (mcu100-f7 onboard, page 27) | I2C PB10/PB11 |
| UART2 | PD5 TX / PD6 RX |
| LEDs | green PD7, blue PE7, yellow PE8, red PD4 |

## Populate-options and notes that affect firmware configuration

* **Ignition outputs are logic-level (smart coil) by default.** For dumb coils: remove
  R870–R872, R876–R878 and populate Q841–Q846 (ISL9V3040 IGBTs, page 6). Stock Z31
  drives the OEM external power transistor from IGN1 — logic level is correct there.
* **Z31 trigger is the optical CAS in the distributor** (crank + cam photo-interrupter
  outputs on OEM pins 17 and 8) — these are wired to digital inputs IN_D2/IN_D1, *not*
  to the VR modules. The VR MAX9924 (page 13) and VR-discrete (page 12) modules are
  present on the mega-uaefi but their inputs are not brought out to any connector on
  this design.
* **Onboard MAP option** MPX4/MPXH6400 (page 5) feeds `INTERNAL_MAP` → PC1
  (EFI_ADC_11); external MAP goes to J1-31 → PC0 (EFI_ADC_10). The mcu100-f7 also has
  an LPS22HB baro on I2C (page 27).
* **CAN wakeup:** move resistor R143 → R145 to enable CAN wakeup on PA0; the AUX1
  analog input (IN_AUX_KNOCK, J1-22) becomes unavailable (page 1).
* **WBO controller is autonomous** (STM32F042K6, own firmware, CAN-attached, page 9) —
  main firmware sees it as a CAN wideband. LSU cells break out on J1 pins 19/20/26/27/28.
* **MAF car**: the Z31 (VG30E/ET) is a MAF (hot-wire) engine — IN_MAF lands on
  EFI_ADC_1. Speed-density via external/onboard MAP is the usual rusEFI conversion.
* **EGT** MAX31855 (page 7) is a populate option (`EFI_MAX_31855` commented in
  `board.mk`).
* **ETB**: TLE9201SG H-bridges exist inside the mega-uaefi module, but OUT_DC1±/OUT_DC2±
  have no labeled destination on the top-level page — no factory ETB on Z31.

## ⚠ Known discrepancy found during extraction

This repo's local `knock_config.h` said knock is **PA3 / ADC_CHANNEL_IN3**, but the
schematic (page 1: `IN_KNOCK_(PA2)`, page 2: knock module → `IN_KNOCK_(PA2)`) and the
F7 sibling `ext/rusefi/firmware/config/boards/hellen/super-uaefi/knock_config.h` both
say **PA2 / ADC_CHANNEL_IN2** (PA3 is IN_PPS). Same stale-copy bug that was found in
fw-uaefi-Honda-OBD1. Fixed together with the board configuration work documented here.

## Key ICs (for reference)

| IC | Function | Page |
|---|---|---|
| STM32F765VGT6 | Main MCU (mcu100-f7 module) | 19 |
| VNLD5160TR-E | Dual smart low-side drivers (injectors, LS outputs) | 3, 4, 8 |
| ISL9V3040D3ST | Ignition IGBTs (dumb-coil option, DNP by default) | 6 |
| MAX9924UAUB+ | VR conditioner (unused inputs on this design) | 13 |
| TLE9201SG | DC motor H-bridge ×2 (no factory ETB) | 14 |
| MPX4/MPXH6400 | Onboard MAP option | 5 |
| MAX31855KASA | EGT thermocouple interface (option) | 7 |
| STM32F042K6 | Standalone WBO controller | 9 |
| TJA1051T/3 | CAN transceivers | 9, 23, 28 |
| LPS22HBTR + LIS2DH12TR | Baro + accelerometer (mcu100-f7) | 27 |
