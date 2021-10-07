#pragma once

#include "../macros.h"

static inline sim800L_err_t SIM800L_CGATT(sim800L_t *sim800L, int state)
{
    char response[30] = {0};
    char at_cmd[15] = {0};

    // MAKE AT COMMAND
    sprintf(at_cmd, "AT+CGATT=%i", state);

    // SEND AND RECEIVE ANSWER
    sim800L_err_t res;
    res = SIM800L_SEND_AT_CMD(sim800L, at_cmd, response, sizeof(response) / sizeof(char), 1, 10000);

    if (res != SIM800L_OK)
        return res;

    // PARSE ANSWER
    char *ok = strtok(response, "\r\n");
    if (ok == 0 || strcmp(ok, "OK") != 0)
        return SIM800L_ERROR;

    return SIM800L_OK;
}
