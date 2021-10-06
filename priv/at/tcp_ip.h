#pragma once

#include "macros.h"

static inline sim800L_err_t SIM800L_CIPSHUT(sim800L_t *sim800L)
{
    char response[20] = {0};
    char at_cmd[15] = {0};

    // MAKE AT COMMAND
    sprintf(at_cmd, "AT+CIPSHUT");

    // SEND AND RECEIVE ANSWER
    sim800L_err_t res;
    res = SIM800L_SEND_AT_CMD(sim800L, at_cmd, response, sizeof(response) / sizeof(char), 1, 2000);

    if (res != SIM800L_OK)
        return res;

    // PARSE ANSWER
    char *ok = strtok(response, "\r\n");
    if (ok == 0 || strcmp(ok, "SHUT OK") != 0)
        return SIM800L_ERROR;

    return SIM800L_OK;
}

static inline sim800L_err_t SIM800L_CIPMUX(sim800L_t *sim800L, int n)
{
    char response[20] = {0};
    char at_cmd[15] = {0};

    // MAKE AT COMMAND
    sprintf(at_cmd, "AT+CIPMUX=%i", n);

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

static inline sim800L_err_t SIM800L_CSTT(sim800L_t *sim800L, char *apn, char *username, char *password)
{
    char response[30] = {0};
    char at_cmd[100] = {0};

    // MAKE AT COMMAND
    snprintf(at_cmd, sizeof(at_cmd) / sizeof(char), "AT+CSTT=\"%s\",\"%s\",\"%s\"", apn, username, password);

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

static inline sim800L_err_t SIM800L_CIICR(sim800L_t *sim800L)
{
    char response[20] = {0};
    char at_cmd[10] = {0};

    // MAKE AT COMMAND
    sprintf(at_cmd, "AT+CIICR");

    // SEND AND RECEIVE ANSWER
    sim800L_err_t res;
    res = SIM800L_SEND_AT_CMD(sim800L, at_cmd, response, sizeof(response) / sizeof(char), 1, 5000);

    if (res != SIM800L_OK)
        return res;

    // PARSE ANSWER
    char *ok = strtok(response, "\r\n");
    if (ok == 0 || strcmp(ok, "OK") != 0)
        return SIM800L_ERROR;

    return SIM800L_OK;
}

static inline sim800L_err_t SIM800L_CIFSR(sim800L_t *sim800L, char *ip_addr, int len)
{
    char response[20] = {0};
    char at_cmd[10] = {0};

    // MAKE AT COMMAND
    sprintf(at_cmd, "AT+CIFSR");

    // SEND AND RECEIVE ANSWER
    sim800L_err_t res;
    res = SIM800L_SEND_AT_CMD(sim800L, at_cmd, response, sizeof(response) / sizeof(char), 1, 2000);

    if (res != SIM800L_OK)
        return res;

    // PARSE ANSWER
    char *parsed = strtok(response, "\r\n");
    if (parsed == 0 || strcmp(parsed, "ERROR") == 0)
        return SIM800L_ERROR;

    snprintf(ip_addr, len, "%s", parsed);

    return SIM800L_OK;
}

static inline sim800L_err_t SIM800L_CIPSTART(sim800L_t *sim800L, char *mode, char *domain, int port)
{
    char response[30] = {0};
    char at_cmd[100] = {0};

    // MAKE AT COMMAND
    snprintf(at_cmd, sizeof(at_cmd) / sizeof(char),
             "AT+CIPSTART=\"%s\",\"%s\",\"%i\"", mode, domain, port);

    // SEND AND RECEIVE ANSWER
    sim800L_err_t res;
    res = SIM800L_SEND_AT_CMD(sim800L, at_cmd, response, sizeof(response) / sizeof(char), 2, 5000);

    if (res != SIM800L_OK)
        return res;

    // PARSE ANSWER
    char *ok = strtok(response, "\r\n");
    char *result = strtok(NULL, "\r\n");

    if (ok == 0 || strcmp(ok, "OK") != 0)
        return SIM800L_ERROR;
    if (result == 0 || strcmp(result, "CONNECT OK") != 0)
        return SIM800L_ERROR;

    return SIM800L_OK;
}

static inline sim800L_err_t SIM800L_CIPSEND(sim800L_t *sim800L, char *tosend)
{
    char response[15] = {0};
    char at_cmd[15] = {0};

    // MAKE AT COMMAND
    sprintf(at_cmd, "AT+CIPSEND");

    // SEND AND RECEIVE ANSWER
    sim800L_err_t res;
    res = SIM800L_SEND_AT_CMD(sim800L, at_cmd, response, sizeof(response) / sizeof(char), -1, 2000);

    if (res != SIM800L_OK)
        return res;

    // PARSE ANSWER
    char *final_char = strtok(response, "\r\n");

    if (final_char == 0 || strcmp(final_char, ">") != 0)
        return SIM800L_ERROR;

    // SEND BODY
    sim800L->flush();
    sim800L->send_string(tosend);

    // FINISH BODY
    char end[] = {26, 0};
    sim800L->send_string(end);

    // WAIT FOR 2CND ANSWER
    res = SIM800L_WAIT_FOR_RESPONSE(sim800L, response, sizeof(response) / sizeof(char), 1, 2000);
    if (res != SIM800L_OK)    
        return res;    

    // PARSE ANSWER
    char *ok = strtok(response, "\r\n");

    if (ok == 0 || strcmp(ok, "SEND OK") != 0)    
        return SIM800L_ERROR;    

    return SIM800L_OK;
}

static inline sim800L_err_t SIM800L_CIPCLOSE(sim800L_t *sim800L)
{
    char response[20] = {0};
    char at_cmd[15] = {0};

    // MAKE AT COMMAND
    sprintf(at_cmd, "AT+CIPCLOSE");

    // SEND AND RECEIVE ANSWER
    sim800L_err_t res;
    res = SIM800L_SEND_AT_CMD(sim800L, at_cmd, response, sizeof(response) / sizeof(char), 1, 1000);

    if (res != SIM800L_OK)
        return res;

    // PARSE ANSWER
    char *ok = strtok(response, "\r\n");
    if (ok == 0 || strcmp(ok, "CLOSE OK") != 0)
        return SIM800L_ERROR;

    return SIM800L_OK;
}