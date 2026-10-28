import { useEffect, useState } from "react";
import { Activity, AlertTriangle, CheckCircle2, Clock3, Pill, RefreshCw } from "lucide-react";

function Metric({ icon: Icon, label, value, note }) {
  return <article className="metric">
    <div className="metric-top"><span>{label}</span><Icon size={19} /></div>
    <strong>{value ?? "—"}</strong>
    <small>{note}</small>
  </article>;
}

function formatDate(value) {
  if (!value) return "—";
  const date = new Date(value);
  return Number.isNaN(date.getTime()) ? "—" : date.toLocaleString();
}

export default function App() {
  const [summary, setSummary] = useState(null);
  const [events, setEvents] = useState([]);
  const [status, setStatus] = useState("Loading");
  const [busy, setBusy] = useState(false);

  async function refresh() {
    setBusy(true);
    try {
      const [healthResponse, summaryResponse, eventsResponse] = await Promise.all([
        fetch("/api/health"),
        fetch("/api/summary"),
        fetch("/api/events?limit=50")
      ]);
      if (!healthResponse.ok || !summaryResponse.ok || !eventsResponse.ok) {
        throw new Error("API request failed");
      }
      const health = await healthResponse.json();
      const summaryData = await summaryResponse.json();
      const eventData = await eventsResponse.json();
      setStatus(health.database === "connected" ? "API connected" : "Database unavailable");
      setSummary(summaryData);
      setEvents(eventData.events || []);
    } catch {
      setStatus("Backend unavailable");
      setSummary(null);
      setEvents([]);
    } finally {
      setBusy(false);
    }
  }

  useEffect(() => {
    refresh();
    const id = setInterval(refresh, 15000);
    return () => clearInterval(id);
  }, []);

  return <main className="shell">
    <header className="header">
      <div className="brand"><div className="brand-icon"><Pill size={23} /></div>
        <div><p className="eyebrow">PT 331 · MAJOR PROJECT</p><h1>Smart APD</h1>
          <p className="subtitle">Automated Pill Dispenser · Monitoring Console</p></div>
      </div>
      <button className="refresh" onClick={refresh} disabled={busy}>
        <RefreshCw size={16} className={busy ? "spin" : ""} /> Refresh
      </button>
    </header>

    <section className="statusbar">
      <span className={`status-dot ${status === "API connected" ? "good" : "bad"}`} />
      <span>{status}</span><span className="status-separator">•</span>
      <span>Auto-refresh every 15 seconds</span>
    </section>

    <section className="intro">
      <div><p className="eyebrow">SYSTEM OVERVIEW</p><h2>Device activity</h2>
        <p>Review recorded events and identify issues requiring manual inspection.</p></div>
      <div className="device-chip"><Activity size={16} /> smart-apd-01</div>
    </section>

    <section className="metrics">
      <Metric icon={Activity} label="Total events" value={summary?.total_events ?? 0} note="Recorded in database" />
      <Metric icon={CheckCircle2} label="Marked successful" value={summary?.successful_events ?? 0} note="Passage sensor reported" />
      <Metric icon={AlertTriangle} label="Marked failed" value={summary?.failed_events ?? 0} note="Needs manual review" />
      <Metric icon={Clock3} label="Last event" value={summary?.last_event_at ? formatDate(summary.last_event_at) : "—"} note="Most recent record" />
    </section>

    <section className="panel">
      <div className="panel-heading"><div><h2>Event history</h2>
        <p>Latest 50 records from the backend</p></div><span className="count">{events.length} records</span></div>
      {events.length === 0 ? <div className="empty">
        <div className="empty-icon"><Activity size={25} /></div>
        <h3>No events to display</h3>
        <p>Start the API and database, then post a test event to populate this table.</p>
      </div> : <div className="table-wrap"><table>
        <thead><tr><th>Time</th><th>Device</th><th>Event</th><th>Slot</th><th>Result</th><th>Message</th></tr></thead>
        <tbody>{events.map((event) => <tr key={event.id}>
          <td>{formatDate(event.created_at)}</td>
          <td>{event.device_id}</td>
          <td>{event.event_type}</td>
          <td>{event.slot ?? "—"}</td>
          <td><span className={`pill ${event.success === true ? "success" : event.success === false ? "failure" : "neutral"}`}>
            {event.success === true ? "Success" : event.success === false ? "Failed" : "Review"}
          </span></td>
          <td>{event.message || "—"}</td>
        </tr>)}</tbody>
      </table></div>}
    </section>
    <footer>Student prototype · Sensor detection is not proof of pill identity or correct dose quantity.</footer>
  </main>;
}
