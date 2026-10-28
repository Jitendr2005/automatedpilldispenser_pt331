import "dotenv/config";
import express from "express";
import cors from "cors";
import { pool } from "./db.js";
import eventsRouter from "./routes/events.js";

const app = express();
const port = Number(process.env.PORT || 3000);

app.use(cors({ origin: process.env.CORS_ORIGIN || "http://localhost:5173" }));
app.use(express.json({ limit: "32kb" }));

app.get("/api/health", async (_req, res) => {
  try {
    await pool.query("SELECT 1");
    res.json({ status: "ok", database: "connected", service: "smart-apd-api" });
  } catch {
    res.status(503).json({ status: "degraded", database: "unavailable" });
  }
});

app.get("/api/summary", async (_req, res, next) => {
  try {
    const result = await pool.query(`
      SELECT COUNT(*)::int AS total_events,
             COUNT(*) FILTER (WHERE success = true)::int AS successful_events,
             COUNT(*) FILTER (WHERE success = false)::int AS failed_events,
             MAX(created_at) AS last_event_at
      FROM device_events
    `);
    res.json(result.rows[0]);
  } catch (error) {
    next(error);
  }
});

app.use("/api/events", eventsRouter);

app.use((err, _req, res, _next) => {
  console.error(err);
  res.status(500).json({ error: "Internal server error" });
});

app.listen(port, () => {
  console.log(`Smart APD API listening on http://localhost:${port}`);
});
