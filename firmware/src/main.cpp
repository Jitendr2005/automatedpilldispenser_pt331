#include "config.h"
#include <AccelStepper.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <ESP32Servo.h>
#include <Preferences.h>
#include <RTClib.h>
#include <Wire.h>

RTC_DS3231 rtc;
AccelStepper stepper(AccelStepper::DRIVER, PIN_STEP, PIN_DIR);
Servo gate;
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
Preferences preferences;

bool rtcReady = false;
bool oledReady = false;
bool systemReady = false;
bool faultLatched = false;

void showStatus(const String &title, const String &detail = "") {
  Serial.printf("[STATUS] %s | %s\n", title.c_str(), detail.c_str());
  if (!oledReady)
    return;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 12);
  display.println(title);
  display.setCursor(0, 32);
  display.println(detail);
  display.display();
}

void buzzerBeep(unsigned int durationMs = 250) {
  digitalWrite(PIN_BUZZER, HIGH);
  delay(durationMs);
  digitalWrite(PIN_BUZZER, LOW);
}

void alertUser(const String &message) {
  faultLatched = true;
  showStatus("FAULT - REVIEW", message);
  for (int i = 0; i < 3; ++i) {
    buzzerBeep(180);
    delay(120);
  }
}

bool homeCarousel() {
  showStatus("Homing carousel", "Keep hands clear");

  if (digitalRead(PIN_HOME_SENSOR) == SENSOR_ACTIVE_LEVEL) {
    stepper.setCurrentPosition(0);
    return true;
  }

  stepper.setSpeed(HOMING_SPEED);
  const unsigned long started = millis();

  while (digitalRead(PIN_HOME_SENSOR) != SENSOR_ACTIVE_LEVEL &&
         millis() - started < HOMING_TIMEOUT_MS) {
    stepper.runSpeed();
    delay(1);
  }

  if (digitalRead(PIN_HOME_SENSOR) != SENSOR_ACTIVE_LEVEL) {
    stepper.stop();
    alertUser("Home sensor timeout");
    return false;
  }

  stepper.setCurrentPosition(0);
  return true;
}

bool moveToSlot(uint8_t slot) {
  if (slot >= CAROUSEL_SLOT_COUNT) {
    alertUser("Invalid slot index");
    return false;
  }

  // Always approach from home to make the demo position deterministic.
  if (!homeCarousel())
    return false;

  const long target = static_cast<long>(slot) * STEPS_PER_SLOT;
  stepper.moveTo(target);
  showStatus("Moving carousel", "Slot " + String(slot));

  const unsigned long started = millis();
  while (stepper.distanceToGo() != 0) {
    stepper.run();
    if (millis() - started > 30000UL) {
      stepper.stop();
      alertUser("Motor movement timeout");
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
    alertUser("Chute blocked before dose");
    return false;
  }

  showStatus("Gate opening", "Observe test chute");
  gate.write(GATE_OPEN_DEGREES);
  delay(GATE_SETTLE_MS);

  const unsigned long started = millis();
  bool passageDetected = false;

  while (millis() - started < PASSAGE_TIMEOUT_MS) {
    if (digitalRead(PIN_PASSAGE_SENSOR) == SENSOR_ACTIVE_LEVEL) {
      passageDetected = true;
      break;
    }
    delay(5);
  }

  gate.write(GATE_CLOSED_DEGREES);
  delay(400);

  if (!passageDetected) {
    alertUser("No passage detected");
    return false;
  }

  showStatus("Passage detected", "Event requires review");
  buzzerBeep(250);
  return true;
}

String eventKeyFor(const DateTime &now, size_t scheduleIndex) {
  char key[32];
  snprintf(key, sizeof(key), "%04d%02d%02d_%u", now.year(), now.month(),
           now.day(), static_cast<unsigned>(scheduleIndex));
  return String(key);
}

void checkSchedule() {
  if (!rtcReady || !systemReady || faultLatched)
    return;

  const DateTime now = rtc.now();
  for (size_t i = 0; i < DOSE_SCHEDULE_COUNT; ++i) {
    const auto &event = DOSE_SCHEDULE[i];
    if (now.hour() != event.hour || now.minute() != event.minute)
      continue;

    const String key = eventKeyFor(now, i);
    if (preferences.getString("last_event", "") == key)
      continue;

    // Save before moving. This prevents an unintentional automatic retry
    // after a reboot, but requires a human review if the event fails.
    preferences.putString("last_event", key);
    const bool success = dispenseSlot(event.slot);

    Serial.printf("{\"event\":\"scheduled_dispense\",\"slot\":%u,"
                  "\"passage_detected\":%s,\"review_required\":true}\n",
                  event.slot, success ? "true" : "false");

    if (success) {
      showStatus("Passage detected", "Review event record");
    } else {
      showStatus("Event failed", "Manual review required");
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_ENABLE, OUTPUT);
  digitalWrite(PIN_ENABLE, LOW); // A4988 enabled
  pinMode(PIN_HOME_SENSOR, INPUT);
  pinMode(PIN_PASSAGE_SENSOR, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_ACK_BUTTON, INPUT_PULLUP);
  digitalWrite(PIN_BUZZER, LOW);

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

  oledReady = display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS);
  gate.setPeriodHertz(50);
  gate.attach(PIN_SERVO, 500, 2400);
  gate.write(GATE_CLOSED_DEGREES);

  stepper.setMaxSpeed(MOTOR_MAX_SPEED);
  stepper.setAcceleration(MOTOR_ACCELERATION);
  preferences.begin("smartapd", false);

  rtcReady = rtc.begin();
  if (!rtcReady) {
    alertUser("RTC not detected");
    return;
  }

  // Set RTC once if required:
  // 1. Uncomment the next line and upload.
  // 2. Comment it again and upload a second time.
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  if (rtc.lostPower()) {
    alertUser("RTC lost power; set time");
    return;
  }

  if (!homeCarousel())
    return;

  systemReady = true;
  showStatus("Smart APD Ready", "Schedule active");
}

void loop() {
  if (!rtcReady || !systemReady || faultLatched) {
    delay(100);
    return;
  }

  checkSchedule();

  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate >= 1000) {
    lastUpdate = millis();
    const DateTime now = rtc.now();
    char timeText[24];
    snprintf(timeText, sizeof(timeText), "%02d:%02d:%02d", now.hour(),
             now.minute(), now.second());
    showStatus("Smart APD", timeText);
  }

  delay(50);
}
