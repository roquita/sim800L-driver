#pragma once

#include "../macros.h"

static inline sim800L_err_t SIM800L_CGNSPWR_WRITE(sim800L_t *sim800L, int mode)
{
    char response[20] = {0};
    char at_cmd[20] = {0};

    // MAKE AT COMMAND
    sprintf(at_cmd, "AT+CGNSPWR=%i", mode);

    // SEND AND RECEIVE ANSWER
    sim800L_err_t res;
    res = SIM800L_SEND_AT_CMD(sim800L, at_cmd, response, sizeof(response) / sizeof(char), 1, 5000);

    if (res != SIM800L_OK)
        return res;

    // PARSE ANSWER
    char *ok = strtok(response, "\r\n");

    if (ok == 0 || (strcmp(ok, "OK") != 0))
        return SIM800L_ERROR;

    return SIM800L_OK;
}
static inline sim800L_err_t SIM800L_CGNSPWR_READ(sim800L_t *sim800L, int *mode)
{
    char response[50] = {0};
    char at_cmd[20] = {0};

    // MAKE AT COMMAND
    sprintf(at_cmd, "AT+CGNSPWR?");

    // SEND AND RECEIVE ANSWER
    sim800L_err_t res;
    res = SIM800L_SEND_AT_CMD(sim800L, at_cmd, response, sizeof(response) / sizeof(char), 2, 1000);

    if (res != SIM800L_OK)
        return res;

    // PARSE ANSWER
    char *cgnspwr = strtok(response, " \r\n");
    char *arg1 = strtok(NULL, " \r\n");
    char *ok = strtok(NULL, " \r\n");

    if (cgnspwr == 0 || (strcmp(cgnspwr, "+CGNSPWR:") != 0))
        return SIM800L_ERROR;

    if (ok == 0 || (strcmp(ok, "OK") != 0))
        return SIM800L_ERROR;

    *mode = atoi(arg1);

    return SIM800L_OK;
}
static inline sim800L_err_t SIM800L_CGNSINF_EXE(sim800L_t *sim800L)
{
    char response[200] = {0};
    char at_cmd[20] = {0};

    // MAKE AT COMMAND
    sprintf(at_cmd, "AT+CGNSINF");

    // SEND AND RECEIVE ANSWER
    sim800L_err_t res;
    res = SIM800L_SEND_AT_CMD(sim800L, at_cmd, response, sizeof(response) / sizeof(char), 2, 2000);

    if (res != SIM800L_OK)
        return res;

    // PARSE ANSWER
    char *cgnsinf = strtok(response, "\r\n");
    char *ok = strtok(NULL, "\r\n");

    if (ok == 0 || (strcmp(ok, "OK") != 0))
        return SIM800L_ERROR;

    return SIM800L_OK;
}
static inline sim800L_err_t SIM800L_CGNSTST_WRITE(sim800L_t *sim800L, int mode)
{
    char response[20] = {0};
    char at_cmd[20] = {0};

    // MAKE AT COMMAND
    sprintf(at_cmd, "AT+CGNSTST=%i", mode);

    // SEND AND RECEIVE ANSWER
    sim800L_err_t res;
    res = SIM800L_SEND_AT_CMD(sim800L, at_cmd, response, sizeof(response) / sizeof(char), 1, 2000);

    if (res != SIM800L_OK)
        return res;

    // PARSE ANSWER
    char *ok = strtok(response, "\r\n");

    if (ok == 0 || (strcmp(ok, "OK") != 0))
        return SIM800L_ERROR;

    return SIM800L_OK;
}