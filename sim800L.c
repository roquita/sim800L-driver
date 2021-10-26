#include <stdio.h>
#include <stdbool.h>
#include "priv/macros.h"
#include "priv/at/all.h"

sim800L_err_t sim800L_init(sim800L_t *sim800L)
{
    // hold-up at start-up
    sim800L->reset_gpio_set_level(1);

    // turn on power if it exists
    if (sim800L->power_gpio_set_level)
        sim800L->power_gpio_set_level(1);

    sim800L_err_t res;

    // disable sim800 echo
    for (int i = 0; i < 10; i++)
    {
        res = SIM800L_ATE(sim800L, 0);
        if (res == SIM800L_OK)
            break;
    }
    if (res != SIM800L_OK)
        return res;

    // check for sim800 module
    res = SIM800L_AT(sim800L);
    if (res != SIM800L_OK)
        return res;
  

    // check for sim card
    for (int i = 0; i < 5; i++)
    {
        res = SIM800L_CPIN_QUERY(sim800L);
        if (res == SIM800L_OK)
            break;
    }
    if (res != SIM800L_OK)
    {
        printf("%s, %s, %u\n", __FILE__, __func__, __LINE__);
        return res;
    }

    return SIM800L_OK;
}

sim800L_err_t sim800_link_net(sim800L_t *sim800L, char *apn, char *username, char *password, int mode)
{
    sim800L_err_t res;

    SIM800L_CIPSHUT(sim800L);

    res = SIM800L_CIPMUX(sim800L, 0);
    if (res != SIM800L_OK)
        return res;

    res = SIM800L_CIPMODE(sim800L, mode);
    if (res != SIM800L_OK)
        return res;

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

    printf("rssi: %i\n", rssi);
    return SIM800L_OK;
}

sim800L_err_t sim800_tcp_request(sim800L_t *sim800L, char *domain, int port,
                                 char *pre, uint8_t *body, int size, char *post,
                                 char *torcv, int torcv_len, int ssl, int mode)
{
    //SIM800L_CIPCLOSE(sim800L);

    sim800L_err_t res;

    if (ssl > 0)
    {
        res = SIM800L_CIPSSL(sim800L, ssl);
        if (res != SIM800L_OK)
            return res;
    }

    res = SIM800L_CIPSTART(sim800L, "TCP", domain, port, mode);
    if (res != SIM800L_OK)
    {
        //SIM800L_CIPCLOSE(sim800L);
        SIM800L_ATE(sim800L, 0);
        return res;
    }

    if (mode == 0) // normal
    {
        res = SIM800L_CIPSEND(sim800L, pre, (uint8_t *)body, size, post);
        if (res != SIM800L_OK)
            return res;
    }
    else // transparent
    {
        if (pre)
            sim800L->write((uint8_t *)pre, strlen(pre));
        if (body)
            sim800L->write(body, size);
        if (post)
            sim800L->write((uint8_t *)post, strlen(post));
        sim800L->delay_ms(1000);
        sim800L->write((uint8_t *)"+++", 3);
        sim800L->delay_ms(1000);
    }

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

sim800L_err_t sim800_enable_time_by_net(sim800L_t *sim800L)
{
    sim800L_err_t res;

    // check for net-time enabled
    int mode = -1;
    /*
    res = SIM800L_CLTS_QUERY(sim800L, &mode);
    if (res != SIM800L_OK)
    {
        printf("%s, %s, %u\n", __FILE__, __func__, __LINE__);
        return res;
    }

    // if not ,then enable it
    if (mode == 0)
    {
        */
    res = SIM800L_CLTS(sim800L, 1);
    if (res != SIM800L_OK)
    {
        printf("%s, %s, %u\n", __FILE__, __func__, __LINE__);
        return res;
    }

    // save in profile 0
    res = SIM800L_ATW(sim800L);
    if (res != SIM800L_OK)
        return res;

    res = SIM800L_CLTS_QUERY(sim800L, &mode);
    if (res != SIM800L_OK)
    {
        printf("%s, %s, %u\n", __FILE__, __func__, __LINE__);
        return res;
    }
    printf("mode: %i\n", mode);

    // reset sim800L
    sim800L->power_gpio_set_level(0);
    sim800L->delay_ms(100);
    sim800L->power_gpio_set_level(1);

    // check for sim800L
    for (int i = 0; i < 5; i++)
    {
        res = SIM800L_AT(sim800L);
        if (res == SIM800L_OK)
            break;
    }
    if (res != SIM800L_OK)
        return res;

    // verify if net-time enabled
    /*
        res = SIM800L_CLTS_QUERY(sim800L, &mode);
        if (res != SIM800L_OK || mode == 0)
        {
            printf("mode error: %i\n", mode);
            printf("%s, %s, %u\n", __FILE__, __func__, __LINE__);
            return res;
        }
        */
    /*
    }
*/
    return SIM800L_OK;
}

sim800L_err_t sim800_gettimestamp_by_net(sim800L_t *sim800L, uint32_t *timestamp)
{
    sim800L_err_t res;

    // read time
    res = SIM800L_CCLK_QUERY(sim800L);
    if (res != SIM800L_OK)
    {
        printf("%s, %s, %u\n", __FILE__, __func__, __LINE__);
        return res;
    }
    return SIM800L_OK;
}
