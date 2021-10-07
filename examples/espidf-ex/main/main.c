
#include "main.h"

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

sim800L_err_t modem_delay_ms(int ms)
{
    for (int i = 0; i < ms; i++)
    {
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    return SIM800L_OK;
}

sim800L_err_t modem_reset_gpio_set_level(int level)
{
    if (level == 0)
    {
        gpio_set_level(GPIO_NUM_5, 0);
    }
    else
    {
        gpio_set_level(GPIO_NUM_5, 1);
    }
    return SIM800L_OK;
}
sim800L_err_t modem_power_gpio_set_level(int level)
{
    if (level == 0)
    {
        gpio_set_level(GPIO_NUM_23, 0);
    }
    else
    {
        gpio_set_level(GPIO_NUM_23, 1);
    }

    return SIM800L_OK;
}
sim800L_err_t modem_send_string(char *string)
{
    int txBytes = uart_write_bytes(UART_NUM_1, string, strlen(string));
    if (txBytes <= 0)
        return SIM800L_INVALID_ARG;
    return SIM800L_OK;
}
sim800L_err_t modem_read_byte(char *byte)
{
    int rxBytes = uart_read_bytes(UART_NUM_1, byte, 1, 0);
    if (rxBytes < 0)
        return SIM800L_BUS_FAULT;
    return SIM800L_OK;
}
int modem_available()
{
    size_t cached = -1;
    uart_get_buffered_data_len(UART_NUM_1, &cached);
    return (int)cached;
}
sim800L_err_t modem_flush()
{
    esp_err_t res = uart_flush(UART_NUM_1);
    if (res != ESP_OK)
        return SIM800L_INVALID_ARG;
    return SIM800L_OK;
}

int64_t modem_get_time_ms()
{
    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    int64_t time_ms = (int64_t)tv_now.tv_sec * 1000L + (int64_t)tv_now.tv_usec / 1000L;
    return time_ms;
}

void app_main(void)
{
    const gpio_config_t reset_config = {
        .pin_bit_mask = 1ULL << GPIO_NUM_5,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&reset_config);

    const gpio_config_t power_config = {
        .pin_bit_mask = 1ULL << GPIO_NUM_23,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&power_config);

    static const int RX_BUF_SIZE = 1024;
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, 27, 26, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);

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

    res = sim800_link_net(&modem, "entel", "", "", 1);
    printf("sim800_link_net , res = %u\n\n", res);
    if (res != SIM800L_OK)
    {
        printf("error while connecting to net. Restarting ...\n");
        goto end;
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
        goto end;
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
        goto end;
    }
    printf("RECEIVED:\n\"%s\"\n\n", torcv);
    memset(torcv, 0, 1000);
#endif

#ifdef USE_SSL_POST_REQUEST
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
        goto end;
    }
    printf("RECEIVED:\n\"%s\"\n\n", torcv);
    memset(torcv, 0, 1000);
#endif

end:
    printf("END\n");
}
