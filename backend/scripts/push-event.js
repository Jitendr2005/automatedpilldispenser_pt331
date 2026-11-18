#!/usr/bin/env node

/**
 * Script to push demo dispense events into the Smart APD backend.
 * Usage:
 *   node backend/scripts/push-event.js
 *   node backend/scripts/push-event.js --slot 2 --success true --msg "Dose dispensed"
 *   node backend/scripts/push-event.js --fail --slot 4
 *   node backend/scripts/push-event.js --simulate
 */

const API_URL = process.env.API_URL || "http://localhost:3000/api/events";

const args = process.argv.slice(2);
const isSimulate = args.includes("--simulate");
const isFail = args.includes("--fail");

function getArg(name, defaultValue) {
  const index = args.indexOf(name);
  if (index !== -1 && index + 1 < args.length) {
    return args[index + 1];
  }
  return defaultValue;
}

const slotArg = getArg("--slot", null);
const typeArg = getArg("--type", isFail ? "chute_sensor_alert" : "scheduled_dispense");
const successArg = isFail ? false : getArg("--success", "true") === "true";
const msgArg = getArg(
  "--msg",
  isFail
    ? "Passage sensor timeout (3000ms). Chute blocked or pill failed to drop."
    : "Dose dispensed into chute. IR beam passage verified."
);

async function sendEvent(eventData) {
  try {
    const res = await fetch(API_URL, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(eventData)
    });

    if (!res.ok) {
      const text = await res.text();
      throw new Error(`HTTP ${res.status}: ${text}`);
    }

    const data = await res.json();
    console.log(
      `✅ [${new Date().toLocaleTimeString()}] Event posted:`,
      `[Slot ${data.event.slot ?? "—"}]`,
      data.event.event_type,
      data.event.success ? "(SUCCESS)" : "(FAILED)",
      "-",
      data.event.message
    );
    return data;
  } catch (err) {
    console.error("❌ Failed to send event to backend:", err.message);
    console.log("   (Make sure the backend is running on http://localhost:3000)");
  }
}

async function run() {
  if (isSimulate) {
    console.log("🚀 Starting live APD simulation (pushes a new event every 10 seconds)...");
    console.log("   Press Ctrl+C to stop.\n");

    const slots = [0, 1, 2, 3, 4, 5, 6, 7];
    const medications = [
      "Metformin 500mg",
      "Aspirin 81mg",
      "Atorvastatin 20mg",
      "Lisinopril 10mg",
      "Amlodipine 5mg",
      "Vitamin D3",
      "Omeprazole 20mg",
      "Melatonin 3mg"
    ];

    let count = 0;
    const interval = setInterval(async () => {
      count++;
      const slotIndex = count % 8;
      // Inject an intentional fault occasionally (every 5th event) for realistic demonstration
      const willFail = count % 5 === 0;

      const event = {
        device_id: "smart-apd-01",
        event_type: willFail ? "chute_sensor_alert" : "scheduled_dispense",
        slot: slotIndex,
        success: !willFail,
        message: willFail
          ? `Passage timeout (slot ${slotIndex}). Pill did not clear chute sensor.`
          : `Dispensed slot ${slotIndex} (${medications[slotIndex]}). IR beam passage confirmed.`,
        details: {
          medication: medications[slotIndex],
          passage_time_ms: willFail ? 0 : Math.floor(100 + Math.random() * 60)
        }
      };

      await sendEvent(event);
    }, 10000);

    process.on("SIGINT", () => {
      clearInterval(interval);
      console.log("\nSimulation stopped.");
      process.exit(0);
    });
  } else {
    const slot = slotArg !== null ? Number.parseInt(slotArg, 10) : Math.floor(Math.random() * 8);
    const event = {
      device_id: "smart-apd-01",
      event_type: typeArg,
      slot: slot,
      success: successArg,
      message: msgArg,
      details: {
        source: "manual_trigger",
        timestamp: Date.now()
      }
    };

    console.log("Pushing single demonstration event to backend...");
    await sendEvent(event);
  }
}

run();

