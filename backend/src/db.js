import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import pg from "pg";

const { Pool } = pg;
const __dirname = path.dirname(fileURLToPath(import.meta.url));
const dataDir = path.resolve(__dirname, "../data");
const dataFilePath = path.join(dataDir, "events.json");

if (!fs.existsSync(dataDir)) {
  fs.mkdirSync(dataDir, { recursive: true });
}

function generateDefaultEvents() {
  const now = Date.now();
  const m = (min) => new Date(now - min * 60 * 1000).toISOString();
  const h = (hrs) => new Date(now - hrs * 3600 * 1000).toISOString();

  return [
    {
      id: 1,
      device_id: "smart-apd-01",
      event_type: "power_on_self_test",
      slot: null,
      success: true,
      message: "ESP32 POST complete. RTC DS3231 synced, OLED ready, stepper initialized.",
      details: { firmware: "1.0.0", rtc: "DS3231", oled: "SSD1306_128x64", battery: "OK" },
      created_at: h(24)
    },
    {
      id: 2,
      device_id: "smart-apd-01",
      event_type: "carousel_homing",
      slot: null,
      success: true,
      message: "Optical home sensor calibrated. Stepper position set to 0.",
      details: { sensor_pin: 13, homing_steps: 42, status: "HOMED" },
      created_at: h(23.9)
    },
    {
      id: 3,
      device_id: "smart-apd-01",
      event_type: "scheduled_dispense",
      slot: 0,
      success: true,
      message: "Morning dose dispensed into chute. IR beam passage detected.",
      details: { medication: "Metformin 500mg", passage_time_ms: 122, schedule: "08:00" },
      created_at: h(22)
    },
    {
      id: 4,
      device_id: "smart-apd-01",
      event_type: "scheduled_dispense",
      slot: 1,
      success: true,
      message: "Afternoon dose dispensed. Passage sensor confirmed drop.",
      details: { medication: "Multivitamin", passage_time_ms: 135, schedule: "12:00" },
      created_at: h(18)
    },
    {
      id: 5,
      device_id: "smart-apd-01",
      event_type: "scheduled_dispense",
      slot: 2,
      success: true,
      message: "Evening dose dispensed. IR passage verified.",
      details: { medication: "Atorvastatin 20mg", passage_time_ms: 140, schedule: "18:00" },
      created_at: h(12)
    },
    {
      id: 6,
      device_id: "smart-apd-01",
      event_type: "chute_sensor_alert",
      slot: 3,
      success: false,
      message: "Passage timeout (3000ms). Chute blocked or pill failed to drop.",
      details: { error: "PASSAGE_TIMEOUT", buzzer_activated: true, review_required: true },
      created_at: h(6)
    },
    {
      id: 7,
      device_id: "smart-apd-01",
      event_type: "manual_acknowledgement",
      slot: 3,
      success: null,
      message: "User pressed tactile acknowledge button. Chute cleared.",
      details: { button_pin: 34, user_action: "ACK_BUTTON_PRESSED" },
      created_at: h(5.8)
    },
    {
      id: 8,
      device_id: "smart-apd-01",
      event_type: "manual_dispense_retry",
      slot: 3,
      success: true,
      message: "Manual retry dispense successful. IR passage verified (115ms).",
      details: { retry_attempt: 1, passage_time_ms: 115, medication: "Lisinopril 10mg" },
      created_at: h(5.7)
    },
    {
      id: 9,
      device_id: "smart-apd-01",
      event_type: "carousel_homing",
      slot: null,
      success: true,
      message: "Routine carousel position re-alignment complete.",
      details: { status: "HOMED", drift_steps: 0 },
      created_at: h(3)
    },
    {
      id: 10,
      device_id: "smart-apd-01",
      event_type: "scheduled_dispense",
      slot: 4,
      success: true,
      message: "Scheduled dose dispensed. IR passage confirmed.",
      details: { medication: "Amlodipine 5mg", passage_time_ms: 128, schedule: "08:00" },
      created_at: h(1.5)
    },
    {
      id: 11,
      device_id: "smart-apd-01",
      event_type: "scheduled_dispense",
      slot: 5,
      success: true,
      message: "Scheduled dose dispensed into chute. Gate closed.",
      details: { medication: "Calcium + D3", passage_time_ms: 130, schedule: "12:00" },
      created_at: m(45)
    },
    {
      id: 12,
      device_id: "smart-apd-01",
      event_type: "dispense_warning",
      slot: 6,
      success: false,
      message: "Gate angle sluggish during open cycle (exceeded settle window).",
      details: { error: "GATE_SERVO_SLOW", warning_latched: true },
      created_at: m(15)
    },
    {
      id: 13,
      device_id: "smart-apd-01",
      event_type: "scheduled_dispense",
      slot: 7,
      success: true,
      message: "Night dose dispensed successfully. All carousel slots serviced.",
      details: { medication: "Melatonin 3mg", passage_time_ms: 118, schedule: "21:00" },
      created_at: m(2)
    }
  ];
}

class LocalStore {
  constructor() {
    this.events = [];
    this.init();
  }

  init() {
    if (fs.existsSync(dataFilePath)) {
      try {
        const raw = fs.readFileSync(dataFilePath, "utf-8");
        this.events = JSON.parse(raw);
        if (Array.isArray(this.events) && this.events.length > 0) {
          return;
        }
      } catch (err) {
        console.warn("[LocalStore] Error reading existing events.json, re-seeding:", err.message);
      }
    }
    this.events = generateDefaultEvents();
    this.save();
  }

  save() {
    try {
      fs.writeFileSync(dataFilePath, JSON.stringify(this.events, null, 2), "utf-8");
    } catch (err) {
      console.error("[LocalStore] Failed to save events.json:", err.message);
    }
  }

  query(sql, params = []) {
    const trimmed = sql.trim();
    const upper = trimmed.toUpperCase();

    // Health check
    if (upper.startsWith("SELECT 1")) {
      return Promise.resolve({ rows: [{ "?column?": 1 }] });
    }

    // Summary query
    if (upper.includes("TOTAL_EVENTS") && upper.includes("SUCCESSFUL_EVENTS")) {
      const total = this.events.length;
      const successful = this.events.filter((e) => e.success === true).length;
      const failed = this.events.filter((e) => e.success === false).length;
      let lastEventAt = null;
      if (this.events.length > 0) {
        const sorted = [...this.events].sort(
          (a, b) => new Date(b.created_at).getTime() - new Date(a.created_at).getTime()
        );
        lastEventAt = sorted[0].created_at;
      }
      return Promise.resolve({
        rows: [
          {
            total_events: total,
            successful_events: successful,
            failed_events: failed,
            last_event_at: lastEventAt
          }
        ]
      });
    }

    // Select events query
    if (upper.startsWith("SELECT") && upper.includes("FROM DEVICE_EVENTS")) {
      const limit = typeof params[0] === "number" ? params[0] : 50;
      const sorted = [...this.events].sort(
        (a, b) => new Date(b.created_at).getTime() - new Date(a.created_at).getTime()
      );
      const rows = sorted.slice(0, limit);
      return Promise.resolve({ rows });
    }

    // Insert event query
    if (upper.startsWith("INSERT INTO DEVICE_EVENTS")) {
      const nextId = this.events.reduce((max, e) => Math.max(max, e.id || 0), 0) + 1;
      let parsedDetails = {};
      try {
        if (typeof params[5] === "string") {
          parsedDetails = JSON.parse(params[5]);
        } else if (params[5] && typeof params[5] === "object") {
          parsedDetails = params[5];
        }
      } catch {
        parsedDetails = {};
      }

      const newEvent = {
        id: nextId,
        device_id: params[0] || "smart-apd-01",
        event_type: params[1],
        slot: params[2] !== undefined && params[2] !== null ? Number(params[2]) : null,
        success: params[3] !== undefined && params[3] !== null ? Boolean(params[3]) : null,
        message: params[4] || null,
        details: parsedDetails,
        created_at: new Date().toISOString()
      };

      this.events.unshift(newEvent);
      this.save();
      return Promise.resolve({ rows: [newEvent] });
    }

    return Promise.resolve({ rows: [] });
  }
}

export const localStore = new LocalStore();

let pgPool = null;
let usePg = false;

if (process.env.DATABASE_URL && process.env.DATABASE_URL.trim() !== "") {
  try {
    const ssl = process.env.PG_SSL === "true" ? { rejectUnauthorized: true } : false;
    pgPool = new Pool({
      connectionString: process.env.DATABASE_URL,
      ssl,
      max: 10,
      idleTimeoutMillis: 30000,
      connectionTimeoutMillis: 3000
    });

    pgPool.on("error", (error) => {
      console.warn("PostgreSQL pool error, switching to local store:", error.message);
      usePg = false;
    });

    pgPool.query("SELECT 1")
      .then(() => {
        usePg = true;
        console.log("Connected to PostgreSQL database successfully.");
      })
      .catch((err) => {
        console.warn(`PostgreSQL not reachable (${err.message}). Using local persistent store.`);
        usePg = false;
      });
  } catch (err) {
    console.warn("Failed to initialize PostgreSQL pool, using local store:", err.message);
    usePg = false;
  }
} else {
  console.log("Using local persistent store (events.json).");
}

export const pool = {
  async query(text, params) {
    if (usePg && pgPool) {
      try {
        return await pgPool.query(text, params);
      } catch (err) {
        console.warn("PostgreSQL query failed, falling back to local store:", err.message);
        usePg = false;
        return localStore.query(text, params);
      }
    }
    return localStore.query(text, params);
  },
  on(event, handler) {
    if (pgPool) {
      pgPool.on(event, handler);
    }
  }
};
