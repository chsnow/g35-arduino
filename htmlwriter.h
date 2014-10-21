#ifndef _HTMLWRITER_H_
#define _HTMLWRITER_H_
// TODO(chsnow): make this a separate compilation unit and not a giant #include hack.

#include <SPI.h>
#include <WiFi.h>
#include <avr/pgmspace.h>

#define HTMLWRITER_BUFFER_SZ (110)
namespace htmlwriter {
  //                          ====================================================================================================
  prog_char ps_1[] PROGMEM = "HTTP/1.0 200 OK";
  prog_char ps_2[] PROGMEM = "Content-type:text/html";
  prog_char ps_3[] PROGMEM = "";  // Required empty line.
  prog_char ps_4[] PROGMEM = "<html><body>";  
  prog_char ps_5[] PROGMEM = "<a href=\"/restart\">Restart</a><br>";
  prog_char ps_6[] PROGMEM = "Programs: <a href=\"/halloween\">Halloween</a>, <a href=\"/christmas\">Christmas</a><br>";
  prog_char ps_7[] PROGMEM = "<textarea rows=\"20\" cols=\"80\" name=\"code\" form=\"f\">";
  const int n_prefixes = 7;
  
  prog_char ss_1[] PROGMEM = "</textarea><form action=\"upload\" id=\"f\"><input type=\"submit\"></form>";
  prog_char ss_2[] PROGMEM = "</body></html>";
  prog_char ss_3[] PROGMEM = "";  // Required empty line.
  const int n_suffixes = 3;
  char buffer[HTMLWRITER_BUFFER_SZ];
 
  PROGMEM const char *prefix_string_table[] = {   
    ps_1,
    ps_2,
    ps_3,
    ps_4,
    ps_5,
    ps_6,
    ps_7 };
  PROGMEM const char *suffix_string_table[] = {   
    ss_1,
    ss_2,
    ss_3 };
  
  void Output(WiFiClient* client, char* main_string) {
    for (int i = 0; i < n_prefixes; i++) {
      strcpy_P(buffer, (char*)pgm_read_word(&(prefix_string_table[i])));
      client->println( buffer );
    }
    client->println(main_string);
    for (int i = 0; i < n_suffixes; i++) {
      strcpy_P(buffer, (char*)pgm_read_word(&(suffix_string_table[i])));
      client->println( buffer );
    }
  }
}  // namespace htmlwriter

#endif  // _HTMLWRITER_H_
