
#include "sim800L.h"

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
    esp_restart();
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
    esp_restart();
  }

  res = sim800_link_net(&modem, "entel", "", "", 0);
  printf("sim800_link_net , res = %u\n\n", res);
  if (res != SIM800L_OK)
  {
    printf("error while connecting to net. Restarting ...\n");
    esp_restart();
  }

  char torcv[1000] = {0};
  res = sim800_tcp_http_request(&modem, "exploreembedded.com", 80, "GET /wiki/images/1/15/Hello.txt HTTP/1.0\n\n", torcv, 1000, 0, 0);
  printf("sim800_tcp_get_test , res = %u\n\n", res);
  if (res == SIM800L_OK)
    printf("RECEIVED:\n\"%s\"\n\n", torcv);
  memset(torcv, 0, 1000);

  res = sim800_tcp_http_request(&modem, "ptsv2.com", 80,
                                "POST /t/7jv8h-1545446925/post HTTP/1.0\n"
                                "Accept:*/*\n"
                                "Host: ptsv2.com\n"
                                "Content-Length:7\n"
                                "Content-Type: application/x-www-form-urlencoded\n\n"
                                "temp=89\n\n",
                                torcv, 1000, 0, 0);
  printf("sim800_tcp_post_test , res = %u\n\n", res);
  if (res == SIM800L_OK)
    printf("RECEIVED:\n\"%s\"\n\n", torcv);
  memset(torcv, 0, 1000);
}

void loop()
{
  // put your main code here, to run repeatedly:
  delay(1000);
}
