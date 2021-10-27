#pragma once

#include "../macros.h"

static inline sim800L_err_t SIM800L_CNTPCID_WRITE(sim800L_t *sim800L, int cid)
{
    char response[20] = {0};
    char at_cmd[20] = {0};

    // MAKE AT COMMAND
    sprintf(at_cmd, "AT+CNTPCID=%i", cid);

    // SEND AND RECEIVE ANSWER
    sim800L_err_t res;
    res = SIM800L_SEND_AT_CMD(sim800L, at_cmd, response, sizeof(response) / sizeof(char), 1, 2000);

    if (res != SIM800L_OK)
        return res;

    // PARSE ANSWER
    char *ok = strtok(response, "\r\n");
    if (ok == 0 || strcmp(ok, "OK") != 0)
        return SIM800L_ERROR;

    return SIM800L_OK;
}

static inline sim800L_err_t SIM800L_CNTP_WRITE(sim800L_t *sim800L, char *ntp_ip, int gmt_offset)
{
    char response[20] = {0};
    char at_cmd[50] = {0};

    // MAKE AT COMMAND
    snprintf(at_cmd, 50, "AT+CNTP=\"%s\",%i", ntp_ip, gmt_offset);

    // SEND AND RECEIVE ANSWER
    sim800L_err_t res;
    res = SIM800L_SEND_AT_CMD(sim800L, at_cmd, response, sizeof(response) / sizeof(char), 1, 2000);

    if (res != SIM800L_OK)
        return res;

    // PARSE ANSWER
    char *ok = strtok(response, "\r\n");
    if (ok == 0 || strcmp(ok, "OK") != 0)
        return SIM800L_ERROR;

    return SIM800L_OK;
}

static inline sim800L_err_t SIM800L_CNTP_EXE(sim800L_t *sim800L)
{
    char response[20] = {0};
    char at_cmd[10] = {0};

    // MAKE AT COMMAND
    sprintf(at_cmd, "AT+CNTP");

    // SEND AND RECEIVE ANSWER
    sim800L_err_t res;
    res = SIM800L_SEND_AT_CMD(sim800L, at_cmd, response, sizeof(response) / sizeof(char), 2, 10000);

    if (res != SIM800L_OK)
        return res;

    // PARSE ANSWER
    char *ok = strtok(response, "\r\n: ");
    char *cntp = strtok(NULL, "\r\n: ");
    char *arg1 = strtok(NULL, "\r\n: ");

    if (ok == 0 || strcmp(ok, "OK") != 0)
        return SIM800L_ERROR;
    if (cntp == 0 || strcmp(cntp, "+CNTP") != 0)
        return SIM800L_ERROR;

    int code = atoi(arg1);
    if (code != 1)
        return SIM800L_ERROR;

    return SIM800L_OK;
}