
#include "sim800L.h"

/*
  Connections for SIM800L-TTCALL BOARD:
    https://www.google.com/search?q=esp32+sim800+ttcall&tbm=isch&ved=2ahUKEwjdlNG4z7bzAhV4jJUCHQIFBg4Q2-cCegQIABAA&oq=esp32+sim800+ttcall&gs_lcp=CgNpbWcQAzoHCCMQ7wMQJzoLCAAQgAQQsQMQgwE6CAgAEIAEELEDOgQIABADOgUIABCABDoECAAQQzoECAAQHjoGCAAQBRAeUPfYGVjclxpgp5kaaABwAHgAgAFaiAGkDJIBAjE5mAEAoAEBqgELZ3dzLXdpei1pbWfAAQE&sclient=img&ei=hgZeYd25KPiY1sQPgoqYcA&bih=955&biw=1920&client=firefox-b-d#imgrc=X2fIX6tJqOWVlM

    sim800L                                esp32
    rx(2.8v)  <--(voltage divider)---   gpio 27 (3.3v)
    tx(2.8v)  --------(direct)------>   gpio 26 (3.3v)
    rst       <--------(direct)------   gpio 5(3.3v)   [can be replaced by a pull-up to 3.3v]
    pwrkey                              nc
    status                              nc
    netlight                            nc

    other:
    power switch <--------(direct)------ gpio23 (PRESENT ON SIM800-TTCALL BOARD)

  Connections for SIM800L classic board:
    https://www.google.com/search?q=sim800+module&client=firefox-b-d&sxsrf=AOaemvIWh51ENEigm7VYjvi5q5vsd-GAkg:1633552800022&source=lnms&tbm=isch&sa=X&ved=2ahUKEwjKsPey0rbzAhX9I7kGHSyxC9AQ_AUoAXoECAEQAw&biw=1920&bih=955&dpr=1#imgrc=N5-DbuJ5iQLeiM

    sim800L                                esp32
    rx(2.8v)  <--(voltage divider)---   gpio 27 (3.3v)
    tx(2.8v)  --------(direct)------>   gpio 26 (3.3v)
    rst       <--------(direct)------   gpio 5(3.3v)   [can be replaced by a pull-up to 3.3v]
    pwrkey                              nc
    status                              nc
    netlight                            nc

    - delete or change all about gpio-23
    - set this "modem.power_gpio_set_level = NULL"
*/

// Select what you want to use:
//#define USE_GET_REQUEST
//#define USE_POST_REQUEST
//#define USE_SSL_POST_REQUEST

// Remeber to link_init() need your APN credentials, depends on your sim card provider
// Inside "sim800L.h", "#define SIM800L_DEGUB"  may be uncommented to get verbose print on terminal

sim800L_err_t modem_delay_ms(int ms)
{
  for (int i = 0; i < ms; i++)
  {
    delay(1);
  }
  return SIM800L_OK;
}

sim800L_err_t modem_reset_gpio_set_level(int level)
{
  digitalWrite(5, level);

  return SIM800L_OK;
}
sim800L_err_t modem_power_gpio_set_level(int level)
{
  digitalWrite(23, level);

  return SIM800L_OK;
}
sim800L_err_t modem_send_string(char *string)
{
  int txBytes = Serial2.write((uint8_t *)string, strlen(string));
  if (txBytes <= 0)
    return SIM800L_INVALID_ARG;
  return SIM800L_OK;
}
sim800L_err_t modem_read_byte(char *byte)
{
  int result = Serial2.read();

  *byte = (char)result;
  return SIM800L_OK;
}
int modem_available()
{
  return (int)Serial2.available();
}
sim800L_err_t modem_flush()
{
  while (Serial2.available() > 0)
    Serial2.read();
  return SIM800L_OK;
}
int64_t modem_get_time_ms()
{
  return (int64_t)millis();
}
void finish() {
  printf("END\n");
  while (1) {
    delay(1000);
  }
}
void setup()
{
  Serial.begin(115200); // terminal

  pinMode(5, OUTPUT);
  pinMode(23, OUTPUT);
  Serial2.begin(9600, SERIAL_8N1, 26, 27); // sim800L

  sim800L_t modem;
  modem.send_string = modem_send_string;
  modem.read_byte = modem_read_byte;
  modem.available = modem_available;
  modem.flush = modem_flush;
  modem.delay_ms = modem_delay_ms;
  modem.reset_gpio_set_level = modem_reset_gpio_set_level;
  modem.power_gpio_set_level = modem_power_gpio_set_level; // for SIM800-TTCALL BOARD
  //modem.power_gpio_set_level = NULL; // for classic sim800L board
  modem.get_time_ms = modem_get_time_ms;

  sim800L_err_t res = sim800L_init(&modem);
  printf("sim800L_init , res = %u\n\n", res);
  if (res != SIM800L_OK)
  {
    printf("init failed. Restarting ...\n");
    finish();
  }

  int mvolts = -1;
  res = sim800_battery_level(&modem, &mvolts);
  printf("sim800_battery_level , res = %u, mvolts = %i\n\n", res, mvolts);

  int status = -1;
  int adc = -1;
  res = sim800_read_adc(&modem, &status, &adc);
  printf("sim800_read_adc , res = %u, status = %i, adc= %imv\n\n", res, status, adc);

  res = sim800_wait_until_detect_signal(&modem, 10000);
  printf("sim800_wait_until_detect_signal, res = %u\n", res);
  if (res != SIM800L_OK)
  {
    printf("timeout on detect signal. Restarting ...\n");
    finish();
  }

  res = sim800_link_net(&modem,  "entel", "", "", 1);
  printf("sim800_link_net , res = %u\n\n", res);
  if (res != SIM800L_OK)
  {
    printf("error while connecting to net. Restarting ...\n");
    finish();
  }

  char torcv[1000] = {0};

#ifdef USE_GET_REQUEST
  res = sim800_tcp_request(&modem, "exploreembedded.com", 80,
                           "GET /wiki/images/1/15/Hello.txt HTTP/1.0\n\n",
                           NULL,
                           NULL,
                           torcv, 1000, 0, 1);
  printf("sim800_tcp_get_test , res = %u\n\n", res);
  if (res != SIM800L_OK)
  {
    printf("sim800_tcp_get_test failed. Restarting ...\n");
    finish();
  }
  printf("RECEIVED:\n\"%s\"\n\n", torcv);
  memset(torcv, 0, 1000);
#endif

#ifdef USE_POST_REQUEST
  res = sim800_tcp_request(&modem, "ptsv2.com", 80,
                           "POST /t/7jv8h-1545446925/post HTTP/1.0\n"
                           "Accept: */*\n"
                           "Host: ptsv2.com\n"
                           "Content-Length:7\n"
                           "Content-Type: application/x-www-form-urlencoded\n\n",
                           "temp=89",
                           "\n\n",
                           torcv, 1000, 0, 1);
  printf("sim800_tcp_post_test , res = %u\n\n", res);
  if (res != SIM800L_OK)
  {
    printf("sim800_tcp_post_test failed. Restarting ...\n");
    finish();
  }
  printf("RECEIVED:\n\"%s\"\n\n", torcv);
  memset(torcv, 0, 1000);
#endif

#ifdef USE_SSL_POST_REQUEST

  // "Authorization: Bearer" depends on your personal dropbox account
  char body[] = "temp=123456789xxyy";
  char pre[500] = {0};
  snprintf(pre, 500, "POST /2/files/upload HTTP/1.1\n"
           "Host: content.dropboxapi.com\n"
           "Authorization: Bearer vRV42LzfriIAAAAAAAAAAYu60VWQh7B9HUiw29lu-GEOvhspHrvOKVQ13fuS1p55\n"
           "Dropbox-API-Arg: {\"path\": \"/test.txt\","
           "\"mode\": \"add\","
           "\"autorename\": true,"
           "\"mute\": false,"
           "\"strict_conflict\": false}\n"
           "Content-Type: application/octet-stream\n"
           "Content-Length: %u\n"
           "Connection: close\n\n",
           strlen(body));

  res = sim800_tcp_request(&modem, "content.dropboxapi.com", 443,
                           pre,
                           body,
                           NULL,
                           torcv, 1000, 1, 1);
  printf("dropbox upload file test , res = %u\n\n", res);
  if (res != SIM800L_OK)
  {
    printf("dropbox upload file test failed. Restarting ...\n");
    finish();
  }
  printf("RECEIVED:\n\"%s\"\n\n", torcv);
  memset(torcv, 0, 1000);
#endif

}

void loop()
{
  // put your main code here, to run repeatedly:
  delay(1000);
}

