# Arduino Uno Wiring Guide — Smart APD

This guide is for running the Smart APD on an **Arduino Uno** (ATmega328P).

---

## 1. Pin Mapping Summary

| Arduino Uno Pin | Connected To | Function / Notes |
|---|---|---|
| **A4** | DS3231 RTC `SDA` & OLED `SDA` | I²C Data (Shared bus) |
| **A5** | DS3231 RTC `SCL` & OLED `SCL` | I²C Clock (Shared bus) |
| **D2** | A4988 `STEP` | Stepper step pulses |
| **D3** | A4988 `DIR` | Stepper rotation direction |
| **D4** | A4988 `ENABLE` | Stepper driver enable (Active LOW) |
| **D5** | Optical / IR Home Sensor `OUT` | Carousel home position detection |
| **D6** | IR Passage Sensor `OUT` | Pill drop / chute verification |
| **D7** | Pushbutton (Acknowledge) | Pushbutton to GND (Uses internal pull-up) |
| **D8** | Active Buzzer `+` | Audio alarm / alerts |
| **D9** | Servo Signal (Orange / White wire) | Gate open/close mechanism (PWM) |
| **5V** | OLED VCC, DS3231 VCC, Sensors VCC | 5V Logic power |
| **GND** | All module GNDs | Common Ground (MANDATORY) |

---

## 2. A4988 Stepper Driver Connections

- `VDD` -> Arduino 5V
- `GND` (logic) -> Arduino GND
- `RESET` and `SLEEP` -> Jumpered / shorted together
- `MS1, MS2, MS3` -> Tied to GND (Full-step mode)
- `VMOT` -> External 12V DC power supply positive
- `GND` (motor) -> External 12V DC power supply negative
- `1A, 1B, 2A, 2B` -> NEMA 17 motor coil pairs
- *Important:* Connect a 100 µF electrolytic capacitor across `VMOT` and motor `GND` to protect the driver.

---

## 3. SG90 Servo Connections

- `Orange / Yellow (Signal)` -> Arduino **D9**
- `Red (V+)` -> External 5V Power Supply (DO NOT power high-draw servo directly from Arduino 5V pin during motor movement)
- `Brown / Black (GND)` -> Common GND

---

## 4. Libraries Required in Arduino IDE

Open **Arduino IDE** -> **Tools** -> **Manage Libraries...** and install:
1. **AccelStepper** (by Mike McCauley)
2. **RTClib** (by Adafruit)
3. **Adafruit SSD1306** (by Adafruit)
4. **Adafruit GFX Library** (by Adafruit)
5. **Servo** (Built-in standard library)

