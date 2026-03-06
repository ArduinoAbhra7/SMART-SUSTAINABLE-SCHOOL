/*
 ╔══════════════════════════════════════════════════════════════════════╗
 ║      SMART SUSTAINABLE SCHOOL – "The Last Signal-AS"               ║
 ║      ESP32 MASTER SKETCH v6 FINAL | STEMxplore 2026               ║
 ║      Theme: Engineering Energy-Efficient Communities               ║
 ╠══════════════════════════════════════════════════════════════════════╣
 ║  SYSTEMS:                                                           ║
 ║   1. 👣 Entry Counter   – IR × 2 (beam break, opposite walls)     ║
 ║   2. 💡 Zone Lighting   – LED × 4 → girls/boys split zones        ║
 ║   3. 🔊 Noise Monitor   – Sound sensor → Blynk notification       ║
 ║   4. ☀️  Solar Tracker   – LDR module × 2 (AO) → SG90 servo      ║
 ║   5. 🔔 Auto Bell (NTP) – PASSIVE buzzer, different tones per bell ║
 ║   6. 💧 Water Tank      – HC-SR04 + relay auto-cut pump           ║
 ╚══════════════════════════════════════════════════════════════════════╝

 ┌──────────────────────────────────────────────────────────────────────┐
 │  PIN MAP – ESP32 DevKit V1                                          │
 │                                                                      │
 │  👣 ENTRY COUNTER                                                    │
 │    GPIO 34 → IR OUTSIDE wall sensor DO  (INPUT ONLY)               │
 │    GPIO 35 → IR INSIDE  wall sensor DO  (INPUT ONLY)               │
 │    3.3V    → Both IR VCC  |  GND → Both IR GND                    │
 │    OUTSIDE beam first → INSIDE second  = ENTRY  count++           │
 │    INSIDE  beam first → OUTSIDE second = EXIT   count--           │
 │                                                                      │
 │  💡 ZONE LIGHTING                                                    │
 │    GPIO 16 → Girls Zone1 LED + 220Ω  (1–10 girls)                 │
 │    GPIO 17 → Girls Zone2 LED + 220Ω  (11–20 girls)                │
 │    GPIO 18 → Boys  Zone1 LED + 220Ω  (1–10 boys)                  │
 │    GPIO 19 → Boys  Zone2 LED + 220Ω  (11–20 boys)                 │
 │                                                                      │
 │  🔊 NOISE MONITOR                                                    │
 │    GPIO 36 → Sound Sensor DO  (INPUT ONLY)                         │
 │    3.3V    → Sound VCC  |  GND → Sound GND                        │
 │                                                                      │
 │  ☀️  SOLAR TRACKER                                                   │
 │    GPIO 39 → LDR Left  AO  (INPUT ONLY)                           │
 │    GPIO 23 → LDR Right AO                                         │
 │    GPIO  4 → SG90 Servo signal (PWM)                              │
 │    3.3V    → Both LDR VCC  |  5V → Servo VCC (red wire)          │
 │    GND     → Servo GND (brown wire)                               │
 │                                                                      │
 │  🔔 BELL + DISPLAY  ← PASSIVE BUZZER                                │
 │    GPIO  5 → Passive Buzzer + leg  (PWM tone via LEDC)            │
 │    GPIO 21 → TM1637 CLK                                            │
 │    GPIO 22 → TM1637 DIO                                            │
 │    5V      → TM1637 VCC  |  GND → TM1637 GND + Buzzer –          │
 │                                                                      │
 │  💧 WATER TANK                                                       │
 │    GPIO 25 → HC-SR04 TRIG  (direct)                               │
 │    GPIO 26 → HC-SR04 ECHO  via LLC (5V→3.3V)                     │
 │             LLC HV pin → 5V  |  LLC LV pin → 3.3V                │
 │             ECHO → LLC HV1  →  LLC LV1 → GPIO 26                 │
 │    GPIO 27 → Relay IN1  (LOW=pump ON, HIGH=pump OFF)              │
 │    5V      → HC-SR04 VCC + LLC HV + Relay VCC                    │
 │    GND     → HC-SR04 GND + LLC GND + Relay GND + Battery –       │
 │                                                                      │
 │  RELAY + PUMP WIRING:                                               │
 │    Relay IN1  → GPIO 27                                            │
 │    Relay COM  → Battery + (6V or 9V DC battery)                   │
 │    Relay NO   → Pump +  red wire                                  │
 │    Pump  –    → Battery –                                         │
 │    Battery –  → ESP32 GND  ← MUST connect (common ground!)       │
 │    Relay IN2  → not connected  (free channel)                     │
 │                                                                      │
 │  POWER                                                               │
 │    USB  → ESP32 micro-USB  (powerbank or phone charger)           │
 │    3.3V → IR×2, Sound, LDR×2, LLC LV                             │
 │    5V   → Servo, TM1637, HC-SR04, LLC HV, Relay                  │
 │    GND  → ALL GNDs + Battery – joined together                    │
 └──────────────────────────────────────────────────────────────────────┘

 PASSIVE BUZZER NOTE:
   Passive buzzer needs a PWM frequency to make sound.
   We use ESP32 LEDC hardware PWM (ledcWriteTone).
   LEDC channel 0 = buzzer  (channel 1 reserved for servo library)
   tone()  → ledcWriteTone(BUZZER_CH, frequency)
   noTone()→ ledcWriteTone(BUZZER_CH, 0)

 LIBRARIES – install via Arduino IDE Library Manager:
   • BlynkSimpleEsp32  → search "Blynk" by Volodymyr Shymanskyy
   • TM1637Display     → search "TM1637" by Avishay Orpaz
   • ESP32Servo        → search "ESP32Servo" by Kevin Harrington
   • WiFi.h, time.h   → built-in, no install needed
*/

// ══════════════════════════════════════════════════════
//  BLYNK CREDENTIALS  ← fill these in
// ══════════════════════════════════════════════════════
#define BLYNK_TEMPLATE_ID "TMPL3OKbmqe4K"
#define BLYNK_TEMPLATE_NAME "SMART SCHOOL HEADSIR NOTIFICATION "
#define BLYNK_AUTH_TOKEN    "DBw9kRnK_sARiclPrpSaGoOuQCLSSuUi"

// ══════════════════════════════════════════════════════
//  LIBRARIES
// ══════════════════════════════════════════════════════
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <BlynkSimpleEsp32.h>
#include <TM1637Display.h>
#include <ESP32Servo.h>

// ══════════════════════════════════════════════════════
//  USER SETTINGS
// ══════════════════════════════════════════════════════
const char* WIFI_SSID     = "A SEN";
const char* WIFI_PASSWORD = "Abhrasen1@";

const long  GMT_OFFSET_SEC  = 19800;
const int   DAYLIGHT_OFFSET = 0;
const char* NTP_SERVER      = "pool.ntp.org";

// ══════════════════════════════════════════════════════
//  PIN DEFINITIONS
// ══════════════════════════════════════════════════════
#define IR_OUTSIDE    34
#define IR_INSIDE     35

#define LED_GIRLS1    16
#define LED_GIRLS2    17
#define LED_BOYS1     18
#define LED_BOYS2     19

#define SOUND_DO      36

#define LDR_LEFT      33
#define LDR_RIGHT     32
#define SERVO_PIN      5

#define BUZZER_PIN     4
#define SEG_CLK       21
#define SEG_DIO       22

#define TRIG_PIN      25
#define ECHO_PIN      26
#define RELAY_PUMP    23

// ══════════════════════════════════════════════════════
// 🔔 PASSIVE BUZZER (LEDC – FIXED)
// ══════════════════════════════════════════════════════
#define BUZZER_CH   0
#define BUZZER_RES  8

void setupBuzzer() {
  
  ledcSetup(BUZZER_CH, 2000, BUZZER_RES);
  ledcAttachPin(BUZZER_PIN, BUZZER_CH);
  ledcWriteTone(BUZZER_CH, 0);
}

void buzzTone(int freq, int duration_ms) {
  if (freq > 0) {
    ledcWriteTone(BUZZER_CH, freq);
  } else {
    ledcWriteTone(BUZZER_CH, 0);
  }
  delay(duration_ms);
  ledcWriteTone(BUZZER_CH, 0);
}

void buzzOff() {
  ledcWriteTone(BUZZER_CH, 0);
}

// ══════════════════════════════════════════════════════
// CONFIG
// ══════════════════════════════════════════════════════
const int  SERVO_CENTER  = 90;
const int  SERVO_MIN     = 10;
const int  SERVO_MAX     = 170;
const int  SERVO_STEP    =  2;
const int  LDR_TOLERANCE = 200;

const unsigned long NOISE_COOLDOWN = 10000;
const unsigned long CROSS_TIMEOUT  = 2000;

const float TANK_HEIGHT_CM = 20.0;
const float SENSOR_OFFSET  = 1.0;
const float PUMP_ON_LEVEL  = 20.0;
const float PUMP_OFF_LEVEL = 95.0;

// ══════════════════════════════════════════════════════
// GLOBALS
// ══════════════════════════════════════════════════════
Servo solarServo;
TM1637Display segDisplay(SEG_CLK, SEG_DIO);
BlynkTimer blynkTimer;

int studentCount = 0;
enum CrossState { IDLE, OUTSIDE_FIRST, INSIDE_FIRST };
CrossState crossState = IDLE;
unsigned long crossStart = 0;

int servoAngle = SERVO_CENTER;
bool timeReady = false;

unsigned long lastNoiseAlert = 0;
float waterPercent = 0;
bool pumpRunning = false;

unsigned long lastSolar = 0;
unsigned long lastWater = 0;
unsigned long lastDisplay = 0;

// ══════════════════════════════════════════════════════
// SETUP
// ══════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);

  setupPins();
  setupBuzzer();     // 🔥 IMPORTANT FIX
  connectWiFiAndBlynk();
  syncNTP();

  solarServo.attach(SERVO_PIN);
  solarServo.write(servoAngle);

  segDisplay.setBrightness(7);
  segDisplay.showNumberDecEx(0, 0b01000000, true);

  blynkTimer.setInterval(3000L, pushToBlynk);
}

// ══════════════════════════════════════════════════════
// LOOP
// ══════════════════════════════════════════════════════
void loop() {
  Blynk.run();
  blynkTimer.run();

  unsigned long now = millis();

  checkEntryCounter();
  updateZoneLights();
  checkNoise();

  if (now - lastSolar >= 400) {
    lastSolar = now;
    trackSolar();
  }

  if (now - lastWater >= 500) {
    lastWater = now;
    checkWaterLevel();
  }

  if (now - lastDisplay >= 1000) {
    lastDisplay = now;
    updateDisplay();
  }
}

// ══════════════════════════════════════════════════════
// ENTRY COUNTER
// ══════════════════════════════════════════════════════
void checkEntryCounter() {
  bool outsideBroken = digitalRead(IR_OUTSIDE) == LOW;
  bool insideBroken  = digitalRead(IR_INSIDE)  == LOW;

  switch (crossState) {
    case IDLE:
      if (outsideBroken) {
        crossState = OUTSIDE_FIRST;
        crossStart = millis();
      } else if (insideBroken) {
        crossState = INSIDE_FIRST;
        crossStart = millis();
      }
      break;

    case OUTSIDE_FIRST:
      if (insideBroken) {
        studentCount++;
        crossState = IDLE;
      }
      break;

    case INSIDE_FIRST:
      if (outsideBroken) {
        studentCount--;
        if (studentCount < 0) studentCount = 0;
        crossState = IDLE;
      }
      break;
  }
}

// ══════════════════════════════════════════════════════
// NOISE
// ══════════════════════════════════════════════════════
void checkNoise() {
  if (digitalRead(SOUND_DO) == LOW) {
    unsigned long now = millis();
    if (now - lastNoiseAlert > NOISE_COOLDOWN) {
      lastNoiseAlert = now;
      Blynk.logEvent("noise_alert", "⚠ Classroom is too loud!");
      Blynk.virtualWrite(V1, "🔊 NOISY!");
    }
  }
}

// ══════════════════════════════════════════════════════
// SOLAR
// ══════════════════════════════════════════════════════
void trackSolar() {
  int leftVal  = analogRead(LDR_LEFT);
  int rightVal = analogRead(LDR_RIGHT);
  int diff = leftVal - rightVal;

  if (diff > LDR_TOLERANCE) servoAngle -= SERVO_STEP;
  else if (diff < -LDR_TOLERANCE) servoAngle += SERVO_STEP;

  servoAngle = constrain(servoAngle, SERVO_MIN, SERVO_MAX);
  solarServo.write(servoAngle);
}

// ══════════════════════════════════════════════════════
// WATER SYSTEM
// ══════════════════════════════════════════════════════
float measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;
  return (duration * 0.034) / 2;
}

void checkWaterLevel() {
  float dist = measureDistance();
  if (dist < 0) return;

  float depth = TANK_HEIGHT_CM - (dist - SENSOR_OFFSET);
  depth = constrain(depth, 0, TANK_HEIGHT_CM);
  waterPercent = (depth / TANK_HEIGHT_CM) * 100.0;

  if (waterPercent <= PUMP_ON_LEVEL && !pumpRunning) {
    digitalWrite(RELAY_PUMP, LOW);
    pumpRunning = true;
  }
  else if (waterPercent >= PUMP_OFF_LEVEL && pumpRunning) {
    digitalWrite(RELAY_PUMP, HIGH);
    pumpRunning = false;
  }
}

// ══════════════════════════════════════════════════════
// DISPLAY
// ══════════════════════════════════════════════════════
void updateDisplay() {
  if (!timeReady) return;
  struct tm t;
  if (!getLocalTime(&t)) return;
  int val = t.tm_hour * 100 + t.tm_min;
  bool colon = (t.tm_sec % 2 == 0);
  segDisplay.showNumberDecEx(val, colon ? 0b01000000 : 0, true);
}

// ══════════════════════════════════════════════════════
// BLYNK PUSH
// ══════════════════════════════════════════════════════
void pushToBlynk() {
  Blynk.virtualWrite(V0, studentCount);
  Blynk.virtualWrite(V2, waterPercent);
  Blynk.virtualWrite(V3, pumpRunning ? "Pump ON 🟢" : "Pump OFF ⚪");
}

// ══════════════════════════════════════════════════════
// SETUP HELPERS
// ══════════════════════════════════════════════════════
void setupPins() {
  pinMode(IR_OUTSIDE, INPUT);
  pinMode(IR_INSIDE, INPUT);

  pinMode(LED_GIRLS1, OUTPUT);
  pinMode(LED_GIRLS2, OUTPUT);
  pinMode(LED_BOYS1, OUTPUT);
  pinMode(LED_BOYS2, OUTPUT);

  pinMode(SOUND_DO, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(RELAY_PUMP, OUTPUT);
  digitalWrite(RELAY_PUMP, HIGH);
}

void connectWiFiAndBlynk() {
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);
}

void syncNTP() {
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET, NTP_SERVER);
  struct tm t;
  if (getLocalTime(&t)) timeReady = true;
}