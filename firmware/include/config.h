#pragma once

// Pin mapping — verify against the actual board and modules.
constexpr int PIN_I2C_SDA = 21;
constexpr int PIN_I2C_SCL = 22;

constexpr int PIN_STEP = 26;
constexpr int PIN_DIR = 27;
constexpr int PIN_ENABLE = 25;  // A4988 ENABLE is active LOW

constexpr int PIN_SERVO = 18;
constexpr int PIN_HOME_SENSOR = 35;
constexpr int PIN_PASSAGE_SENSOR = 34;
constexpr int PIN_BUZZER = 19;
constexpr int PIN_ACK_BUTTON = 32;

constexpr uint8_t OLED_I2C_ADDRESS = 0x3C;
constexpr int OLED_WIDTH = 128;
constexpr int OLED_HEIGHT = 64;

// Adjust for your sensor modules: many IR obstacle boards are active LOW.
constexpr int SENSOR_ACTIVE_LEVEL = LOW;

// Mechanical calibration: 200-step motor, full-step mode, direct 1:1 drive.
// Change this for microstepping, gearing, or a different motor.
constexpr long MOTOR_FULL_STEPS_PER_REV = 200;
constexpr int CAROUSEL_SLOT_COUNT = 8;
constexpr long STEPS_PER_SLOT =
    MOTOR_FULL_STEPS_PER_REV / CAROUSEL_SLOT_COUNT;

constexpr float MOTOR_MAX_SPEED = 500.0;
constexpr float MOTOR_ACCELERATION = 200.0;
constexpr float HOMING_SPEED = -150.0;
constexpr unsigned long HOMING_TIMEOUT_MS = 15000;

constexpr int GATE_CLOSED_DEGREES = 10;
constexpr int GATE_OPEN_DEGREES = 90;
constexpr unsigned long GATE_SETTLE_MS = 500;
constexpr unsigned long PASSAGE_TIMEOUT_MS = 5000;

// Demo-only daily schedule. Slot numbering is 0–7.
// Change to your own test schedule before demo.
struct DoseSchedule {
  uint8_t hour;
  uint8_t minute;
  uint8_t slot;
};
constexpr DoseSchedule DOSE_SCHEDULE[] = {
  {8, 0, 0},
  {20, 0, 1},
};
constexpr size_t DOSE_SCHEDULE_COUNT =
    sizeof(DOSE_SCHEDULE) / sizeof(DOSE_SCHEDULE[0]);

// Backend integration is disabled by default.
// Enable only after adding Wi-Fi credentials and a secured API.
constexpr bool ENABLE_BACKEND_POST = false;
