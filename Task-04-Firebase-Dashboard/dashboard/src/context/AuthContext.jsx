import { createContext, useContext, useEffect, useState } from "react";
import {
  onAuthStateChanged, signInWithPopup, GoogleAuthProvider,
  signInWithEmailAndPassword, createUserWithEmailAndPassword, signOut,
} from "firebase/auth";
import { auth } from "../firebase";

const AuthContext = createContext(null);
export const useAuth = () => useContext(AuthContext);

export function AuthProvider({ children }) {
  const [currentUser, setCurrentUser] = useState(null);
  const [loading, setLoading] = useState(true);   // true until Firebase has restored (or not) a session

  useEffect(() => onAuthStateChanged(auth, (user) => { setCurrentUser(user); setLoading(false); }), []);

  const value = {
    currentUser,
    googleLogin: () => signInWithPopup(auth, new GoogleAuthProvider()),
    emailLogin: (email, pw) => signInWithEmailAndPassword(auth, email, pw),
    emailSignup: (email, pw) => createUserWithEmailAndPassword(auth, email, pw),
    logout: () => signOut(auth),
  };
  return <AuthContext.Provider value={value}>{!loading && children}</AuthContext.Provider>;
}
