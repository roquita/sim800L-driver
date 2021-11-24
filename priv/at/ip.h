#pragma once

#include "../macros.h"

typedef struct
{
    int cmd_type;
    int cid;
    char connParamTag[20];
    char connParamValue[20];
} sapbr_input_t;

typedef union
{
    struct
    {
        int cid;
        int status;
        char ip_addr[20];
    };
    struct
    {
        char connParamTag[20];
        char connParamValue[20];
    };
} sapbr_output_t;

static inline sim800L_err_t SIM800L_SAPBR(sim800L_t *sim800L, sapbr_input_t *input, sapbr_output_t *output)
{
    int cmd_type = input->cmd_type;
    assert(cmd_type >= 0 && cmd_type <= 4);

    char response[100] = {0};
    char at_cmd[100] = {0};

    // MAKE AT COMMAND
    if (cmd_type == 0 || cmd_type == 1 || cmd_type == 2 || cmd_type == 4)
        sprintf(at_cmd, "AT+SAPBR=%i,%i", cmd_type, input->cid);
    else if (cmd_type == 3)
        sprintf(at_cmd, "AT+SAPBR=%i,%i,\"%s\",\"%s\"", cmd_type, input->cid, input->connParamTag, input->connParamValue);
    else
        return SIM800L_INVALID_ARG;

    // SEND AND RECEIVE ANSWER
    sim800L_err_t res;
    if (cmd_type == 0 || cmd_type == 1 || cmd_type == 3)
        res = SIM800L_SEND_AT_CMD(sim800L, at_cmd, response, sizeof(response) / sizeof(char), 1, 10000);
    else if (cmd_type == 2)
        res = SIM800L_SEND_AT_CMD(sim800L, at_cmd, response, sizeof(response) / sizeof(char), 2, 10000);
    else if (cmd_type == 4)
        res = SIM800L_SEND_AT_CMD(sim800L, at_cmd, response, sizeof(response) / sizeof(char), 3, 10000);
    else
        return SIM800L_INVALID_ARG;

    if (res != SIM800L_OK)
        return res;

    // PARSE ANSWER
    if (cmd_type == 0 || cmd_type == 1 || cmd_type == 3)
    {
        char *ok = strtok(response, "\r\n");
        if (ok == 0 || strcmp(ok, "OK") != 0)
            return SIM800L_ERROR;
    }
    else if (cmd_type == 2)
    {
        char *sapbr = strtok(response, "\r\n:,");
        char *cid = strtok(NULL, "\r\n:,");
        char *status = strtok(NULL, "\r\n:,");
        char *ip_addr = strtok(NULL, "\r\n:,");
        char *ok = strtok(NULL, "\r\n:,");

        if (sapbr == 0 || strcmp(sapbr, "+SAPBR") != 0)
            return SIM800L_ERROR;
        if (ok == 0 || strcmp(ok, "OK") != 0)
            return SIM800L_ERROR;

        if (output)
        {
            output->cid = atoi(cid);
            output->status = atoi(status);
            snprintf(output->ip_addr, 20, "%s", ip_addr);
        }
    }
    else if (cmd_type == 4)
    {
        char *sapbr = strtok(response, "\r\n:,");
        char *connParamTag = strtok(NULL, "\r\n:,");
        char *connParamValue = strtok(NULL, "\r\n:,");
        char *ok = strtok(NULL, "\r\n:,");

        if (sapbr == 0 || strcmp(sapbr, "+SAPBR") != 0)
            return SIM800L_ERROR;
        if (ok == 0 || strcmp(ok, "OK") != 0)
            return SIM800L_ERROR;

        if (output)
        {
            snprintf(output->connParamTag, 20, "%s", connParamTag);
            snprintf(output->connParamValue, 20, "%s", connParamValue);
        }
    }
    else
        return SIM800L_INVALID_ARG;

    return SIM800L_OK;
}
