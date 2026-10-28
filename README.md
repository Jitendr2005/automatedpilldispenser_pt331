# Smart APD — Smart Automated Pill Dispenser (PT 331)

Educational prototype starter repository for an ESP32-based, eight-slot carousel dispenser.

## Repository layout

```text
smart-apd/
├── firmware/
│   ├── platformio.ini
│   ├── include/config.h
│   └── src/main.cpp
├── backend/
│   ├── package.json
│   ├── .env.example
│   ├── schema.sql
│   └── src/
│       ├── server.js
│       ├── db.js
│       └── routes/events.js
├── dashboard/
│   ├── package.json
│   ├── index.html
│   └── src/
│       ├── main.jsx
│       ├── App.jsx
│       └── style.css
└── docs/
    └── WIRING.md
```

## 1. Hardware assumed

- ESP32 DevKit V1
- DS3231 RTC
- A4988 stepper driver + NEMA 17 motor
- 8-slot carousel
- Servo-operated gate
- SSD1306 128×64 I²C OLED
- Home-position sensor and chute/passage sensor
- Active buzzer and acknowledgement button
- Optional HX711 + load cell (not enabled in this starter)
- Separate suitable motor and servo power supplies

Pin assignments are in `firmware/include/config.h`. Confirm your actual modules and wiring before powering the circuit.

## 2. Firmware

This repo uses PlatformIO. Install VS Code + PlatformIO, open the `firmware` folder, then build/upload.

Libraries are declared in `platformio.ini`. Configure the RTC once using the `SET_RTC_ONCE` instructions in `main.cpp`, upload, then remove/disable the one-time adjustment and upload again.

Edit the demonstration schedule in `config.h`. Slots are numbered 0–7. Calibrate `STEPS_PER_SLOT`, direction, sensor polarity and gate angles for your physical mechanism.

## 3. Backend

Requires Node.js 20+ and PostgreSQL.

```bash
cd backend
cp .env.example .env
npm install
# Create a PostgreSQL database and apply schema:
psql "$DATABASE_URL" -f schema.sql
npm run dev
```

API:
- `GET /api/health`
- `GET /api/events?limit=50`
- `POST /api/events` — append an event
- `GET /api/summary`

The API is a development scaffold. Add authentication, authorization, TLS, rate limiting, input validation, audit controls and device identity before exposing it outside localhost.

## 4. Dashboard

```bash
cd dashboard
npm install
npm run dev
```

Vite uses `/api` as a development proxy to `http://localhost:3000`. Start the backend first. Dashboard currently shows events and a basic summary; schedule management and login are not implemented.

## 5. Wiring

See `docs/WIRING.md`.

## Safety / scope

This is a student demonstration prototype, not a validated medical device. Test with inert objects only. An IR beam interruption does not prove pill identity or dose quantity. The starter firmware deliberately does not automatically retry a scheduled event after a failure. It also needs additional engineering before real medication use: robust power-loss recovery, physical interlocks, tamper protection, validated dose verification, schedule configuration and review, authenticated communications, watchdog/fault recovery, and clinical/regulatory validation.
