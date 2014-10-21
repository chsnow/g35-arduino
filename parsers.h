#ifndef _PARSERS_H_
#define _PARSERS_H_
// TODO(chsnow): make this a separate compilation unit and not a giant #include hack.

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


#endif  // _PARSERS_H_

