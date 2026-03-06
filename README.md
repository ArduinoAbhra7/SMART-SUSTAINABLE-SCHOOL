# 🏫 Smart Sustainable School
### *"The Last Signal-AS" — ESP32 School Automation System*

> An energy-efficient school automation system built on a single ESP32, featuring smart lighting, water management, solar tracking, automatic bell, and biogas demonstration.

---

## 🎯 Project Overview

Schools waste enormous energy daily — lights in empty rooms, overflowing tanks, fixed solar panels. This project solves all of it with **one ESP32 and ~₹1500 in components**.

| Module | What It Does | Energy Impact |
|--------|-------------|---------------|
| 🏫 Smart Classroom | IR student counter + zonal LED lighting + noise alert | Up to 60% lighting savings |
| 💧 Smart Water Tank | Conductivity probes + auto motor control | 100% overflow prevention |
| ☀️ Solar Tracker | Dual LDR + SG90 servo follows the sun | ~35% more solar energy |
| 🔔 Auto Bell | WiFi NTP time-based bell + TM1637 display | No manual operator needed |
| 🌱 Biogas Model | Food waste → methane demonstration | Waste-to-energy concept |

---

## 🔌 Pin Reference

| Component | GPIO |
|-----------|------|
| IR Sensor 1 (Entry) | 34 |
| IR Sensor 2 (Exit) | 35 |
| Sound Sensor | 32 |
| Zone LED 1–4 | 25, 26, 27, 14 |
| Water Probe 1–4 | 33, 15, 2, 4 |
| Level LED 25/50/75/100% | 17, 16, 19, 21 |
| Motor LED | 23 |
| LDR Left / Right | 36, 39 |
| Servo (SG90) | 13 |
| Buzzer | 12 |
| TM1637 CLK / DIO | 5, 18 |

> ⚠️ **COMMON GROUND RULE:** All modules must share GND with ESP32.

---

## 📦 Libraries Required

Install via Arduino IDE → Tools → Manage Libraries:

- `ESP32Servo` by Kevin Harrington
- `TM1637Display` by Avishay Orpaz
- `BlynkSimpleEsp32` by Volodymyr Shymanskyy

---

## 🚀 Getting Started

```bash
# 1. Clone this repository
git clone https://github.com/YOUR_USERNAME/SmartSustainableSchool.git

# 2. Copy config template
cp config.example.h config.h

# 3. Edit config.h with your credentials
#    — WiFi SSID & Password
#    — Blynk Auth Token

# 4. Open SmartSustainableSchool.ino in Arduino IDE
# 5. Select board: ESP32 Dev Module
# 6. Upload!
```

---

## ⚙️ Configuration

Edit `config.h` (copy from `config.example.h`):

```cpp
#define WIFI_SSID       "YourWiFiName"
#define WIFI_PASS       "YourWiFiPassword"
#define BLYNK_TOKEN     "YourBlynkToken"
#define GMT_OFFSET_SEC  19800        // IST = UTC+5:30
#define NOISE_THRESHOLD 2500         // 0-4095, raise if too sensitive
```

> 🔒 `config.h` is in `.gitignore` — your credentials will never be committed.

---

## 🔔 Bell Schedule

Edit in `SmartSustainableSchool.ino`:

```cpp
BellTime bellSchedule[] = {
  {8,  0},   // Morning Assembly
  {8,  45},  // Period 1 End
  {9,  30},  // Period 2 End
  {10, 15},  // Short Break
  ...
  {15, 30},  // Dismissal
};
```

---

## 📱 Blynk Setup

1. Download **Blynk IoT** app (Android/iOS)
2. Create new project → Device: ESP32
3. Copy the **Auth Token** → paste in `config.h`
4. Add a **Notification** widget
5. Noise alerts will appear on the headmaster's phone

---

## 🗂 File Structure

```
SmartSustainableSchool/
├── SmartSustainableSchool.ino   ← Main code
├── config.h                     ← Your credentials (NOT committed)
├── config.example.h             ← Safe template to share
├── .gitignore                   ← Protects config.h
└── README.md                    ← This file
```

---

## 🏆 Competition

Built for **STEMxplore 2026** — *"Engineering Energy-Efficient Communities"*  
Category: Arduino-based project | Class 7–12

---

## 📄 License

MIT License — free to use, modify, and share with attribution.

---

*Built with ❤️ for a greener school 🌱*
