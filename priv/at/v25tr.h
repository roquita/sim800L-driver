#pragma once

#include "../macros.h"

static inline sim800L_err_t SIM800L_AT(sim800L_t *sim800L)
{
    char response[10] = {0};
    char at_cmd[10];

    // MAKE AT COMMAND
    sprintf(at_cmd, "AT");

    // SEND AND RECEIVE ANSWER
    sim800L_err_t res = SIM800L_SEND_AT_CMD(sim800L, at_cmd, response, sizeof(response) / sizeof(char), 1, 1000);
    if (res != SIM800L_OK)
        return res;

    // PARSE ANSWER
    if (strcmp(response, "\r\nOK\r\n") != 0)
        return SIM800L_ERROR;

    return SIM800L_OK;
}
static inline sim800L_err_t SIM800L_ATE(sim800L_t *sim800L, bool echo)
{
    char response[10] = {0};
    char at_cmd[10];

    // MAKE AT COMMAND
    sprintf(at_cmd, "%s", echo ? "ATE1" : "ATE0");

    // SEND AND RECEIVE ANSWER
    sim800L_err_t res = SIM800L_SEND_AT_CMD(sim800L, at_cmd, response, sizeof(response) / sizeof(char), 1, 1000);
    if (res != SIM800L_OK)
        return res;

    // PARSE ANSWER
    if (strcmp(response, "\r\nOK\r\n") != 0)
        return SIM800L_ERROR;

    return SIM800L_OK;
}