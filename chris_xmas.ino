#include "Arduino.h"

// Halloween colors:
// orange = 15,02,00     = F20
// lime green 04,15,00   = 4F0
// bluegreen 00, 10, 15  = 00F

//===================================================================================
// Wifi
//===================================================================================
#include <SPI.h>
#include <WiFi.h>
#include <avr/pgmspace.h>
#include "htmlwriter.h"

int Parse1Hex(char c) {
  if (c >= 'a' && c <= 'f') {
    return 10 + (c - 'a');
  } else if (c >= 'A' && c <= 'F') {
    return 10 + (c - 'A');
 } else if (c >= '0' && c <= '9') {
    return c - '0';
  } else {
    return -1;
  }
}

#define P(a) do { Serial.println(a); } while (0);
#define P2(a, b) do { Serial.print(a); Serial.println(b); } while(0);
#define P3(a, b, c) do { \
  Serial.print(a); Serial.print(b); Serial.println(c); } while(0);
#define P4(a, b, c, d) do { \
  Serial.print(a); Serial.print(b); Serial.print(c); Serial.println(d); } while(0);
#define P8(a, b, c, d, e, f, g, h) do { \
  Serial.print(a); Serial.print(b); Serial.print(c); Serial.print(d); \
  Serial.print(e); Serial.print(f); Serial.print(g); Serial.println(h); } while(0);

namespace Global {
  typedef enum SelectedProgram_e {
    PROGRAM_HALLOWEEN = 1,
    PROGRAM_CHRISTMAS,
    PROGRAM_RAINBOW,
    PROGRAM_TESTING
  } SelectedProgram;
  SelectedProgram selected_program = PROGRAM_HALLOWEEN;
  
  typedef enum CommandType_e {
    CMD_SOLID_COLORS = 1,  // S
    CMD_SHIFT = 2,  // F
    CMD_RACE = 3,  // R or E
    CMD_FADE = 4,  // A
    CMD_WAIT = 5,  // W
  } CommandType;
  
  static const byte kMaxSolidColors = 16;
  typedef struct SolidColorsData_t {
    byte n_colors;
    byte colors[kMaxSolidColors];
  } SolidColorsData;
  
  typedef struct WaitData_t {
    int n_msec;
  } WaitData;
  
  static const byte kMaxShiftColors = 16;
  typedef struct ShiftData_t {
    int n_colors;
    byte colors[kMaxShiftColors];
    int max_shifts;  // default is 50
  } ShiftData;
    
  static const byte kMaxRaceColors = 16;
  typedef struct RaceData_t {
    int n_colors;
    byte colors[kMaxRaceColors];
    bool forwards;
    int delay_msec;  // default 100
  } RaceData;
  
  typedef struct FadeData_t {
    int dest_brightness;
    int delay_msec;  // default 5
  } FadeData;
  
  typedef union CommandData_u {
    SolidColorsData solid_colors;
    WaitData wait;
    ShiftData shift;
    RaceData race;
    FadeData fade;
  } CommandData;
  typedef struct Command_s {
    CommandType type;
    CommandData data;
  } Command;
  
  static const int kMaxCommands = 64;
  int n_commands;
  Command commands[kMaxCommands];
  
  static const int kMaxColors = 16;
  int n_colors;
  int R[kMaxColors];
  int G[kMaxColors];
  int B[kMaxColors];
  
  int brightness = 0xcc;  // Global brightness
  
  // current program
  prog_char program_halloween[] PROGMEM = "C000;CF20;C4F0;C00F;S123;A204;W10000;S0;F123000;S123;W1000;S0;R111;E111;R222;E222;R333;E333;S123;W1000;A0;S1;A204;W2000;A0;S2;A204;W2000;A0;S3;A204;W2000;A0;S0;A204;";
  prog_char program_christmas[] PROGMEM = "C000;CF00;C0F0;A0;S1;A204;W1000;A0;S2;A204;W1000;";
  static const int kMaxProgramBuf = 1024;
  char program_str[kMaxProgramBuf] = "";
  
  void FetchCurrentProgram() {
    P("Fetching program");
    switch (selected_program) {
      case PROGRAM_HALLOWEEN: {
        P("  halloween");
        strcpy_P(program_str, program_halloween);
        break;
      }
      case PROGRAM_CHRISTMAS: {
        P("  christmas");
        strcpy_P(program_str, program_christmas);
        break;
      }
    }
    P2("  Program: ", program_str);
  }
  
  void ParseProgram() {
    FetchCurrentProgram();
    char* buf = program_str;
    n_colors = 0;
    n_commands = 0;
    int i = 0;
    char c;
    while ((c = buf[i++]) != '\0') {
      switch (c) {
        case 'C': {  // Color definition.

          R[n_colors] = Parse1Hex(buf[i++]);
          G[n_colors] = Parse1Hex(buf[i++]);
          B[n_colors] = Parse1Hex(buf[i++]);
          P8("Color: ", n_colors, " R:", R[n_colors], " G:", G[n_colors], " B:", B[n_colors]);
          n_colors++;
          if ((c = buf[i++]) != ';') {
            P2("Unexpected character: ", c);
            return;
          }
          break;
        }
        case 'S': {  // Solid colors.
          P(F("CMD: solid color"));
          commands[n_commands].type = CMD_SOLID_COLORS;
          commands[n_commands].data.solid_colors.n_colors = 0;
          int n_colors = 0;
          while ((c = buf[i++]) != ';') {
            P4("  Color ", n_colors, ": ", Parse1Hex(c));
            commands[n_commands].data.solid_colors.colors[n_colors++] = Parse1Hex(c);
          }
          commands[n_commands].data.solid_colors.n_colors = n_colors;
          n_commands++;
          break;
        }
        case 'W': {  // Wait
          P(F("CMD: wait"));
          int msec = 0;
          while ((c = buf[i++]) != ';') {
            msec = msec*10 + (c - '0');
          }
          commands[n_commands].type = CMD_WAIT;
          commands[n_commands].data.wait.n_msec = msec;
          P3("CMD: wait for ", commands[n_commands].data.wait.n_msec, " msec");
          n_commands++;
        break;
        }
        case 'F': {  // shiFters
          P(F("CMD: shiFters"));
          commands[n_commands].type = CMD_SHIFT;
          int n_colors = 0;
          while ((c = buf[i++]) != ';' && c != ',') {
            P4("  Color ", n_colors, ": ", Parse1Hex(c));
            commands[n_commands].data.shift.colors[n_colors++] = Parse1Hex(c);
          }
          commands[n_commands].data.shift.n_colors = n_colors;
          int max_shifts = 50;
          if (c == ',') {
            max_shifts = 0;
            while ((c = buf[i++]) != ';') {
              max_shifts = 10*max_shifts + (c - '0');
            }
          }
          P2("  Max Shifts: ", max_shifts); 
          commands[n_commands].data.shift.max_shifts = max_shifts;   
          n_commands++;
          break;
        }
        case 'R': // Race forwards
        commands[n_commands].data.race.forwards = true;
        // Fallthrough intended.
        case 'E': {  // racE backwards
          P2("CMD: Race. Fowards? ", commands[n_commands].data.race.forwards);
          commands[n_commands].type = CMD_RACE;
          int n_colors = 0;
          while ((c = buf[i++]) != ';' && c != ',') {
            P4("  Color ", n_colors, ": ", Parse1Hex(c));
            commands[n_commands].data.race.colors[n_colors++] = Parse1Hex(c);
          }
          commands[n_commands].data.race.n_colors = n_colors;
          int delay_msec = 100;
          if (c == ',') {
            delay_msec = 0;
            while ((c = buf[i++]) != ';') {
              delay_msec = 10*delay_msec + (c - '0');
            }
          }
          commands[n_commands].data.race.delay_msec = delay_msec;
          n_commands++;
          break;
        }
        case 'A': {  // fAde
          P("CMD: Fade");
          commands[n_commands].type = CMD_FADE;
          int dest_brightness = 0;
          while ((c = buf[i++]) != ';' && c != ',') {
            dest_brightness = 10*dest_brightness + (c - '0');
          }
          if (dest_brightness > 0xcc) {
            dest_brightness = 0xcc;  // =204
          }
          commands[n_commands].data.fade.dest_brightness = dest_brightness;
          int delay_msec = 5;
          if (c == ',') {
            delay_msec = 0;
            while ((c = buf[i++]) != ';') {
              delay_msec = 10*delay_msec + (c - '0');
            }
          }
          commands[n_commands].data.fade.delay_msec = delay_msec;
          n_commands++;
          break;
        }
      }
    }
  }
}  // namespace Global

#include "credentials.h"  // NOTE: you need to create this file yourself and define WIFI_SSID and WIFI_PASSWORD

namespace Wifi {
  char ssid[] = WIFI_SSID;      //  your network SSID (name) 
  char pass[] = WIFI_PASSWORD;   // your network password
  int status = WL_IDLE_STATUS;
  int last_status = WL_IDLE_STATUS;
  WiFiServer server(80);
  
  bool HandleConnection(WiFiClient* client) {
    bool return_status = false;
    bool found_upload = true;
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client->connected()) {            // loop while the client's connected
      if (client->available()) {             // if there's bytes to read from the client,
        char c = client->read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) { 
            htmlwriter::Output(client, Global::program_str);
            /* 
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:    
            client->println("HTTP/1.1 200 OK");
            client->println("Content-type:text/html");
            client->println();

            // the content of the HTTP response follows the header:
            client->print("Click <a href=\"/H\">here</a> turn the LED on pin 9 on<br>");
            client->print("Click <a href=\"/L\">here</a> turn the LED on pin 9 off<br>");

            // The HTTP response ends with another blank line:
            client->println();
            */
            // break out of the while loop:
            break;         
          } 
          else {      // if you got a newline, then clear currentLine:
            // Was this an Upload request?
            if (found_upload) {
              Serial.println("Found an upload request!!");
            }
            found_upload = false;
            currentLine = "";
          }
        }     
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /restart")) {
          return_status = true;
        }
        if (currentLine.endsWith("GET /halloween")) {
          Global::selected_program = Global::PROGRAM_HALLOWEEN;
          return_status = true;
        }
        if (currentLine.endsWith("GET /christmas")) {
          Global::selected_program = Global::PROGRAM_CHRISTMAS;
          return_status = true;
        }
        if (currentLine.endsWith("Get /upload")) {
          found_upload = true;
        }
      }
    }
    // close the connection:
    client->stop();
    Serial.println("client disonnected");
    return return_status;
  }
  bool WifiCallback() {
    if (status != WL_CONNECTED) {
      Serial.print(F("Attempting to connect to Network named: "));
      Serial.println(ssid);                   // print the network name (SSID);

      // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
      status = WiFi.begin(ssid, pass);
      return false;
    }
    if (status != last_status) {
      last_status = status;
      Serial.println("Connected to Wifi");
      Serial.print("SSID: ");
      Serial.println(WiFi.SSID());

      // print your WiFi shield's IP address:
      IPAddress ip = WiFi.localIP();
      Serial.print("IP Address: ");
      Serial.println(ip);
      
      Serial.println("Starting server");
      server.begin();
    }
    WiFiClient client = server.available();
    if (client) {
      return Wifi::HandleConnection(&client);
    } else {
      return false;
    }
  }
}  // namespace Wifi  


// Collabarative multitasking.
#define NUM_PERIODIC_EVENTS (10)
#define MAX_UNSIGNED_LONG
namespace PeriodicEvent {
  unsigned long msec_[NUM_PERIODIC_EVENTS];
  bool (*callback_[NUM_PERIODIC_EVENTS])();
  unsigned long next_[NUM_PERIODIC_EVENTS];
  unsigned long min_interval_ = 10000000;
  unsigned long last_check_;
  
  // If the callback returns true, this indicates some global state has changed and the main control loop should restart.
  void SetHandler(int pos, unsigned long msec, bool (*callback)()) {
    msec_[pos] = msec;
    callback_[pos] = callback;
    next_[pos] = millis() + msec;
    if (msec < min_interval_) {
      min_interval_ = msec;
      Serial.print("Min interval: ");
      Serial.println(min_interval_);
    }
  }
  
  // Time to run?
  bool Check() {
    bool state_change = false;
    unsigned long last_check_ = millis();
    for (int i = 0; i < NUM_PERIODIC_EVENTS; ++i) {
      if (next_[i] > 0 && next_[i] <= last_check_ && callback_[0]) {
        Serial.print("Calling interrupt: ");
        Serial.println(i);
        if ((*callback_[i])()) {
          state_change = true;
        }
        next_[i] = millis() + msec_[i];
      }
    }
    return state_change;
  }
  
  // Similar to delay(msec), but handles collaboration while waiting. May return early if there was a global state change.
  bool Delay(unsigned long msec) {
    unsigned long sleep_remaining = msec;
    while (sleep_remaining > 0) {
      int sleep_time = (sleep_remaining > min_interval_) ? min_interval_ : sleep_remaining;
      sleep_remaining -= sleep_time;
      Serial.print("delaying: ");
      Serial.println(sleep_time);
      delay(sleep_time);
      if (millis() > last_check_ + min_interval_) { 
        if (Check()) { 
          return true;
        }
      }
    }
    return false;
  } 
}  // namespace


int pin = 31;
int led_pin = 13;
#define N_BULBS (25)
#define N_BULBS_TO_INIT (50)
#define BROADCAST (63)

#define DELAY_US(x) delayMicroseconds(x);

int delay_10us_high = 4;//5  // fact:8.5us
int delay_10us_low = 7;//6   // fact:11us
int delay_20us_low = 19;//15 // fact:23us
int delay_20us_high = 17;//15// fact:21.5us
int delay_start_of_frame = 7;// fact:11.5us
int delay_end_of_frame = 170;  // fact:188us
int delay_interframe = 7600;  // fact:7.67ms

int g_R = 0;
int g_G = 0;
int g_B = 15;

#define X_BEGIN                      \
do {                                 \
  digitalWrite(pin, HIGH);           \
  DELAY_US(delay_start_of_frame);    \
} while(0);

// 1: 20low, 10high
// 0: 10low, 20high
#define WRITE_BITS(bits, nbits)          \
do {                                     \
  for (int i = nbits - 1; i >= 0; --i) { \
    if ((bits >> i) & 0x01) {            \ 
      digitalWrite(pin, LOW);            \
      DELAY_US(delay_20us_low);          \
      digitalWrite(pin, HIGH);           \
      DELAY_US(delay_10us_high);         \
    } else {                             \
      digitalWrite(pin, LOW);            \
      DELAY_US(delay_10us_low);          \
      digitalWrite(pin, HIGH);           \
      DELAY_US(delay_20us_high);         \
    }                                    \
  }                                      \
} while (0);

#define X_END                     \
do {                              \
  digitalWrite(pin, LOW);         \
  DELAY_US(delay_end_of_frame);   \
} while(0);

void InitBulbs() {
  P("InitBulbs: start");
  for (int i = 0; i < N_BULBS_TO_INIT; ++i) {
    SetColor(i, Global::brightness, 0x00, 0x00, 0x00);
  }
  P("InitBulbs: stop");
}

void SetColor(int addr, int intensity, int R, int G, int B) {
  if (addr == 63) {
    // noop
  } else if (addr < 0 || addr >= N_BULBS) { return; }
  noInterrupts();
  X_BEGIN;
  WRITE_BITS(addr, 6);  // address
  WRITE_BITS(intensity, 8);  // intensity
  WRITE_BITS(B, 4);  // blue
  WRITE_BITS(G, 4);  // green
  WRITE_BITS(R, 4);  // red
  X_END;
  interrupts();
}

void SetAllColor(int intensity, int r, int g, int b) {
     for (int i = 0; i < N_BULBS; ++i) {
    SetColor(i, intensity, r, g, b);
  }
} 

int ParseDec(char* buf) {
  return  (buf[0] - '0')*10 + (buf [1] - '0');
}

int Parse3Dec(char* buf) {
  return  (buf[0] - '0')*100 + (buf[1] - '0')*10 + (buf [2] - '0');
}

bool ParseCommand(char *buf, int i) {
  if (buf[0] == 'i') {
    if (buf[1] == 'a') {
      InitBulbs();
    } else {
      int addr = ParseDec(buf+1);
      Serial.print("Init ");
      Serial.println(addr);
      SetColor(addr, 0xCC, 0x0D, 0x0D, 0x0D);
    }
  } else if (buf[0] == 'C') {
    switch(buf[1]) {
      case 'R':
        SetAllColor(0xcc, 0x0d, 0x00, 0x00);
        break;
      case 'G':
        SetAllColor(0xcc, 0x00, 0x0d, 0x00);
        break;
      case 'B':
        SetAllColor(0xcc, 0x00, 0x00, 0x0d);
        break;
      case 'O':
        SetAllColor(0xcc, 15, 01, 0);
        break;
    }
  } else if (buf[0] == 'c') {
    SetAllColor(0xcc, g_R, g_G, g_B);
  } else if (buf[0] == 'g') {
    g_R = ParseDec(buf+1);
    g_G = ParseDec(buf+3);
    g_B = ParseDec(buf+5);
    Serial.println("set global color");
  } /*else if (buf[0] == 'e') {
    int delay_msec = Parse3Dec(buf+1);
    RaceF(g_R, g_G, g_B, delay_msec);
    RaceB(g_R, g_G, g_B, delay_msec);
  } */else if (buf[0] == 'x') {
    Serial.println("Resetting");
    return true;
  } else if (buf[0] == 's') {
    int id = ParseDec(buf+1);
    int R = ParseDec(buf+3);
    int G = ParseDec(buf+5);
    int B = ParseDec(buf+7);
    SetColor(id, 0xcc, R, G, B);
  } else if (buf[0] == 'p') {
  } else {
    Serial.print("Unknown command: ");
    Serial.println(buf);
  }
  return false;
}


bool BlinkLed() {
  static boolean g_led_state = LOW;
  digitalWrite(led_pin, g_led_state);
  g_led_state = !g_led_state;
  return false;
}

// orange = 15,02,00
// lime green 04,15,00
// bluegreen 00, 10, 15
// NO lemon lime 07,15,00
// NO turqoise 00, 15, 10
// NO light orange 15, 03, 00
bool CommandLoop() {
  if (!Serial.available()) { return false; }
  char buf[64];
  int i = 0; 
  do {
    while (!Serial.available()) { }
    i += Serial.readBytesUntil('\n', buf+i, 64-i);
  } while (i == 0);
  buf[i] = '\0';
  return ParseCommand(buf, i);
}

void MainLoop() {
  start:
  Serial.println(F("**Start MainLooop**"));
  Global::ParseProgram();
  SetAllColor(Global::brightness, 0, 0, 0);
    
  while (1) {
    for (int cmd_idx = 0; cmd_idx < Global::n_commands; ++cmd_idx) {
      P2("cmd_idx: ", cmd_idx);
      switch (Global::commands[cmd_idx].type) {
        case Global::CMD_SOLID_COLORS: {
          P("  CMD: Solid Colors");
          P2("    n_colors: ", Global::commands[cmd_idx].data.solid_colors.n_colors);
          for (int i = 0; i < N_BULBS; ++i) {
            int cix = i % Global::commands[cmd_idx].data.solid_colors.n_colors;
            int ix = Global::commands[cmd_idx].data.solid_colors.colors[cix];
            SetColor(i, Global::brightness, Global::R[ix], Global::G[ix], Global::B[ix]);
          }
          break;
        }
        case Global::CMD_WAIT: {
          P("  CMD: Wait");
          if (PeriodicEvent::Delay(Global::commands[cmd_idx].data.wait.n_msec)) { goto start; }
          break;
        }
        case Global::CMD_SHIFT: {
          P("  CMD: Shift");
          int count = 0;
          while (count < Global::commands[cmd_idx].data.shift.max_shifts) {
            for (int i = 0; i < N_BULBS; ++i) {
              int cix = (i + count) % Global::commands[cmd_idx].data.shift.n_colors;
              int ix = Global::commands[cmd_idx].data.shift.colors[cix];
              SetColor(i, Global::brightness, Global::R[ix], Global::G[ix], Global::B[ix]);
            }
           if (PeriodicEvent::Delay(100)) { goto start; }
           ++count;
          }
          break;
        }
        case Global::CMD_RACE: {
          P("  CMD: Race");
          int start_idx;
          int end_idx;
          int step;
          if (Global::commands[cmd_idx].data.race.forwards) {
            start_idx = 0 - Global::commands[cmd_idx].data.race.n_colors;
            end_idx = N_BULBS;
            step = +1;
          } else {
            start_idx = N_BULBS;
            end_idx = 0 - Global::commands[cmd_idx].data.race.n_colors;
            step = -1;
          }
          int clear_idx = -1;
          while (1) {
            if (0 <= clear_idx && clear_idx < N_BULBS) {
              SetColor(clear_idx, Global::brightness, 0, 0, 0);
            }
            for (int i = 0; i < Global::commands[cmd_idx].data.race.n_colors; ++i) {
              int bulb_idx = start_idx + i;
              if (0 <= bulb_idx && bulb_idx < N_BULBS) {
                int cidx = Global::commands[cmd_idx].data.race.colors[i];
                SetColor(bulb_idx, Global::brightness, Global::R[cidx], Global::G[cidx], Global::B[cidx]);
              }
            }
            clear_idx = Global::commands[cmd_idx].data.race.forwards ? 
              start_idx : 
              start_idx + Global::commands[cmd_idx].data.race.n_colors - 1;
            if (start_idx == end_idx) {
              break;
            }
            start_idx += step;
            if (PeriodicEvent::Delay(Global::commands[cmd_idx].data.race.delay_msec)) { goto start; }
          }
          break;
        }
        case Global::CMD_FADE: {
          int target = Global::commands[cmd_idx].data.fade.dest_brightness;
          while (Global::brightness != target) {
            int step = (target > Global::brightness) ? +1 : -1;
            Global::brightness += step;
            SetColor(BROADCAST, Global::brightness, 0, 0, 0);
            if (PeriodicEvent::Delay(Global::commands[cmd_idx].data.fade.delay_msec)) { goto start; }
          }   
          break;
        }
        default:
          P2("Unknown command: ", Global::commands[cmd_idx].type);
          break;
      }
    }
  }
  Serial.println("Done p");
}

void setup() {
  pinMode(pin, OUTPUT);
  pinMode(led_pin, OUTPUT);
  Serial.begin(9600);
  delay(2000);
  InitBulbs();
  SetAllColor(0xcc, 15, 0, 0);
  PeriodicEvent::SetHandler(0, 1000, BlinkLed);
  PeriodicEvent::SetHandler(1, 1000, CommandLoop);
  PeriodicEvent::SetHandler(2, 1000, Wifi::WifiCallback);
}

void loop() {
  MainLoop();
 }


