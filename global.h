#ifndef _GLOBAL_H_
#define _GLOBAL_H_
// TODO(chsnow): make this a separate compilation unit and not a giant #include hack.

#include "parsers.h"

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
  
  // current programs.
  //
  // Halloween colors:
  // orange = 15,02,00
  // lime green 04,15,00
  // bluegreen 00, 10, 15

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


#endif  // _GLOBAL_H_

