import { useEffect, useState } from "react";
import { ref, onValue, set, get, query, limitToLast } from "firebase/database";
import { db } from "../firebase";
import { useAuth } from "../context/AuthContext";
import HistoryChart from "../components/HistoryChart";
import { downloadCsv } from "../utils/csv";
import { publishBulb } from "../utils/adafruit";

const HISTORY_SIZE = 60;

export default function Dashboard() {
  const { currentUser, logout } = useAuth();
  const [records, setRecords] = useState([]);                    // newest last
  const [bulbOn, setBulbOn] = useState(false);
  const [mode, setMode] = useState("manual");
  const [threshold, setThreshold] = useState(2500);
  const [note, setNote] = useState("");

  useEffect(() => {
    const offs = [
      onValue(query(ref(db, "sensorData"), limitToLast(HISTORY_SIZE)), (s) => setRecords(Object.values(s.val() ?? {}))),
      onValue(ref(db, "appliances/bulbState"), (s) => setBulbOn(!!s.val())),
      onValue(ref(db, "settings/mode"), (s) => setMode(s.val() ?? "manual")),
      onValue(ref(db, "settings/ldrThreshold"), (s) => setThreshold(s.val() ?? 2500)),
    ];
    return () => offs.forEach((off) => off());                   // detach every listener
  }, []);

  const latest = records[records.length - 1];
  const auto = mode === "automatic";

  const toggleBulb = async () => {
    const next = !bulbOn;
    await set(ref(db, "appliances/bulbState"), next);            // the ESP32 streams this path
    try { await publishBulb(next); } catch (e) { setNote(`Adafruit IO: ${e.message}`); }
  };

  const exportCsv = async () => {                                // the full history, not just the visible window
    const snap = await get(ref(db, "sensorData"));
    const all = Object.values(snap.val() ?? {}).sort((a, b) => a.timestamp - b.timestamp);
    downloadCsv(all, `sensor_log_${new Date().toISOString().slice(0, 10)}.csv`);
  };

  return (
    <main className="card">
      <header className="bar">
        <h1>IoT Monitor</h1>
        <span className="muted">{currentUser.email} · <button className="link" onClick={logout}>Sign out</button></span>
      </header>

      <section className="tiles">
        <Tile label="Temperature" value={latest ? `${Number(latest.temperature).toFixed(1)} °C` : "—"} />
        <Tile label="Humidity" value={latest ? `${Number(latest.humidity).toFixed(0)} %` : "—"} />
        <Tile label="Light (ADC)" value={latest ? latest.ldr : "—"} />
        <Tile label="Bulb" value={bulbOn ? "ON" : "OFF"} />
      </section>

      <section className="controls">
        <button onClick={toggleBulb} disabled={auto} title={auto ? "Disabled in automatic mode" : ""}>
          Turn bulb {bulbOn ? "OFF" : "ON"}
        </button>
        <button className="ghost" onClick={() => set(ref(db, "settings/mode"), auto ? "manual" : "automatic")}>
          Mode: {auto ? "Automatic" : "Manual"}
        </button>
        <label>LDR threshold
          <input type="number" min="0" max="4095" value={threshold}
                 onChange={(e) => setThreshold(e.target.value)}
                 onBlur={() => set(ref(db, "settings/ldrThreshold"), Number(threshold))} />
        </label>
        <button className="ghost" onClick={exportCsv}>Download CSV</button>
      </section>
      {note && <p className="error">{note}</p>}

      <h2>History</h2>
      <HistoryChart records={records} />
      <table>
        <thead><tr><th>Time</th><th>Temp</th><th>Hum</th><th>Light</th><th>Bulb</th><th>Mode</th></tr></thead>
        <tbody>
          {[...records].reverse().slice(0, 10).map((r) => (
            <tr key={r.timestamp}>
              <td>{new Date(r.timestamp).toLocaleString()}</td><td>{r.temperature}</td><td>{r.humidity}</td>
              <td>{r.ldr}</td><td>{r.bulbState ? "ON" : "OFF"}</td><td>{r.mode ?? "—"}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </main>
  );
}

const Tile = ({ label, value }) => (
  <div className="tile"><span className="muted">{label}</span><strong>{value}</strong></div>
);
