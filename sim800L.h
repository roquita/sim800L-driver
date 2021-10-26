#pragma once

#define SIM800L_DEBUG

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        SIM800L_OK,
        SIM800L_ERROR,
        SIM800L_TIMEOUT,
        SIM800L_BUS_FAULT,
        SIM800L_INVALID_ARG,
        SIM800L_OVERFLOW,
    } sim800L_err_t;

    typedef sim800L_err_t (*sim800L_write_t)(uint8_t *bufer, int size);
    typedef sim800L_err_t (*sim800L_read_byte_t)(char *byte);
    typedef int (*sim800L_available_t)(void);
    typedef sim800L_err_t (*sim800L_flush_t)(void);
    typedef sim800L_err_t (*sim800L_delay_ms_t)(int ms);
    typedef sim800L_err_t (*sim800L_reset_gpio_set_level_t)(int level);
    typedef sim800L_err_t (*sim800L_power_gpio_set_level_t)(int level);
    typedef int64_t (*sim800L_get_time_ms_t)(void);

    typedef struct
    {
        sim800L_write_t write;
        sim800L_read_byte_t read_byte;
        sim800L_available_t available;
        sim800L_flush_t flush;
        sim800L_delay_ms_t delay_ms;
        sim800L_get_time_ms_t get_time_ms;
        sim800L_reset_gpio_set_level_t reset_gpio_set_level;
        sim800L_power_gpio_set_level_t power_gpio_set_level;
    } sim800L_t;

    sim800L_err_t sim800L_init(sim800L_t *sim800L);
    sim800L_err_t sim800_link_net(sim800L_t *sim800L, char *apn, char *username, char *password, int mode);
    sim800L_err_t sim800_battery_level(sim800L_t *sim800L, int *mvolt);
    sim800L_err_t sim800_read_adc(sim800L_t *sim800L, int *status, int *value);
    sim800L_err_t sim800_wait_until_detect_signal(sim800L_t *sim800L, int timeout_ms);
    sim800L_err_t sim800_tcp_request(sim800L_t *sim800L, char *domain, int port,
                                     char *pre, uint8_t *body, int size, char *post,
                                     char *torcv, int torcv_len, int ssl, int mode);
    sim800L_err_t sim800_enable_time_by_net(sim800L_t *sim800L);
    sim800L_err_t sim800_gettimestamp_by_net(sim800L_t *sim800L, uint32_t *timestamp);

#ifdef __cplusplus
}
#endif