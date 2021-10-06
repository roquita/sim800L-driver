#include <stdio.h>
#include <stdbool.h>
#include "priv/macros.h"
#include "priv/at/all.h"

sim800L_err_t sim800L_init(sim800L_t *sim800L)
{
    // hold-up at start-up
    sim800L->reset_gpio_set_level(1);

    if (sim800L->power_gpio_set_level)
        sim800L->power_gpio_set_level(1);

    sim800L_err_t res;
    for (int i = 0; i < 10; i++)
    {
        res = SIM800L_ATE(sim800L, 0);
        if (res == SIM800L_OK)
            break;
    }
    if (res != SIM800L_OK)
        return res;

    res = SIM800L_AT(sim800L);
    if (res != SIM800L_OK)
        return res;

    return SIM800L_OK;
}

sim800L_err_t sim800_link_net(sim800L_t *sim800L, char *apn, char *username, char *password)
{
    sim800L_err_t res;

    SIM800L_CIPSHUT(sim800L);

    res = SIM800L_CIPMUX(sim800L, 0);
    if (res != SIM800L_OK)
        return res;
    /*
    res = SIM800L_CGATT(sim800L, 1);
    if (res != SIM800L_OK)
        return res;
*/
    res = SIM800L_CSTT(sim800L, apn, username, password);
    if (res != SIM800L_OK)
        return res;

    res = SIM800L_CIICR(sim800L);
    if (res != SIM800L_OK)
        return res;

    char ip_addr[20] = {0};
    res = SIM800L_CIFSR(sim800L, ip_addr, 20);
    if (res != SIM800L_OK)
        return res;

    return SIM800L_OK;
}

sim800L_err_t sim800_battery_level(sim800L_t *sim800L, int *mvolt)
{
    return SIM800L_CBC(sim800L, NULL, NULL, mvolt);
}

sim800L_err_t sim800_read_adc(sim800L_t *sim800L, int *status, int *value)
{
    return SIM800L_CADC(sim800L, status, value);
}

sim800L_err_t sim800_wait_until_detect_signal(sim800L_t *sim800L, int timeout_ms)
{
    int rssi = -1;
    int64_t start = sim800L->get_time_ms();
    do
    {
        SIM800L_CSQ(sim800L, &rssi, NULL);
        //printf("res = %u , rssi:%i , ber:%i\n", res, rssi, ber);
        sim800L->delay_ms(1000);

        if (sim800L->get_time_ms() - start >= timeout_ms)
            return SIM800L_TIMEOUT;

    } while (!(rssi >= 1 && rssi <= 30));

    return SIM800L_OK;
}

sim800L_err_t sim800_tcp_http_request(sim800L_t *sim800L, char *domain, int port, char *tosend, char *torcv, int torcv_len)
{
    sim800L_err_t res;

    res = SIM800L_CIPSTART(sim800L, "TCP", domain, port);
    if (res != SIM800L_OK)
        return res;

    res = SIM800L_CIPSEND(sim800L, tosend);
    if (res != SIM800L_OK)
        return res;

#ifdef SIM800L_DEBUG
    printf("\nwaiting for server response...\n");
#endif

    res = SIM800L_WAIT_FOR_BYTES(sim800L, "\r\nCLOSED\r\n", torcv, torcv_len, 15000);
#ifdef SIM800L_DEBUG
    printf("\n");
#endif
    if (res != SIM800L_OK)
        return res;

    SIM800L_CIPCLOSE(sim800L);

    return res;
}
