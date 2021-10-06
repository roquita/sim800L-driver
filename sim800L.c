#include <stdio.h>
#include <stdbool.h>
#include "sim800L.h"
#include "macros.h"
#include "v25tr.h"
#include "tcp_ip.h"
#include "ip.h"
#include "3gpp_ts27007.h"
#include "special_simcom.h"
#include "gprs.h"

sim800L_err_t sim800L_init(sim800L_t *sim800L)
{
    SIM800L_TURN_ON(sim800L);
    sim800L->delay_ms(1000);
    SIM800L_HW_RESET(sim800L);
    sim800L->delay_ms(3000);

    SIM800L_ATE(sim800L, 0); // hold trash data from sim800L
    sim800L_err_t res_ate = SIM800L_ATE(sim800L, 0);
    sim800L_err_t res_at = SIM800L_AT(sim800L);
    bool res = res_ate == SIM800L_OK && res_at == SIM800L_OK;

    return res ? SIM800L_OK : SIM800L_ERROR;
}

sim800L_err_t sim800_link_net(sim800L_t *sim800L)
{ /*
AT+CIPSHUT ---> \CR\LFSHUT OK\CR\LF
AT+CIPMUX=0 ---> \CR\LFOK\CR\LF
AT+CGATT=1 ---> \CR\LFOK\CR\LF
AT+CSTT="entel.pe","","" ---> \CR\LFOK\CR\LF
AT+CIICR ---> \CR\LFOK\CR\LF
AT+CIFSR ---> \CR\LF10.207.242.114\CR\LF

*/
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
    res = SIM800L_CSTT(sim800L, "entel.pe", "", "");
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

sim800L_err_t sim800_tcp_get_request(sim800L_t *sim800L, char *domain, int port, char *tosend, char *torcv, int torcv_len)
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

    res = SIM800L_WAIT_FOR_BYTES(sim800L, torcv, torcv_len, 10000);// timeout depends of the server
#ifdef SIM800L_DEBUG
    printf("\n");
#endif
    if (res != SIM800L_OK)
        return res;

    SIM800L_CIPCLOSE(sim800L);

    return res;
}