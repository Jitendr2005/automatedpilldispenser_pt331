CREATE TABLE IF NOT EXISTS device_events (
  id BIGSERIAL PRIMARY KEY,
  device_id TEXT NOT NULL DEFAULT 'smart-apd-01',
  event_type TEXT NOT NULL,
  slot INTEGER CHECK (slot >= 0 AND slot <= 7),
  success BOOLEAN,
  message TEXT,
  details JSONB NOT NULL DEFAULT '{}'::jsonb,
  created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_device_events_created_at
  ON device_events (created_at DESC);
CREATE INDEX IF NOT EXISTS idx_device_events_device_id
  ON device_events (device_id);
