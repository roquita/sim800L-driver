#pragma once

#include "../macros.h"

static inline sim800L_err_t SIM800L_CIPGSMLOC_WRITE(sim800L_t *sim800L, int type, int cid, float *lon, float *lat)
{
    char response[80] = {0};
    char at_cmd[20] = {0};

    // MAKE AT COMMAND
    sprintf(at_cmd, "AT+CIPGSMLOC=%i,%i", type, cid);

    // SEND AND RECEIVE ANSWER
    sim800L_err_t res;
    res = SIM800L_SEND_AT_CMD(sim800L, at_cmd, response, sizeof(response) / sizeof(char), 2, 3000);

    if (res != SIM800L_OK)
        return res;

    // PARSE ANSWER
    char *cipgsmloc = strtok(response, " ,\r\n");
    char *arg1 = strtok(NULL, " ,\r\n");
    char *arg2 = strtok(NULL, " ,\r\n");
    char *arg3 = strtok(NULL, " ,\r\n");
    char *arg4 = strtok(NULL, " ,\r\n"); //2021/11/24
    char *arg5 = strtok(NULL, " ,\r\n"); //22:22:19
    char *ok = strtok(NULL, " ,\r\n");

    if (ok == 0 || (strcmp(ok, "OK") != 0))
        return SIM800L_ERROR;

    if (cipgsmloc == 0 || (strcmp(cipgsmloc, "+CIPGSMLOC:") != 0))
        return SIM800L_ERROR;

    int loccode = atoi(arg1);
    if (loccode != 0)
        return SIM800L_ERROR;

    *lon = atof(arg2);
    *lat = atof(arg3);

    return SIM800L_OK;
}