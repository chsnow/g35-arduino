#include "Arduino.h"
#include <SPI.h>
#include <WiFi.h>
#include <avr/pgmspace.h>
#include "debug.h"
#include "global.h"
#include "parsers.h"
#include "wifi_http_server.h"
#include "htmlwriter.h"
#include "multitasking.h"

int g_led_pin = 13;
#define PHYSICAL_BROADCAST (63)  // Special G-35 bulb address which is used for global brightness control.

// Information about a single string of bulbs.
typedef struct StringInfo_t {
  int pin;
  int n_bulbs;
  bool first_bulb_is_zero;  // Normal or reverse addressing.
} StringInfo;

#define MAX_STRINGS (2)
// Information about all strings of bulbs connected to the Arduino.
typedef struct AllStringInfo_t {
  int n_strings;                 // Number of strings of bulbs.
  int first_addr[MAX_STRINGS];   // first logical address of bulbs on the ith string
  int last_addr[MAX_STRINGS];    // last logical address of bulbs on the ith string
  StringInfo info[MAX_STRINGS];  // Per-string info.
  int n_bulbs;                   // Total number of bulbs. Logical addresses are from 0 to (n_bulbs-1).
} AllStringInfo;
AllStringInfo g_string_info;

// Add a string of bulbs to the global config. Logical addresses are assigned sequentially 
// based on the order strings are added here.
void AddAndInitString(int pin, int n_bulbs, bool first_bulb_is_zero) {
  if (g_string_info.n_strings == MAX_STRINGS) {
    P(F("ERROR: too many strings!"));
    return;
  }
  // Save config info.
  int idx = g_string_info.n_strings;
  if (idx == 0) {
    g_string_info.first_addr[idx] = 0;
  } else {
    g_string_info.first_addr[idx] = g_string_info.last_addr[idx-1] + 1;
  }
  g_string_info.last_addr[idx] = g_string_info.first_addr[idx] + n_bulbs - 1;
  g_string_info.info[idx].pin = pin;
  g_string_info.info[idx].n_bulbs = n_bulbs;
  g_string_info.info[idx].first_bulb_is_zero = first_bulb_is_zero;
  g_string_info.n_bulbs += n_bulbs;
  g_string_info.n_strings++;
    
  // Initialize the string.
  pinMode(pin, OUTPUT);
  if (first_bulb_is_zero) {
    // Natural addressing: the first bulb from the power supply gets address 0.
    for (int i = 0; i < n_bulbs; ++i) {
      SetColorPhysical(pin, i, 0xcc, 15, 15, 15);
    }
  } else {
    // Reverse addressing: the first bulb from the power supply gets address "n_bulbs-1".
    for (int i = n_bulbs - 1; i >= 0; --i) {
      SetColorPhysical(pin, i, 0xcc, 15, 15, 15);
    }
  }
}

#define DELAY_US(x) delayMicroseconds(x);

// These timings were measured on a Mega 2560 using a logic analyzer, to get as close as possible 
// to the stock G-35 controller timings.
int delay_10us_high = 4;       // stock controller: 8.5us
int delay_10us_low = 7;        // stock controller: 11us
int delay_20us_low = 19;       // stock controller: 23us
int delay_20us_high = 17;      // stock controller: 21.5us
int delay_start_of_frame = 7;  // stock controller: 11.5us
int delay_end_of_frame = 170;  // stock controller: 188us

#define X_BEGIN(pin)                 \
do {                                 \
  digitalWrite(pin, HIGH);           \
  DELAY_US(delay_start_of_frame);    \
} while(0);

// 1: 20low, 10high
// 0: 10low, 20high
#define WRITE_BITS(pin, bits, nbits)     \
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

#define X_END(pin)                \
do {                              \
  digitalWrite(pin, LOW);         \
  DELAY_US(delay_end_of_frame);   \
} while(0);

void SetColorPhysical(int pin, int physical_addr, int intensity, int R, int G, int B) {
  noInterrupts();
  X_BEGIN(pin);
  WRITE_BITS(pin, physical_addr, 6);  // address
  WRITE_BITS(pin, intensity, 8);  // intensity
  WRITE_BITS(pin, B, 4);  // blue
  WRITE_BITS(pin, G, 4);  // green
  WRITE_BITS(pin, R, 4);  // red
  X_END(pin);
  interrupts();
}

void SetColor(int logical_addr, int intensity, int R, int G, int B) {
  if (logical_addr >= g_string_info.n_bulbs || logical_addr < 0) {
    P2("SetColor: invalid logical address ", logical_addr);
    return;
  }
  // Find physical addr.
  int idx = 0;
  while (g_string_info.last_addr[idx] < logical_addr) {
   idx++;
  }
  const int physical_addr = logical_addr - g_string_info.first_addr[idx];
  SetColorPhysical(g_string_info.info[idx].pin, physical_addr, intensity, R, G, B);
}

void SetAllColor(int intensity, int r, int g, int b) {
   for (int i = 0; i < g_string_info.n_bulbs; ++i) {
    SetColor(i, intensity, r, g, b);
  }
} 

void SetGlobalBrightness(int brightness) {
  for (int i = 0; i < g_string_info.n_strings; ++i) {
    SetColorPhysical(g_string_info.info[i].pin, PHYSICAL_BROADCAST, brightness, 0, 0, 0);  // RGB is ignored.
  }
}

// Blink the onboard LED. When the lights are powered off, this provides a visual indicator 
// that the control code is still running.
bool BlinkLed() {
  static boolean g_led_state = LOW;
  digitalWrite(g_led_pin, g_led_state);
  g_led_state = !g_led_state;
  return false;
}

bool ParseCommand(char *buf, int i) {
  if (buf[0] == 'x') {
    Serial.println("Resetting");
    return true;
  } else {
    Serial.print("Unknown command: ");
    Serial.println(buf);
  }
  return false;
}

// Very basic serial console handling. Add new commands to ParseCommand() function.
bool SerialCommandLoop() {
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

// This is the main G-35 driver loop. It parses the current global program script, then loops the program 
// endlessly. It restarts itself if the Cooperative Scheduler indicates there has been a global state 
// change [via the return value of PeriodicEvent::Delay()].
void MainLoop() {
  start:  // Yep, we are going to use goto. ;)
  
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
          for (int i = 0; i < g_string_info.n_bulbs; ++i) {
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
            for (int i = 0; i < g_string_info.n_bulbs; ++i) {
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
            end_idx = g_string_info.n_bulbs;
            step = +1;
          } else {
            start_idx = g_string_info.n_bulbs;
            end_idx = 0 - Global::commands[cmd_idx].data.race.n_colors;
            step = -1;
          }
          int clear_idx = -1;
          while (1) {
            if (0 <= clear_idx && clear_idx < g_string_info.n_bulbs) {
              SetColor(clear_idx, Global::brightness, 0, 0, 0);
            }
            for (int i = 0; i < Global::commands[cmd_idx].data.race.n_colors; ++i) {
              int bulb_idx = start_idx + i;
              if (0 <= bulb_idx && bulb_idx < g_string_info.n_bulbs) {
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
          P("  CMD: Fade");
          int target = Global::commands[cmd_idx].data.fade.dest_brightness;
          while (Global::brightness != target) {
            int step = (target > Global::brightness) ? +1 : -1;
            Global::brightness += step;
            SetGlobalBrightness(Global::brightness);
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
  pinMode(g_led_pin, OUTPUT);
  Serial.begin(9600);
  delay(2000);

  AddAndInitString(/* pin = */ 53, /* n_bulbs = */ 25, /* first_bulb_is_zero = */ true);
  // In my setup, the far end of the 50-bulb string is physically closest to the far end 
  // of the 25-bulb string, so use reverse address ordering for the 50-bulb string.
  AddAndInitString(/* pin = */ 31, /* n_bulbs = */ 50, /* first_bulb_is_zero = */ false);

  SetAllColor(0xcc, 15, 0, 0);  // POST color.
  
  // Register the various multitasking functions.
  PeriodicEvent::SetHandler(0, 1000, BlinkLed);
  PeriodicEvent::SetHandler(1, 1000, SerialCommandLoop);
  PeriodicEvent::SetHandler(2, 1000, Wifi::WifiCallback);
}

void loop() {
  MainLoop();
 }


