#pragma once

#include "../macros.h"

static inline sim800L_err_t SIM800L_CBC(sim800L_t *sim800L, int *bcs, int *bcl, int *mvolt)
{
    char response[50] = {0};
    char at_cmd[10] = {0};

    // MAKE AT COMMAND
    sprintf(at_cmd, "AT+CBC");

    // SEND AND RECEIVE ANSWER
    sim800L_err_t res;
    res = SIM800L_SEND_AT_CMD(sim800L, at_cmd, response,  sizeof(response) / sizeof(char), 2, 2000);

    if (res != SIM800L_OK)
        return res;

    // PARSE ANSWER
    char *cbc = strtok(response, "\r\n:,");
    char *arg1 = strtok(NULL, "\r\n:,");
    char *arg2 = strtok(NULL, "\r\n:,");
    char *arg3 = strtok(NULL, "\r\n:,");
    char *ok = strtok(NULL, "\r\n:,");

    if (cbc == 0 || strcmp(cbc, "+CBC") != 0)
        return SIM800L_ERROR;
    if (ok == 0 || strcmp(ok, "OK") != 0)
        return SIM800L_ERROR;

    if (bcs)
        *bcs = atoi(arg1);
    if (bcl)
        *bcl = atoi(arg2);
    if (mvolt)
        *mvolt = atoi(arg3);

    return SIM800L_OK;
}

static inline sim800L_err_t SIM800L_CSQ(sim800L_t *sim800L, int *rssi, int *ber)
{
    char response[50] = {0};
    char at_cmd[10] = {0};

    // MAKE AT COMMAND
    sprintf(at_cmd, "AT+CSQ");

    // SEND AND RECEIVE ANSWER
    sim800L_err_t res;
    res = SIM800L_SEND_AT_CMD(sim800L, at_cmd, response,  sizeof(response) / sizeof(char), 2, 1000);

    if (res != SIM800L_OK)
        return res;

    // PARSE ANSWER
    char *csq = strtok(response, "\r\n:,");
    char *arg1 = strtok(NULL, "\r\n:,");
    char *arg2 = strtok(NULL, "\r\n:,");
    char *ok = strtok(NULL, "\r\n:,");

    if (csq == 0 || strcmp(csq, "+CSQ") != 0)
        return SIM800L_ERROR;
    if (ok == 0 || strcmp(ok, "OK") != 0)
        return SIM800L_ERROR;

    if (rssi)
        *rssi = atoi(arg1);
    if (ber)
        *ber = atoi(arg2);

    return SIM800L_OK;
}