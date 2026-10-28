import pg from "pg";

const { Pool } = pg;
const ssl = process.env.PG_SSL === "true"
  ? { rejectUnauthorized: true }
  : false;

export const pool = new Pool({
  connectionString: process.env.DATABASE_URL,
  ssl,
  max: 10,
  idleTimeoutMillis: 30000,
  connectionTimeoutMillis: 5000
});

pool.on("error", (error) => {
  console.error("Unexpected PostgreSQL pool error:", error.message);
});
