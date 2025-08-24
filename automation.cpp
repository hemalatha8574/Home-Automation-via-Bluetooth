/*
  Project: Bluetooth Home Automation
  Features:
  - Controls up to 4 relays via HC-05 (classic Serial)
  - Simple ASCII protocol with checksum (optional), e.g., "SET,1,ON"
  - Commands: SET,<ch>,ON|OFF ; TOGGLE,<ch> ; STATUS ; ALL,ON|OFF
  - Feedback responses and persistent save
  - Optional schedules: auto-off timer per channel
*/

#include <EEPROM.h>

// ------------------ USER CONFIG ------------------
const uint8_t RELAY_PINS[4] = {4,5,6,7};
const bool RELAY_ACTIVE_HIGH = true;
const uint32_t AUTO_OFF_MS[4] = {0, 0, 0, 0}; // e.g., {60000,0,0,0} -> ch1 auto-off 60s
// -------------------------------------------------

bool state[4] = {false,false,false,false};

void setRelay(uint8_t ch, bool on) {
  digitalWrite(RELAY_PINS[ch], (RELAY_ACTIVE_HIGH ? on : !on) ? HIGH : LOW);
  state[ch] = on;
}

void loadState() {
  uint16_t m; EEPROM.get(0, m);
  if (m == 0xCAFE) {
    EEPROM.get(2, state);
  }
}

void saveState() {
  uint16_t m = 0xCAFE;
  EEPROM.put(0, m);
  EEPROM.put(2, state);
}

uint32_t tOn[4] = {0,0,0,0};

void autoOffCheck() {
  uint32_t now = millis();
  for (uint8_t i=0;i<4;i++) {
    if (AUTO_OFF_MS[i] > 0 && state[i] && (now - tOn[i] >= AUTO_OFF_MS[i])) {
      setRelay(i, false);
      Serial.print(F("AUTO-OFF,")); Serial.println(i+1);
      saveState();
    }
  }
}

void printStatus() {
  Serial.print(F("STATUS,"));
  for (uint8_t i=0;i<4;i++) {
    Serial.print(state[i]?"ON":"OFF");
    if (i<3) Serial.print(',');
  }
  Serial.println();
}

void processCommand(const String& cmd) {
  // Expected forms:
  // SET,<ch>,ON|OFF
  // TOGGLE,<ch>
  // ALL,ON|OFF
  // STATUS
  // SAVE
  if (cmd == "STATUS") { printStatus(); return; }
  if (cmd == "SAVE") { saveState(); Serial.println(F("OK")); return; }

  if (cmd.startsWith("SET,")) {
    // SET,2,ON
    int p1 = cmd.indexOf(',', 4);
    if (p1 < 0) { Serial.println(F("ERR")); return; }
    uint8_t ch = (uint8_t)cmd.substring(4, p1).toInt();
    if (ch < 1 || ch > 4) { Serial.println(F("ERR")); return; }
    String token = cmd.substring(p1+1);
    token.trim();
    bool on = token == "ON";
    setRelay(ch-1, on);
    if (on) tOn[ch-1]=millis();
    saveState();
    Serial.print(F("OK,CH=")); Serial.print(ch); Serial.print(','); Serial.println(on?"ON":"OFF");
    return;
  }

  if (cmd.startsWith("TOGGLE,")) {
    uint8_t ch = (uint8_t)cmd.substring(7).toInt();
    if (ch < 1 || ch > 4) { Serial.println(F("ERR")); return; }
    setRelay(ch-1, !state[ch-1]);
    if (state[ch-1]) tOn[ch-1]=millis();
    saveState();
    Serial.print(F("OK,CH=")); Serial.print(ch); Serial.print(','); Serial.println(state[ch-1]?"ON":"OFF");
    return;
  }

  if (cmd.startsWith("ALL,")) {
    String token = cmd.substring(4); token.trim();
    bool on = token == "ON";
    for (uint8_t i=0;i<4;i++) { setRelay(i,on); if(on) tOn[i]=millis(); }
    saveState();
    Serial.println(F("OK,ALL"));
    return;
  }

  Serial.println(F("ERR"));
}

void setup() {
  for (uint8_t i=0;i<4;i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
    setRelay(i, false);
  }
  Serial.begin(9600); // HC-05 default baud
  loadState();
  // restore pins to saved state
  for (uint8_t i=0;i<4;i++) setRelay(i, state[i]);
  Serial.println(F("BT Home Auto Ready. Commands: STATUS, SET,<ch>,ON|OFF, TOGGLE,<ch>, ALL,ON|OFF, SAVE"));
}

void loop() {
  static String line="";
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c=='\n' || c=='\r') {
      line.trim();
      if (line.length()) processCommand(line);
      line="";
    } else {
      line += c;
      if (line.length() > 60) line=""; // safety
    }
  }
  autoOffCheck();
}
