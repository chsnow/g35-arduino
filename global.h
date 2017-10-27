#ifndef _GLOBAL_H_
#define _GLOBAL_H_
// TODO(chsnow): make this a separate compilation unit and not a giant #include hack.

#include "parsers.h"

// Pins:
// Light strings:  22, 24, 26, 28

namespace Global {
typedef enum SelectedProgram_e {
  PROGRAM_HALLOWEEN = 1,     // 001
  PROGRAM_THANKSGIVING,      // 010
  PROGRAM_CHRISTMAS,         // 011
  PROGRAM_VALENTINES,        // 100
  PROGRAM_STPATRICKS,        // 101
  PROGRAM_RAINBOW,           // 110
  PROGRAM_EASTER,            // 111
  PROGRAM_TESTING
} SelectedProgram;
SelectedProgram selected_program = PROGRAM_STPATRICKS;

typedef enum CommandType_e {
  CMD_SOLID_COLORS = 1,  // S
  CMD_SHIFT = 2,  // F
  CMD_RACE = 3,  // R or E
  CMD_FADE = 4,  // A
  CMD_WAIT = 5,  // W
  CMD_TWINKLE = 6, // T
} CommandType;

static const byte kMaxSolidColors = 16;
typedef struct SolidColorsData_t {
  byte n_colors;
  byte colors[kMaxSolidColors];
} SolidColorsData;

typedef struct WaitData_t {
  int n_msec;
} WaitData;

static const byte kMaxShiftColors = 32;
typedef struct ShiftData_t {
  int n_colors;
  byte colors[kMaxShiftColors];
  int max_shifts;  // default is 50
} ShiftData;

static const byte kMaxTwinkleColors = 2;
typedef struct TwinkleData_t {
  int n_colors;
  byte colors[kMaxTwinkleColors];
  int max_shifts;  // default is 50
  int percent;  // default is 10
} TwinkleData;

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
  TwinkleData twinkle;
} CommandData;
typedef struct Command_s {
  CommandType type;
  CommandData data;
} Command;

static const int kMaxCommands = 96;
int n_commands;
Command commands[kMaxCommands];

static const int kMaxColors = 16;
int n_colors;
int R[kMaxColors];
int G[kMaxColors];
int B[kMaxColors];

int brightness = 0xcc;  // Global brightness

// current programs.
//
// Halloween colors:
// orange = 15,02,00
// lime green 04,15,00
// bluegreen 00, 10, 15

// Monster Halloween: const char program_halloween[] PROGMEM = "C000;CF20;C4F0;C00F;S123;A204;W5000;F11112222333300000000000000000000,150;S123;A204;W10000;S0;F123000;S0;A0,0;S1;A204;W1000;A0;S2;A204;W1000;A0;S3;A204;W1000;A0;S0;A204,0;F123000;S0;A0,0;S123;A204;W1000;A0;S231;A204;W1000;A0;S312;A204;W1000;A0;S123;A204;";
// 2016
const char program_halloween[] PROGMEM = "C000;CF20;CFF9;CF20;S123;A204;W5000;F33331111222233330000000000000000,150;S123;A204;W10000;S0;F123000;S0;A0,0;S1;A204;W1000;A0;S2;A204;W1000;A0;S3;A204;W1000;A0;S0;A204,0;F123000;S0;A0,0;S123;A204;W1000;A0;S231;A204;W1000;A0;S312;A204;W1000;A0;S123;A204;";

// older 
// const char program_thanksgiving[] PROGMEM = "C000;CE30;CF00;CF90;S123;A204;W5000;F11112222333300000000000000000000,150;S123;A204;W10000;S0;F123000;S0;A0,0;S1;A204;W1000;A0;S2;A204;W1000;A0;S3;A204;W1000;A0;S0;A204,0;F123000;S0;A0,0;S123;A204;W1000;A0;S231;A204;W1000;A0;S312;A204;W1000;A0;S123;A204;";
// 2016
const char program_thanksgiving[] PROGMEM = "C000;CF20;CF00;S12;A204;W5000;F111122220000000000000000,150;S12;A204;W10000;S0;F1200;S0;A0,0;S1;A204;W1000;A0;S2;A204;W1000;A0;S0;A204,0;F1200;S0;A0,0;S12;A204;W1000;A0;S21;A204;W1000;A0;S12;A204;";

// 2014: const char program_christmas[] PROGMEM = "C000;CF00;C0F0;CFFF;S123;A204;W5000;F33331111222233330000000000000000,150;S123;A204;W10000;S0;F123000;S0;A0,0;S1;A204;W1000;A0;S2;A204;W1000;A0;S3;A204;W1000;A0;S0;A204,0;F123000;S0;A0,0;S123;A204;W1000;A0;S231;A204;W1000;A0;S312;A204;W1000;A0;S123;A204;";
// 2015 const char program_christmas[] PROGMEM = "C000;CFF9;CF00;CFF9;S123;A204;W5000;F33331111222233330000000000000000,150;S123;A204;W10000;S0;F123000;S0;A0,0;S1;A204;W1000;A0;S2;A204;W1000;A0;S3;A204;W1000;A0;S0;A204,0;F123000;S0;A0,0;S123;A204;W1000;A0;S231;A204;W1000;A0;S312;A204;W1000;A0;S123;A204;";
// 2016 
const char program_christmas[] PROGMEM = "C000;CF00;C0F0;CF00;CC82;CFFF;T45,400,10;S123;A204;W5000;F1111222233330000000000000000,150;S123;A204;W10000;S0;F123000;S0;A0,0;S1;A204;W1000;A0;S2;A204;W1000;A0;S3;A204;W1000;A0;S0;A204,0;F123000;S0;A0,0;S123;A204;W1000;A0;S231;A204;W1000;A0;S312;A204;W1000;A0;S123;A204;";


// Warm White: CC82

// iTwinkle test program
//const char program_christmas[] PROGMEM = "";

const char program_valentines[] PROGMEM = "C000;CF00;CF25;CFFB;S123;A204;W5000;F33331111222233330000000000000000,150;S123;A204;W10000;S0;F123000;S0;A0,0;S1;A204;W1000;A0;S2;A204;W1000;A0;S3;A204;W1000;A0;S0;A204,0;F123000;S0;A0,0;S123;A204;W1000;A0;S231;A204;W1000;A0;S312;A204;W1000;A0;S123;A204;";

// Rainbow + Green/White/Orange + Green/White
//                                         0=bl 1=gr 2=wi 3=or 4=re 5=bl 6=ye 7=pu
const char program_stpatricks[] PROGMEM = "C000;C0F0;CFF9;CF20;CF00;C00F;CFF0;CA0F;S0;F4443336661115557770000000000,150;S123;A204;W5000;F1111222233330000000000000000,150;S123;A204;W10000;S0;F121000;S0;A0,0;S1;A204;W1000;A0;S2;A204;W1000;A0;S0;A204,0;F121000;";

// lightpink, green, yellow, purple, blue, orange
const char program_easter[] PROGMEM = "C000;CF05;C0F0;CFF0;CA0F;C0CF;CF30;S123456;A204;W10000;F1122334455660000000000000,100;S123456;A204;W10000;F12345600;";
static const int kMaxProgramBuf = 1024;
char program_str[kMaxProgramBuf] = "";


// High:           52, 50, 48
// Low:            46, 44, 42
// Settings:       34, 36, 38  (34 = high-order bit)
void CheckSettings() {
  pinMode(42, OUTPUT); digitalWrite(42, LOW);
  pinMode(44, OUTPUT); digitalWrite(44, LOW);
  pinMode(46, OUTPUT); digitalWrite(46, LOW);
  pinMode(48, OUTPUT); digitalWrite(48, HIGH);
  pinMode(50, OUTPUT); digitalWrite(50, HIGH);
  pinMode(52, OUTPUT); digitalWrite(52, HIGH);

  pinMode(34, INPUT); pinMode(36, INPUT); pinMode (38, INPUT);
  unsigned int value = 0;
  if (digitalRead(34) == HIGH) { value += 4; }
  if (digitalRead(36) == HIGH) { value += 2; }
  if (digitalRead(38) == HIGH) { value += 1; }

  selected_program = value;
}

void FetchCurrentProgram() {
  P("Fetching program");
  CheckSettings();
  switch (selected_program) {
    case PROGRAM_HALLOWEEN: {
        P("  halloween");
        strcpy_P(program_str, program_halloween);
        break;
      }
    case PROGRAM_THANKSGIVING: {
        P("  thanksgiving");
        strcpy_P(program_str, program_thanksgiving);
        break;
      }
    case PROGRAM_CHRISTMAS: {
        P("  christmas");
        strcpy_P(program_str, program_christmas);
        break;
      }
    case PROGRAM_VALENTINES: {
        P("  valentines");
        strcpy_P(program_str, program_valentines);
        break;
      }
    case PROGRAM_STPATRICKS: {
        P("  stpatricks");
        strcpy_P(program_str, program_stpatricks);
        break;
      }
    case PROGRAM_EASTER: {
        P("  easter");
        strcpy_P(program_str, program_easter);
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
            msec = msec * 10 + (c - '0');
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
              max_shifts = 10 * max_shifts + (c - '0');
            }
          }
          P2("  Max Shifts: ", max_shifts);
          commands[n_commands].data.shift.max_shifts = max_shifts;
          n_commands++;
          break;
        }
      case 'T': {  // Twinkle
          P(F("CMD: Twinkle"));
          commands[n_commands].type = CMD_TWINKLE;
          int n_colors = 0;
          while ((c = buf[i++]) != ';' && c != ',') {
            P4("  Color ", n_colors, ": ", Parse1Hex(c));
            commands[n_commands].data.twinkle.colors[n_colors++] = Parse1Hex(c);
          }
          commands[n_commands].data.twinkle.n_colors = n_colors;
          int max_shifts = 50;
          if (c == ',') {
            max_shifts = 0;
            while ((c = buf[i++]) != ';' && c != ',') {
              max_shifts = 10 * max_shifts + (c - '0');
            }
          }
          P2("  Shifts: ", max_shifts);
          commands[n_commands].data.twinkle.max_shifts = max_shifts;
          int percent = 10;
          if (c == ',') {
            percent = 0;
            while ((c = buf[i++]) != ';') {
              percent = 10 * percent + (c - '0');
            }
          }
          P2("  Percent: ", percent);
          commands[n_commands].data.twinkle.percent = percent;
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
              delay_msec = 10 * delay_msec + (c - '0');
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
            dest_brightness = 10 * dest_brightness + (c - '0');
          }
          if (dest_brightness > 0xcc) {
            dest_brightness = 0xcc;  // =204
          }
          commands[n_commands].data.fade.dest_brightness = dest_brightness;
          int delay_msec = 5;
          if (c == ',') {
            delay_msec = 0;
            while ((c = buf[i++]) != ';') {
              delay_msec = 10 * delay_msec + (c - '0');
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


#endif  // _GLOBAL_H_

