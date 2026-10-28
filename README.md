# Smart APD — Smart Automated Pill Dispenser

**Project Type:** Major Project — PT 331  
**Project Name:** Smart Automated Pill Dispenser (Smart APD)  
**Domain:** Embedded Systems, IoT, and Healthcare Automation  
**Status:** Prototype Development

## 1. Overview

Smart APD is a smart medication dispensing system designed to automate scheduled pill dispensing and provide medication reminders. The system uses a microcontroller, a real-time clock (RTC), a stepper motor-driven dispensing mechanism, and sensors to support scheduled operation and pill passage verification.

The project aims to reduce missed medication schedules, improve dispensing consistency, and provide a foundation for future medication tracking and monitoring.

## 2. Objectives

- Automate medication dispensing according to predefined schedules.
- Maintain accurate time using a dedicated RTC module.
- Provide audible reminders using a buzzer.
- Control the dispensing mechanism using a stepper motor.
- Detect the home position of the dispensing mechanism.
- Verify pill passage using an infrared sensor.
- Display system information through an OLED display.
- Store important system settings persistently.
- Provide a foundation for future dashboard and database integration.

## 3. Proposed System Architecture

The system follows a modular architecture consisting of the following components:

| Module | Component | Responsibility |
|---|---|---|
| Controller | ESP32 DevKit | Coordinates system operations |
| Timekeeping | DS3231 RTC | Maintains real-time clock information |
| Display | OLED SSD1306 | Displays system information |
| Motor control | A4988 driver | Controls stepper motor movement |
| Dispensing mechanism | NEMA 17 stepper motor | Moves the dispensing assembly |
| Gate control | Servo motor | Controls the dispensing gate |
| Position detection | IR home sensor | Detects the mechanism's home position |
| Pill detection | IR passage sensor | Detects pill passage |
| Alert system | Buzzer | Provides audible reminders |
| User input | Push buttons | Supports basic user interaction |
| Persistent storage | ESP32 Preferences | Stores selected system settings |

### System Workflow

1. The ESP32 initializes the connected hardware modules.
2. The DS3231 RTC provides the current date and time.
3. The firmware checks the configured medication schedule.
4. When a scheduled dispensing event is due, the dispensing mechanism is commanded to move.
5. The servo controls the dispensing gate.
6. The IR passage sensor is used to detect pill passage.
7. The buzzer provides an audible indication when required.
8. The system records relevant event information using the implemented firmware logic and available persistent storage.

**Note:** This workflow describes the intended system operation. Complete physical operation and sensor reliability must be confirmed through hardware testing.

## 4. Hardware Requirements

The proposed hardware includes:

- ESP32 DevKit development board
- DS3231 RTC module
- SSD1306 OLED display
- A4988 stepper motor driver
- NEMA 17 stepper motor
- Servo motor
- IR home-position sensor
- IR pill-passage sensor
- Buzzer
- Push buttons
- Suitable regulated power supply
- Jumper wires and connecting cables
- Breadboard or appropriate prototyping connections
- 100 µF capacitor for the stepper driver's motor supply
- Pill storage and dispensing mechanism

Additional mounting hardware and mechanical components may be required depending on the final dispenser design.

## 5. Circuit Connections

The following pin assignments are used in the proposed firmware configuration.

| Component | Signal | ESP32 Pin |
|---|---|---|
| DS3231 RTC | SDA | GPIO 21 |
| DS3231 RTC | SCL | GPIO 22 |
| OLED SSD1306 | SDA | GPIO 21 |
| OLED SSD1306 | SCL | GPIO 22 |
| A4988 driver | STEP | GPIO 26 |
| A4988 driver | DIR | GPIO 27 |
| A4988 driver | ENABLE | GPIO 25 |
| Servo motor | Control signal | GPIO 18 |
| IR home sensor | Digital input | GPIO 35 |
| IR passage sensor | Digital input | GPIO 34 |
| Buzzer | Control signal | GPIO 19 |
| Push button 1 | Input | GPIO 32 |
| Push button 2 | Input | GPIO 33 |

The RTC and OLED share the I²C communication bus.

### Wiring Considerations

- Connect the ESP32 and peripheral modules according to their voltage and signal requirements.
- Supply the NEMA 17 motor through the A4988 driver using a suitable motor power supply.
- The proposed motor supply is 12 V, connected to the driver's VMOT input.
- Connect the motor supply ground and ESP32 ground appropriately.
- Install the 100 µF capacitor across the motor supply connections near VMOT and GND, observing correct polarity.
- Power the servo using a suitable external 5 V supply when required by its current demand.
- Ensure that the servo signal ground and ESP32 ground share a common reference.
- Check the electrical characteristics and output voltage of both IR sensors before connecting them to the ESP32.
- Configure the A4988 current limit according to the stepper motor's specifications.

**Important:** Verify all connections before powering the circuit. Do not connect the 12 V motor supply directly to the ESP32's GPIO pins or 3.3 V supply.

## 6. Software and Technologies

### Embedded Firmware

The firmware is designed for the ESP32 and uses the following libraries:

| Library | Purpose |
|---|---|
| RTClib | RTC communication |
| AccelStepper | Stepper motor control |
| ESP32Servo | Servo motor control |
| Adafruit SSD1306 | OLED display support |
| Adafruit GFX Library | Graphics and display rendering |
| Preferences | Persistent configuration storage |

### Development Environment

- Visual Studio Code
- PlatformIO
- Arduino framework for ESP32
- C++

### Optional Backend and Dashboard

The project structure also provides for future software expansion.

- **Backend:** Node.js
- **Database:** Supabase PostgreSQL, if integrated
- **Dashboard:** Web-based monitoring interface, if implemented

Backend APIs, database connectivity, and dashboard functionality should be considered separate components until their integration and operation have been verified.

## 7. Project Structure

The starter project is organized into three main directories.

```text
smart-apd/
├── firmware/
│   ├── platformio.ini
│   ├── src/
│   │   └── main.cpp
│   └── include/
│
├── backend/
│   ├── package.json
│   └── src/
│
├── dashboard/
│   └── README.md
│
└── README.md
```

The structure represents the intended organization of the project. Additional files, configuration files, and source modules should be included as they are created.

## 8. Firmware Configuration

The firmware design includes configuration for an eight-position dispensing mechanism.

The starter configuration includes:

- Eight dispensing positions.
- A nominal movement of 200 motor steps per position.
- A demonstration schedule at 08:00 for slot 0.
- A demonstration schedule at 20:00 for slot 1.
- Homing logic for establishing the mechanism's reference position.
- Servo-controlled gate operation.
- IR sensor-based pill passage verification.
- Persistent event-key handling.
- A system readiness check and a 15-second homing timeout.

These are initial configuration values, not measured performance results. The actual movement per position depends on the motor, driver configuration, mechanical arrangement, and any microstepping settings.

The demonstration schedules must be replaced with the intended medication schedules before practical use.

## 9. Installation and Setup

### Prerequisites

Install the following tools:

- Visual Studio Code
- PlatformIO IDE extension
- Appropriate USB drivers, if required
- USB cable compatible with the ESP32 board

### Firmware Setup

1. Clone or download the project repository.
2. Open the `firmware` directory in Visual Studio Code.
3. Open the project using PlatformIO.
4. Verify the board configuration and library dependencies in `platformio.ini`.
5. Review the GPIO assignments and hardware connections.
6. Connect the ESP32 to the computer.
7. Build the firmware.
8. Upload the firmware to the board when the hardware is ready.
9. Open the serial monitor to inspect startup messages and debugging output.

**Note:** Successful compilation, firmware upload, and physical hardware operation must be verified independently. No successful end-to-end test is claimed by this documentation.

### Backend Setup

If the backend implementation is present, install its dependencies from the backend directory:

```bash
cd backend
npm install
```

Start the backend using the script configured in `package.json`.

The exact startup command depends on the scripts available in the repository.

### Dashboard Setup

Dashboard dependencies and startup commands should be documented according to the actual frontend implementation.

The dashboard should not be considered operational until its code, configuration, and connectivity have been verified.

## 10. Current Implementation Status

### Work Completed

- Defined the overall Smart APD architecture.
- Identified the proposed hardware components.
- Prepared the initial circuit connection plan.
- Established a preliminary firmware configuration.
- Defined the intended dispensing and reminder workflow.
- Identified the main software modules and their responsibilities.

### Work in Progress or Pending

- Hardware assembly and circuit wiring.
- Firmware compilation and debugging on the target board.
- Integration of the RTC, OLED, stepper motor, servo, and IR sensors.
- Verification of the homing and dispensing mechanisms.
- Testing of scheduled dispensing and buzzer reminders.
- Validation of pill passage detection.
- End-to-end system testing.
- Optional backend, database, and dashboard integration.

The project remains in the prototype development stage until the hardware and software are integrated and validated.

## 11. Testing and Validation

The following tests are planned for the validation phase.

| Test Case | Purpose | Current Status |
|---|---|---|
| RTC initialization | Verify communication with the DS3231 | Pending |
| Timekeeping | Check RTC time accuracy | Pending |
| OLED operation | Verify display initialization and output | Pending |
| Motor movement | Verify controlled movement between positions | Pending |
| Homing | Check reference-position detection | Pending |
| Gate operation | Verify servo-controlled dispensing | Pending |
| Pill passage detection | Evaluate IR sensor response | Pending |
| Medication reminders | Verify scheduled buzzer alerts | Pending |
| Repeated dispensing | Assess consistency across multiple cycles | Pending |
| Integrated operation | Evaluate the complete system | Pending |

No measured accuracy, latency, dispensing success rate, or reliability figures are currently available.

Performance metrics should be reported only after the corresponding tests have been performed and their results recorded.

## 12. Pending Milestones

The remaining work includes:

1. Assemble the hardware and complete circuit wiring.
2. Implement and debug the ESP32 firmware.
3. Integrate the RTC, dispensing mechanism, sensors, display, and buzzer.
4. Implement and verify scheduled dispensing logic.
5. Test medication reminders and pill passage detection.
6. Measure dispensing accuracy and timing reliability.
7. Perform repeated-cycle and end-to-end testing.
8. Update the documentation using actual experimental results.
9. Explore Supabase integration for medication schedules and dispensing records, if required.

## 13. Future Enhancements

Potential future improvements include:

- A web dashboard for medication schedule management.
- Supabase-based storage of medication schedules and dispensing events.
- Remote monitoring of device status.
- Improved handling of missed dispensing events.
- Low-pill-level detection, if suitable hardware is added.
- Fault detection and recovery mechanisms.
- Improved dispensing verification and reliability reporting.
- User-configurable medication schedules.

These enhancements are proposed possibilities and are not claimed to be implemented.

## 14. Safety Considerations

This project is a prototype intended for educational and engineering development.

Before any practical medication use, the dispensing mechanism must be validated for correct pill selection, dosage quantity, timing, and prevention of accidental or repeated dispensing.

Sensor failures, power interruptions, mechanical jams, incorrect schedules, and missed dispensing events must be considered during testing.

The prototype should not be relied upon for medication management until its safety and reliability have been appropriately validated.

## 15. Conclusion

Smart APD proposes an embedded solution for scheduled medication dispensing using an ESP32, RTC, motorized dispensing mechanism, sensors, and audible reminders.

The project establishes a modular foundation for automated dispensing and future software integration. The next development phase focuses on hardware assembly, firmware integration, functional testing, and measurement-based validation.

## 16. License

A suitable open-source license can be added after the project owner selects the licensing terms.
