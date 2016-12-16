#ifndef _HTMLWRITER_H_
#define _HTMLWRITER_H_
// TODO(chsnow): make this a separate compilation unit and not a giant #include hack.

#include <SPI.h>
#include <WiFi.h>
#include <avr/pgmspace.h>

#define HTMLWRITER_BUFFER_SZ (110)
namespace htmlwriter {
  //                          ====================================================================================================
  const char ps_1[] PROGMEM = "HTTP/1.0 200 OK";
  const char ps_2[] PROGMEM = "Content-type:text/html";
  const char ps_3[] PROGMEM = "";  // Required empty line.
  const char ps_4[] PROGMEM = "<html><body>";  
  const char ps_5[] PROGMEM = "<a href=\"/restart\">Restart</a><br>";
  const char ps_6[] PROGMEM = "Programs: <a href=\"/halloween\">Halloween</a>, <a href=\"/christmas\">Christmas</a><br>";
  const char ps_7[] PROGMEM = "<textarea rows=\"20\" cols=\"80\" name=\"code\" form=\"f\">";
  const int n_prefixes = 7;
  
  const char ss_1[] PROGMEM = "</textarea><form action=\"upload\" id=\"f\"><input type=\"submit\"></form>";
  const char ss_2[] PROGMEM = "</body></html>";
  const char ss_3[] PROGMEM = "";  // Required empty line.
  const int n_suffixes = 3;
  char buffer[HTMLWRITER_BUFFER_SZ];
 
  PGM_P const prefix_string_table[] PROGMEM = {   
    ps_1,
    ps_2,
    ps_3,
    ps_4,
    ps_5,
    ps_6,
    ps_7 };
  PGM_P const suffix_string_table[] PROGMEM = {   
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
