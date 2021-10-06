
#include "main.h"

sim800L_err_t modem_delay_ms(int ms)
{
    for (int i = 0; i < ms; i++)
    {
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    return SIM800L_OK;
}
sim800L_err_t modem_pwrkey_gpio_set_level(int level)
{
    if (level == 0)
    {
        gpio_set_level(GPIO_NUM_4, 0);
    }
    else
    {
        gpio_set_level(GPIO_NUM_4, 1);
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
    const gpio_config_t pwrkey_config = {
        .pin_bit_mask = 1ULL << GPIO_NUM_4,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&pwrkey_config);

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
    modem.pwrkey_gpio_set_level = modem_pwrkey_gpio_set_level;
    modem.reset_gpio_set_level = modem_reset_gpio_set_level;
    modem.power_gpio_set_level = modem_power_gpio_set_level;
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

    res = sim800_link_net(&modem);
    printf("sim800_link_net , res = %u\n\n", res);
    if (res != SIM800L_OK)
    {
        printf("error while connecting to net. Restarting ...\n");
        esp_restart();
    }

    char torcv[1000] = {0};
    res = sim800_tcp_get_request(&modem, "exploreembedded.com", 80, "GET /wiki/images/1/15/Hello.txt HTTP/1.0\n\n", torcv, 1000);
    printf("sim800_link_net , res = %u\n\n", res);
    if (res == SIM800L_OK)
        printf("RECEIVED:\n\"%s\"\n", torcv);
}
