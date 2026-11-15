// Dependency-free SVG line chart: each series is scaled to its own min/max so
// temperature (~27), humidity (~60) and light (0-4095) can share one plot.
const SERIES = [
  { key: "temperature", label: "Temperature °C", color: "#1E8E5A" },
  { key: "humidity", label: "Humidity %", color: "#2A6CF0" },
  { key: "ldr", label: "Light (ADC)", color: "#C98512" },
];
const W = 760, H = 220, PAD = 12;

export default function HistoryChart({ records }) {
  if (records.length < 2) return <p className="muted">Waiting for more readings…</p>;
  const x = (i) => PAD + (i / (records.length - 1)) * (W - 2 * PAD);
  const line = (key) => {
    const vals = records.map((r) => Number(r[key]));
    const min = Math.min(...vals), max = Math.max(...vals), span = max - min || 1;
    return vals.map((v, i) => `${x(i)},${H - PAD - ((v - min) / span) * (H - 2 * PAD)}`).join(" ");
  };
  return (
    <figure>
      <svg viewBox={`0 0 ${W} ${H}`} role="img" aria-label="Sensor history chart" style={{ width: "100%" }}>
        {SERIES.map((s) => <polyline key={s.key} points={line(s.key)} fill="none" stroke={s.color} strokeWidth="2" />)}
      </svg>
      <figcaption className="legend">
        {SERIES.map((s) => <span key={s.key}><i style={{ background: s.color }} />{s.label}</span>)}
      </figcaption>
    </figure>
  );
}
