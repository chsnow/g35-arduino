#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "Arduino.h"

#define P(a) do { Serial.println(a); } while (0);
#define P2(a, b) do { Serial.print(a); Serial.println(b); } while(0);
#define P3(a, b, c) do { \
  Serial.print(a); Serial.print(b); Serial.println(c); } while(0);
#define P4(a, b, c, d) do { \
  Serial.print(a); Serial.print(b); Serial.print(c); Serial.println(d); } while(0);
#define P8(a, b, c, d, e, f, g, h) do { \
  Serial.print(a); Serial.print(b); Serial.print(c); Serial.print(d); \
  Serial.print(e); Serial.print(f); Serial.print(g); Serial.println(h); } while(0);

#endif  // _DEBUG_H_
