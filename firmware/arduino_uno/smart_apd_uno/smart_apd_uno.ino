/*
 * Smart APD — Automated Pill Dispenser (Arduino Uno ATmega328P Edition)
 * PT 331 · Major Project
 *
 * Designed specifically for Arduino Uno (uses standard <Servo.h>, <EEPROM.h>,
 * and SRAM-saving F() macros for ATmega328P 2KB RAM stability with SSD1306).
 *
 * Arduino Uno Pinout:
 *   - A4: I2C SDA (OLED SSD1306 + DS3231 RTC)
 *   - A5: I2C SCL (OLED SSD1306 + DS3231 RTC)
 *   - D2: Stepper STEP (A4988)
 *   - D3: Stepper DIR (A4988)
 *   - D4: Stepper ENABLE (A4988 - Active LOW)
 *   - D5: Home Position Sensor (IR / Optical limit)
 *   - D6: Chute / Passage Sensor (IR obstacle beam)
 *   - D7: Acknowledge Pushbutton (to GND, internal pull-up)
 *   - D8: Active Buzzer
 *   - D9: Gate Servo Signal (PWM)
 */

#include <AccelStepper.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include <RTClib.h>
#include <Servo.h>
#include <Wire.h>

// ================= Pin Configuration for Arduino Uno =================
const int PIN_STEP = 2;
const int PIN_DIR = 3;
const int PIN_ENABLE = 4; // A4988 ENABLE (LOW = Driver enabled)
const int PIN_HOME_SENSOR = 5;
const int PIN_PASSAGE_SENSOR = 6;
const int PIN_ACK_BUTTON = 7;
const int PIN_BUZZER = 8;
const int PIN_SERVO = 9;

// ================= OLED Configuration =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ================= Hardware Instances =================
RTC_DS3231 rtc;
AccelStepper stepper(AccelStepper::DRIVER, PIN_STEP, PIN_DIR);
Servo gate;

// ================= Parameters & Calibration =================
const int SENSOR_ACTIVE_LEVEL =
    LOW; // Most IR modules output LOW when obstacle detected
const long MOTOR_FULL_STEPS_PER_REV = 200;
const int CAROUSEL_SLOT_COUNT = 8;
const long STEPS_PER_SLOT =
    MOTOR_FULL_STEPS_PER_REV / CAROUSEL_SLOT_COUNT; // 25 steps per slot

const float MOTOR_MAX_SPEED = 400.0;
const float MOTOR_ACCELERATION = 200.0;
const float HOMING_SPEED = -120.0;
const unsigned long HOMING_TIMEOUT_MS = 12000;

const int GATE_CLOSED_DEGREES = 10;
const int GATE_OPEN_DEGREES = 90;
const unsigned long GATE_SETTLE_MS = 500;
const unsigned long PASSAGE_TIMEOUT_MS = 4000;

// Demonstration Schedule (Slots 0 to 7)
// Modify these demonstration times to test specific dose triggers:
struct DoseSchedule {
  uint8_t hour;
  uint8_t minute;
  uint8_t slot;
};

const DoseSchedule DOSE_SCHEDULE[] = {
    {8, 0, 0},  // Morning dose - Slot 0
    {12, 0, 1}, // Afternoon dose - Slot 1
    {18, 0, 2}, // Evening dose - Slot 2
    {21, 0, 3}  // Night dose - Slot 3
};
const size_t DOSE_SCHEDULE_COUNT =
    sizeof(DOSE_SCHEDULE) / sizeof(DOSE_SCHEDULE[0]);

// ================= System State =================
bool rtcReady = false;
bool oledReady = false;
bool systemReady = false;
bool faultLatched = false;
uint8_t lastDispensedHour = 255;
uint8_t lastDispensedMinute = 255;

// ================= Helper Functions =================
void showStatus(const __FlashStringHelper *title, const char *detail = "") {
  Serial.print(F("[STATUS] "));
  Serial.print(title);
  if (detail && detail[0] != '\0') {
    Serial.print(F(" | "));
    Serial.print(detail);
  }
  Serial.println();

  if (!oledReady)
    return;
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println(title);
  display.setCursor(0, 32);
  display.println(detail);
  display.display();
}

void buzzerBeep(unsigned int durationMs = 200) {
  digitalWrite(PIN_BUZZER, HIGH);
  delay(durationMs);
  digitalWrite(PIN_BUZZER, LOW);
}

void alertUser(const __FlashStringHelper *message) {
  faultLatched = true;
  Serial.print(F("[ALERT] "));
  Serial.println(message);

  if (oledReady) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 8);
    display.println(F("! FAULT DETECTED !"));
    display.setCursor(0, 28);
    display.println(message);
    display.setCursor(0, 48);
    display.println(F("Press Button to Clear"));
    display.display();
  }

  for (int i = 0; i < 3; ++i) {
    buzzerBeep(150);
    delay(100);
  }
}

bool homeCarousel() {
  showStatus(F("Homing carousel..."), "Please wait");

  if (digitalRead(PIN_HOME_SENSOR) == SENSOR_ACTIVE_LEVEL) {
    stepper.setCurrentPosition(0);
    return true;
  }

  stepper.setSpeed(HOMING_SPEED);
  unsigned long started = millis();

  while (digitalRead(PIN_HOME_SENSOR) != SENSOR_ACTIVE_LEVEL &&
         (millis() - started < HOMING_TIMEOUT_MS)) {
    stepper.runSpeed();
    delay(1);
  }

  if (digitalRead(PIN_HOME_SENSOR) != SENSOR_ACTIVE_LEVEL) {
    stepper.stop();
    alertUser(F("Home sensor timeout"));
    return false;
  }

  stepper.setCurrentPosition(0);
  return true;
}

bool moveToSlot(uint8_t slot) {
  if (slot >= CAROUSEL_SLOT_COUNT) {
    alertUser(F("Invalid slot index"));
    return false;
  }

  if (!homeCarousel())
    return false;

  const long target = (long)slot * STEPS_PER_SLOT;
  stepper.moveTo(target);

  char buf[16];
  snprintf(buf, sizeof(buf), "Slot %u", slot);
  showStatus(F("Moving to slot"), buf);

  unsigned long started = millis();
  while (stepper.distanceToGo() != 0) {
    stepper.run();
    if (millis() - started > 15000UL) {
      stepper.stop();
      alertUser(F("Motor timeout"));
      return false;
    }
    delay(1);
  }
  return true;
}

bool passageSensorClear() {
  return digitalRead(PIN_PASSAGE_SENSOR) != SENSOR_ACTIVE_LEVEL;
}

bool dispenseSlot(uint8_t slot) {
  if (!systemReady || faultLatched)
    return false;
  if (!moveToSlot(slot))
    return false;

  if (!passageSensorClear()) {
    alertUser(F("Chute blocked!"));
    return false;
  }

  showStatus(F("Gate opening..."), "Dispensing dose");
  gate.write(GATE_OPEN_DEGREES);
  delay(GATE_SETTLE_MS);

  unsigned long started = millis();
  bool passageDetected = false;

  while (millis() - started < PASSAGE_TIMEOUT_MS) {
    if (digitalRead(PIN_PASSAGE_SENSOR) == SENSOR_ACTIVE_LEVEL) {
      passageDetected = true;
      break;
    }
    delay(5);
  }

  gate.write(GATE_CLOSED_DEGREES);
  delay(300);

  if (!passageDetected) {
    alertUser(F("No passage detected"));
    return false;
  }

  showStatus(F("Passage detected!"), "Dose dispensed OK");
  buzzerBeep(250);
  return true;
}

void checkSchedule() {
  if (!rtcReady || !systemReady || faultLatched)
    return;

  DateTime now = rtc.now();
  for (size_t i = 0; i < DOSE_SCHEDULE_COUNT; ++i) {
    const DoseSchedule &event = DOSE_SCHEDULE[i];
    if (now.hour() == event.hour && now.minute() == event.minute) {
      if (lastDispensedHour == now.hour() &&
          lastDispensedMinute == now.minute()) {
        continue; // Already serviced this minute
      }

      lastDispensedHour = now.hour();
      lastDispensedMinute = now.minute();

      const bool success = dispenseSlot(event.slot);

      // JSON Telemetry for Backend / Serial monitor
      Serial.print(F("{\"event\":\"scheduled_dispense\",\"slot\":"));
      Serial.print(event.slot);
      Serial.print(F(",\"passage_detected\":"));
      Serial.print(success ? F("true") : F("false"));
      Serial.println(F(",\"review_required\":true}"));

      if (success) {
        showStatus(F("Dose Dispensed"), "Record updated");
      } else {
        showStatus(F("Dose Failed"), "Review required");
      }
    }
  }
}

// ================= Setup =================
void setup() {
  Serial.begin(115200);
  Serial.println(F("\n--- Smart APD Starting (Arduino Uno) ---"));

  pinMode(PIN_ENABLE, OUTPUT);
  digitalWrite(PIN_ENABLE, LOW); // A4988 enabled (Active LOW)

  pinMode(PIN_HOME_SENSOR, INPUT_PULLUP);
  pinMode(PIN_PASSAGE_SENSOR, INPUT_PULLUP);
  pinMode(PIN_ACK_BUTTON, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);

  Wire.begin();

  oledReady = display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS);
  if (oledReady) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 16);
    display.println(F("Smart APD Initializing"));
    display.display();
  }

  gate.attach(PIN_SERVO);
  gate.write(GATE_CLOSED_DEGREES);

  stepper.setMaxSpeed(MOTOR_MAX_SPEED);
  stepper.setAcceleration(MOTOR_ACCELERATION);

  rtcReady = rtc.begin();
  if (!rtcReady) {
    alertUser(F("RTC not detected!"));
    return;
  }

  if (rtc.lostPower()) {
    Serial.println(F("[RTC] Lost power; adjusting RTC to compile time."));
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  if (!homeCarousel()) {
    return;
  }

  systemReady = true;
  showStatus(F("Smart APD Ready"), "Schedule Active");
  buzzerBeep(100);
}

// ================= Loop =================
void loop() {
  // Check if acknowledge button is pressed to clear latched faults
  if (faultLatched) {
    if (digitalRead(PIN_ACK_BUTTON) == LOW) {
      delay(50); // Debounce
      if (digitalRead(PIN_ACK_BUTTON) == LOW) {
        faultLatched = false;
        buzzerBeep(100);
        showStatus(F("Fault Cleared"), "Homing carousel...");
        homeCarousel();
        showStatus(F("Smart APD Ready"), "Schedule Active");
      }
    }
    delay(100);
    return;
  }

  checkSchedule();

  // Update OLED with current clock time every second
  static unsigned long lastClockUpdate = 0;
  if (millis() - lastClockUpdate >= 1000) {
    lastClockUpdate = millis();
    if (rtcReady && systemReady) {
      DateTime now = rtc.now();
      char timeStr[16];
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", now.hour(),
               now.minute(), now.second());
      showStatus(F("Smart APD"), timeStr);
    }
  }

  // Optional manual test via Serial Monitor: type 'd0', 'd1' ... 'd7' or 'h'
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'h') {
      homeCarousel();
    } else if (cmd == 'd') {
      int slot = Serial.parseInt();
      if (slot >= 0 && slot < CAROUSEL_SLOT_COUNT) {
        Serial.print(F("Manual dispense slot "));
        Serial.println(slot);
        dispenseSlot(slot);
      }
    }
  }

  delay(20);
}
