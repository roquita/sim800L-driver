
#include "main.h"
sim800L_err_t modem_write(uint8_t *bufer, int size)
{
    int txBytes = uart_write_bytes(UART_NUM_1, bufer, size);
    //printf("\n\ntyBytes : %i\n\n", txBytes);
    if (txBytes != size)
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
sim800L_err_t modem_delay_ms(int ms)
{
    for (int i = 0; i < ms; i++)
    {
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    return SIM800L_OK;
}
int64_t modem_get_time_ms()
{
    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    int64_t time_ms = (int64_t)tv_now.tv_sec * 1000L + (int64_t)tv_now.tv_usec / 1000L;
    return time_ms;
}
sim800L_err_t modem_pwrkey_gpio_set_level(int level)
{
    if (level == 0)
    {
        gpio_set_level(GPIO_NUM_13, 0);
    }
    else
    {
        gpio_set_level(GPIO_NUM_13, 1);
    }

    return SIM800L_OK;
}
void app_main(void)
{
    // HARDWARE SETUP

    const gpio_config_t pwrkey_config = {
        .pin_bit_mask = 1ULL << GPIO_NUM_13,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&pwrkey_config);

    static const int RX_BUF_SIZE = 1024;
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, 15, 14, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);

    // SIM800 INITIALIZATION
    sim800L_t modem;
    modem.write = modem_write;
    modem.read_byte = modem_read_byte;
    modem.available = modem_available;
    modem.flush = modem_flush;
    modem.delay_ms = modem_delay_ms;
    modem.get_time_ms = modem_get_time_ms;
    modem.ctrl.pwrkey_gpio_set_level = modem_pwrkey_gpio_set_level;
    modem.ctrl.type = SIM800L_CTRL_PWRKEY;

    sim800L_err_t res = sim800L_init(&modem);
    printf("sim800L_init , res = %u\n\n", res);

    // APPLICATION CODE
    res = sim800_gps_on(&modem);
    printf("sim800_gps_on , res = %u\n\n", res);

    while (1)
    {

        sim800_gps_read(&modem);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    printf("END\n");
}
