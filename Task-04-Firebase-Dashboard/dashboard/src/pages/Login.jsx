import { useState } from "react";
import { Navigate } from "react-router-dom";
import { useAuth } from "../context/AuthContext";

export default function Login() {
  const { currentUser, googleLogin, emailLogin, emailSignup } = useAuth();
  const [email, setEmail] = useState("");
  const [pw, setPw] = useState("");
  const [error, setError] = useState("");
  if (currentUser) return <Navigate to="/dashboard" replace />;

  const run = (fn) => async (e) => {
    e?.preventDefault();
    setError("");
    try { await fn(); } catch (err) { setError(err.code || err.message); }
  };

  return (
    <main className="card narrow">
      <h1>Forge IoT Monitor</h1>
      <p className="muted">Sign in to view and control the device.</p>
      <form onSubmit={run(() => emailLogin(email, pw))}>
        <input type="email" placeholder="Email" value={email} onChange={(e) => setEmail(e.target.value)} required />
        <input type="password" placeholder="Password" value={pw} onChange={(e) => setPw(e.target.value)} required />
        <button type="submit">Sign in</button>
        <button type="button" className="ghost" onClick={run(() => emailSignup(email, pw))}>Create account</button>
      </form>
      <button className="ghost" onClick={run(googleLogin)}>Continue with Google</button>
      {error && <p className="error" role="alert">{error}</p>}
    </main>
  );
}
