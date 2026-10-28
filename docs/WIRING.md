# Wiring reference

All grounds for ESP32, A4988 logic, motor supply negative and external servo supply negative must be common. Keep the high-current motor/servo power wiring away from sensor signal wiring.

## ESP32 / I²C
| Signal | ESP32 |
|---|---|
| DS3231 SDA + OLED SDA | GPIO 21 |
| DS3231 SCL + OLED SCL | GPIO 22 |
| I²C module VCC | 3V3 when supported by the module |
| I²C GND | GND |

Never allow I²C pull-ups to pull ESP32 pins to 5 V.

## A4988
| A4988 | Connection |
|---|---|
| VDD | ESP32 3V3 |
| logic GND | common GND |
| STEP | GPIO 26 |
| DIR | GPIO 27 |
| ENABLE | GPIO 25 (active LOW) |
| RESET + SLEEP | tied together, then to 3V3 |
| MS1/MS2/MS3 | GND for full-step mode |
| VMOT | suitable regulated motor supply (example: 12 V) |
| motor GND | motor supply negative / common GND |
| 1A/1B | one coil pair |
| 2A/2B | other coil pair |

Place a suitably voltage-rated electrolytic capacitor (e.g. 100 µF, rated above the supply voltage) close across VMOT and motor GND. Set current limit to match the motor and driver cooling. Never plug/unplug a stepper motor while powered.

## Other peripherals
| Component | Connection |
|---|---|
| Servo signal | GPIO 18 |
| Servo V+ | external regulated 5 V supply sized for servo current |
| Servo GND | common GND |
| Home sensor output | GPIO 35 |
| Chute/passage sensor output | GPIO 34 |
| Active buzzer control | GPIO 19 (use transistor if current exceeds GPIO rating) |
| Acknowledge button | GPIO 32 to GND, internal pull-up |
| OLED / RTC | shared I²C bus above |

GPIO 34 and 35 are input-only and have no internal pull-ups. Add external pull-ups if your sensor outputs need them. Sensor outputs must be safe for 3.3 V logic. Confirm whether each sensor is active LOW or active HIGH and update config.

## Power
Use separate appropriately rated supplies for motor and servo. Common ground is required. Do not power a NEMA 17 from the ESP32 3V3/5V pin.
