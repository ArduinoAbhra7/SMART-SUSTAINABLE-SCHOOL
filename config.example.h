/*
 * config.example.h — Smart Sustainable School
 * ✅  This file IS safe to share/commit to GitHub
 *
 * HOW TO USE:
 *   1. Copy this file and rename it to:  config.h
 *   2. Fill in your actual WiFi and Blynk details
 *   3. Never commit config.h to GitHub (it's in .gitignore)
 */

#ifndef CONFIG_H
#define CONFIG_H

// ── WiFi Credentials ──────────────────────────
#define WIFI_SSID        "YOUR_WIFI_NAME"
#define WIFI_PASS        "YOUR_WIFI_PASSWORD"

// ── Blynk Auth Token ──────────────────────────
// Get from Blynk app → Project Settings → Auth Token
#define BLYNK_TOKEN      "YOUR_BLYNK_AUTH_TOKEN"

// ── Time Settings ─────────────────────────────
// IST = UTC + 5:30 = 19800 seconds
#define GMT_OFFSET_SEC   19800

// ── Noise Sensitivity ─────────────────────────
// Range: 0–4095. Raise if too many false alerts.
#define NOISE_THRESHOLD  2500

#endif
