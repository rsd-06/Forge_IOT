// Turn the logged records into a CSV file and hand it to the browser as a download.
const HEADER = ["timestamp", "datetime", "temperature_c", "humidity_pct", "ldr_adc", "bulb_state", "mode"];

export function recordsToCsv(records) {
  const rows = records.map((r) => [
    r.timestamp,
    new Date(r.timestamp).toISOString(),
    r.temperature,
    r.humidity,
    r.ldr,
    r.bulbState ? "ON" : "OFF",
    r.mode ?? "",
  ]);
  return [HEADER, ...rows].map((row) => row.join(",")).join("\n");
}

export function downloadCsv(records, filename = "sensor_log.csv") {
  const blob = new Blob([recordsToCsv(records)], { type: "text/csv;charset=utf-8" });
  const url = URL.createObjectURL(blob);
  const a = document.createElement("a");
  a.href = url; a.download = filename; a.click();
  URL.revokeObjectURL(url);
}
