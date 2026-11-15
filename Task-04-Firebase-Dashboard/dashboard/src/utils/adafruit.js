// Task 2 path: also publish the bulb command to the Adafruit IO "bulb" feed.
// The key is an account-wide master key and VITE_* values are readable in the browser —
// the server-side version (Cloud Function with a secret) is in ../../README.md.
const USER = import.meta.env.VITE_AIO_USERNAME;
const KEY = import.meta.env.VITE_AIO_KEY;

export async function publishBulb(state) {
  if (!USER || !KEY) return;                       // not configured: Firebase-only
  const res = await fetch(`https://io.adafruit.com/api/v2/${USER}/feeds/bulb/data`, {
    method: "POST",
    headers: { "X-AIO-Key": KEY, "Content-Type": "application/json" },
    body: JSON.stringify({ value: state ? "ON" : "OFF" }),
  });
  if (!res.ok) throw new Error(await res.text());
}
