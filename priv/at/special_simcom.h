#pragma once

#include "../macros.h"

static inline sim800L_err_t SIM800L_CADC(sim800L_t *sim800L, int *status, int *value)
{
    char response[50] = {0};
    char at_cmd[10] = {0};

    // MAKE AT COMMAND
    sprintf(at_cmd, "AT+CADC?");

    // SEND AND RECEIVE ANSWER
    sim800L_err_t res;
    res = SIM800L_SEND_AT_CMD(sim800L, at_cmd, response, sizeof(response) / sizeof(char), 2, 2000);

    if (res != SIM800L_OK)
        return res;

    // PARSE ANSWER
    char *cadc = strtok(response, "\r\n:,");
    char *arg1 = strtok(NULL, "\r\n:,");
    char *arg2 = strtok(NULL, "\r\n:,");
    char *ok = strtok(NULL, "\r\n:,");

    if (cadc == 0 || strcmp(cadc, "+CADC") != 0)
        return SIM800L_ERROR;
    if (ok == 0 || strcmp(ok, "OK") != 0)
        return SIM800L_ERROR;

    if (status)
        *status = atoi(arg1);
    if (value)
        *value = atoi(arg2);

    return SIM800L_OK;
}