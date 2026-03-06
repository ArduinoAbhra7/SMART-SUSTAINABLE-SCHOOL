/*
 * ============================================================
 *   SMART SUSTAINABLE SCHOOL — "The Last Signal-AS"
 *   Powered by ESP32
 * ============================================================
 *  MODULES:
 *   1. Smart Classroom  — IR entry counter + zonal LEDs + noise alert
 *   2. Smart Water Tank — Conductivity probes + auto motor
 *   3. Sun Tracking     — Dual LDR + SG90 servo
 *   4. Automatic Bell   — WiFi NTP time + TM1637 display
 * ============================================================
 *  LIBRARIES REQUIRED (install via Arduino Library Manager):
 *   - ESP32Servo        by Kevin Harrington
 *   - TM1637Display     by Avishay Orpaz
 *   - BlynkSimpleEsp32  by Volodymyr Shymanskyy
 * ============================================================
 *  BOARD: ESP32 Dev Module
 *  Upload Speed: 115200
 * ============================================================
 */

#include <WiFi.h>
#include <time.h>
#include <ESP32Servo.h>
#include <TM1637Display.h>
#include <BlynkSimpleEsp32.h>
#include "config.h"   // ← WiFi, Blynk credentials stored here

// ============================================================
//  📌  PIN DEFINITIONS
// ============================================================

// --- Classroom ---
#define IR1_PIN     34
#define IR2_PIN     35
#define SOUND_PIN   32
#define ZONE1_LED   25
#define ZONE2_LED   26
#define ZONE3_LED   27
#define ZONE4_LED   14

// --- Water Tank ---
#define PROBE1      33
#define PROBE2      15
#define PROBE3      2
#define PROBE4      4
#define LED_25      17
#define LED_50      16    // GPIO 16 (free from TM1637)
#define LED_75      19
#define LED_100     21
#define MOTOR_LED   23

// --- Solar Tracker ---
#define LDR_LEFT    36
#define LDR_RIGHT   39
#define SERVO_PIN   13

// --- Bell & Display ---
#define BUZZER_PIN  12
#define CLK_PIN     5
#define DIO_PIN     18

// ============================================================
//  🕐  BELL SCHEDULE  (24-hour IST)
// ============================================================
struct BellTime { int hour; int minute; };

BellTime bellSchedule[] = {
  {8,  0},   // Morning Assembly
  {8,  45},  // Period 1 End
  {9,  30},  // Period 2 End
  {10, 15},  // Short Break Start
  {10, 30},  // Short Break End
  {11, 15},  // Period 4 End
  {12, 0},   // Period 5 End
  {13, 0},   // Lunch Start
  {13, 30},  // Lunch End
  {14, 15},  // Period 7 End
  {15, 0},   // Period 8 End
  {15, 30},  // School Dismissal
};

const int BELL_COUNT = sizeof(bellSchedule) / sizeof(bellSchedule[0]);

// ============================================================
//  🌐  OBJECTS & STATE
// ============================================================
Servo         panelServo;
TM1637Display display(CLK_PIN, DIO_PIN);

int  studentCount = 0;
int  servoAngle   = 90;
bool bellRung[12] = {false};

// ============================================================
//  🔧  SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Smart Sustainable School Booting... ===");

  // Classroom
  pinMode(IR1_PIN,   INPUT);
  pinMode(IR2_PIN,   INPUT);
  pinMode(SOUND_PIN, INPUT);
  pinMode(ZONE1_LED, OUTPUT);
  pinMode(ZONE2_LED, OUTPUT);
  pinMode(ZONE3_LED, OUTPUT);
  pinMode(ZONE4_LED, OUTPUT);

  // Water Tank
  pinMode(PROBE1,    INPUT);
  pinMode(PROBE2,    INPUT);
  pinMode(PROBE3,    INPUT);
  pinMode(PROBE4,    INPUT);
  pinMode(LED_25,    OUTPUT);
  pinMode(LED_50,    OUTPUT);
  pinMode(LED_75,    OUTPUT);
  pinMode(LED_100,   OUTPUT);
  pinMode(MOTOR_LED, OUTPUT);

  // Solar Tracker
  panelServo.attach(SERVO_PIN);
  panelServo.write(servoAngle);

  // Bell
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Display
  display.setBrightness(5);
  display.showNumberDecEx(0, 0b01000000, true);

  // WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 20) {
    delay(500); Serial.print("."); tries++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected! IP: " + WiFi.localIP().toString());
    configTime(GMT_OFFSET_SEC, 0, "pool.ntp.org");
    Serial.println("NTP time synced (IST).");
  } else {
    Serial.println("\nWiFi FAILED — running in offline mode.");
  }

  // Blynk
  Blynk.config(BLYNK_TOKEN);
  Blynk.connect(3000);

  Serial.println("=== Boot Complete! All systems GO. ===\n");
}

// ============================================================
//  🔄  MAIN LOOP — millis() only, no delay()
// ============================================================
void loop() {
  Blynk.run();
  checkEntryCount();
  updateZoneLights();
  checkNoise();
  checkWaterLevel();
  trackSolar();
  checkBell();
  updateDisplay();
}

// ============================================================
//  MODULE 1: SMART CLASSROOM
// ============================================================
void checkEntryCount() {
  static bool ir1Last = HIGH, ir2Last = HIGH;
  bool ir1 = digitalRead(IR1_PIN);
  bool ir2 = digitalRead(IR2_PIN);

  if (ir1 == LOW && ir1Last == HIGH) {
    studentCount++;
    Serial.println("ENTER → Count: " + String(studentCount));
  }
  if (ir2 == LOW && ir2Last == HIGH && studentCount > 0) {
    studentCount--;
    Serial.println("EXIT  → Count: " + String(studentCount));
  }
  ir1Last = ir1;
  ir2Last = ir2;
}

void updateZoneLights() {
  digitalWrite(ZONE1_LED, studentCount >= 1  ? HIGH : LOW);
  digitalWrite(ZONE2_LED, studentCount >= 11 ? HIGH : LOW);
  digitalWrite(ZONE3_LED, studentCount >= 21 ? HIGH : LOW);
  digitalWrite(ZONE4_LED, studentCount >= 31 ? HIGH : LOW);
}

void checkNoise() {
  static unsigned long lastAlert = 0;
  int level = analogRead(SOUND_PIN);
  if (level > NOISE_THRESHOLD && millis() - lastAlert > 30000) {
    Serial.println("NOISE ALERT! Level: " + String(level));
    Blynk.notify("⚠️ High noise detected in classroom!");
    lastAlert = millis();
  }
}

// ============================================================
//  MODULE 2: SMART WATER TANK
// ============================================================
void checkWaterLevel() {
  bool p1 = digitalRead(PROBE1);   // 25%
  bool p2 = digitalRead(PROBE2);   // 50%
  bool p3 = digitalRead(PROBE3);   // 75%
  bool p4 = digitalRead(PROBE4);   // 100%

  digitalWrite(LED_25,  p1 ? HIGH : LOW);
  digitalWrite(LED_50,  p2 ? HIGH : LOW);
  digitalWrite(LED_75,  p3 ? HIGH : LOW);
  digitalWrite(LED_100, p4 ? HIGH : LOW);

  // Motor: ON if below 25%, OFF if 100% full
  if (!p1) {
    digitalWrite(MOTOR_LED, HIGH);
    Serial.println("WATER LOW — Motor ON");
  }
  if (p4) {
    digitalWrite(MOTOR_LED, LOW);
    Serial.println("WATER FULL — Motor OFF");
  }
}

// ============================================================
//  MODULE 3: SUN TRACKING SOLAR PANEL
// ============================================================
void trackSolar() {
  static unsigned long lastTrack = 0;
  if (millis() - lastTrack < 500) return;
  lastTrack = millis();

  int leftVal  = analogRead(LDR_LEFT);
  int rightVal = analogRead(LDR_RIGHT);
  int diff     = leftVal - rightVal;

  if (diff > 100  && servoAngle > 0)   servoAngle -= 2;  // rotate left
  if (diff < -100 && servoAngle < 180) servoAngle += 2;  // rotate right

  panelServo.write(servoAngle);
  Serial.println("Solar → L:" + String(leftVal) + " R:" + String(rightVal) + " Angle:" + String(servoAngle));
}

// ============================================================
//  MODULE 4: AUTOMATIC BELL (NTP WiFi Time)
// ============================================================
void checkBell() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  for (int i = 0; i < BELL_COUNT; i++) {
    bool matchTime = (timeinfo.tm_hour == bellSchedule[i].hour &&
                      timeinfo.tm_min  == bellSchedule[i].minute);
    if (matchTime && !bellRung[i]) {
      Serial.println("🔔 BELL at " + String(bellSchedule[i].hour) + ":" + String(bellSchedule[i].minute));
      digitalWrite(BUZZER_PIN, HIGH);
      delay(2000);               // 2-second ring (only place delay is acceptable)
      digitalWrite(BUZZER_PIN, LOW);
      bellRung[i] = true;
    }
    if (!matchTime) bellRung[i] = false;
  }
}

void updateDisplay() {
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate < 10000) return;
  lastUpdate = millis();

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    display.showNumberDecEx(0, 0b01000000, true);
    return;
  }
  int timeVal = timeinfo.tm_hour * 100 + timeinfo.tm_min;
  display.showNumberDecEx(timeVal, 0b01000000, true);
}
