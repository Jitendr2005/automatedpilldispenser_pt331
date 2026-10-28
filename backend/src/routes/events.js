import { Router } from "express";
import { pool } from "../db.js";

const router = Router();

router.get("/", async (req, res, next) => {
  try {
    const parsed = Number.parseInt(req.query.limit ?? "50", 10);
    const limit = Number.isFinite(parsed) ? Math.min(Math.max(parsed, 1), 200) : 50;
    const result = await pool.query(
      `SELECT id, device_id, event_type, slot, success, message, details, created_at
       FROM device_events ORDER BY created_at DESC LIMIT $1`,
      [limit]
    );
    res.json({ events: result.rows });
  } catch (error) {
    next(error);
  }
});

router.post("/", async (req, res, next) => {
  try {
    const { device_id, event_type, slot, success, message, details } = req.body ?? {};

    if (typeof event_type !== "string" || event_type.trim().length < 1 ||
        event_type.length > 80) {
      return res.status(400).json({ error: "event_type is required (max 80 chars)" });
    }
    if (slot !== undefined && slot !== null &&
        (!Number.isInteger(slot) || slot < 0 || slot > 7)) {
      return res.status(400).json({ error: "slot must be an integer from 0 to 7" });
    }
    if (success !== undefined && success !== null && typeof success !== "boolean") {
      return res.status(400).json({ error: "success must be boolean or null" });
    }

    const result = await pool.query(
      `INSERT INTO device_events
       (device_id, event_type, slot, success, message, details)
       VALUES ($1, $2, $3, $4, $5, $6::jsonb)
       RETURNING id, device_id, event_type, slot, success, message, details, created_at`,
      [
        typeof device_id === "string" && device_id.length <= 80 ? device_id : "smart-apd-01",
        event_type.trim(),
        slot ?? null,
        success ?? null,
        typeof message === "string" ? message.slice(0, 500) : null,
        JSON.stringify(details && typeof details === "object" && !Array.isArray(details) ? details : {})
      ]
    );
    res.status(201).json({ event: result.rows[0] });
  } catch (error) {
    next(error);
  }
});

export default router;
