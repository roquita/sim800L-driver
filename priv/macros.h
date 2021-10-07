#pragma once

#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "../sim800L.h"

#define COLOR_CYAN "\033[0;36m"
#define COLOR_WHITE "\033[0;37m"

#define SIM800L_ENABLE(p_sim800L)            \
    {                                        \
        p_sim800L->pwrkey_gpio_set_level(1); \
        p_sim800L->delay_ms(1);              \
        p_sim800L->pwrkey_gpio_set_level(0); \
        p_sim800L->delay_ms(2000);           \
        p_sim800L->pwrkey_gpio_set_level(1); \
    }
#define SIM800L_DISABLE(p_sim800L)           \
    {                                        \
        p_sim800L->pwrkey_gpio_set_level(1); \
        p_sim800L->delay_ms(1);              \
        p_sim800L->pwrkey_gpio_set_level(0); \
        p_sim800L->delay_ms(1000);           \
        p_sim800L->pwrkey_gpio_set_level(1); \
    }
#define SIM800L_HW_RESET(p_sim800L)         \
    {                                       \
        p_sim800L->reset_gpio_set_level(1); \
        p_sim800L->delay_ms(1);             \
        p_sim800L->reset_gpio_set_level(0); \
        p_sim800L->delay_ms(105);           \
        p_sim800L->reset_gpio_set_level(1); \
    }

static inline sim800L_err_t SIM800L_WAIT_FOR_BYTES(sim800L_t *sim800L, char *tofind, char *response, int max_len, int32_t timeout)
{
    char prev_byte = 0;
    int response_index = 0;
    int64_t start_ms = sim800L->get_time_ms();
    while (sim800L->get_time_ms() - start_ms < (int64_t)timeout)
    {
        while (sim800L->available() > 0)
        {
            char byte = 0;
            sim800L_err_t res = sim800L->read_byte(&byte);
            if (res != SIM800L_OK)
            {
                return SIM800L_BUS_FAULT;
            }
            response[response_index] = byte;
            response_index++;
            max_len--;

#ifdef SIM800L_DEBUG
            if (byte == '\n')
                printf("<LF>\n");
            else if (byte == '\r')
                printf("<CR>");
            else
                printf("%c", byte);
#endif

            if (byte == '\n' && prev_byte == '\r')
            {
                char *p = strstr(response, tofind);
                if (p != 0)
                {
                    *p = 0; // remove "tofind" from torcv
                    return SIM800L_OK;
                }
            }
            prev_byte = byte;

            if (max_len <= 0)
            {
                return SIM800L_OVERFLOW;
            }
        }

        sim800L->delay_ms(1);
    }
    return SIM800L_TIMEOUT;
}
static inline sim800L_err_t SIM800L_WAIT_FOR_RESPONSE(sim800L_t *sim800L, char *response, int max_len, int lines, int32_t timeout)
{
    char final_char = '\n';
    if (lines == -1)
    {
        final_char = '>';
        lines = 1;
    }
    else
    {
        lines *= 2; // double \r\n per line
    }

    int response_index = 0;
    int64_t start_ms = sim800L->get_time_ms();
    while (sim800L->get_time_ms() - start_ms < (int64_t)timeout)
    {
        while (sim800L->available() > 0)
        {

            static char prev_byte = 0;
            char byte = 0;
            sim800L_err_t res = sim800L->read_byte(&byte);
            if (res != SIM800L_OK)
            {
                return SIM800L_BUS_FAULT;
            }
            response[response_index] = byte;
            response_index++;
            max_len--;

#ifdef SIM800L_DEBUG
            if (byte == '\n')
                printf("<LF>");
            else if (byte == '\r')
                printf("<CR>");
            else
                printf("%c", byte);
#endif

            if (byte == '\n' && prev_byte == '\r')
            {
                lines--;
            }
            prev_byte = byte;

            if (lines <= 0 && max_len >= 1 && byte == final_char)
            {
                response[response_index] = 0;
                return SIM800L_OK;
            }
            if (max_len <= 0)
            {
                return SIM800L_OVERFLOW;
            }
        }

        sim800L->delay_ms(1);
    }
    return SIM800L_TIMEOUT;
}

static inline sim800L_err_t SIM800L_SEND_AT_CMD(sim800L_t *sim800L, char *cmd, char *response,
                                                int max_len, int lines, int32_t timeout)
{
    sim800L->flush();

#ifdef SIM800L_DEBUG
    printf("\"%s\" ---> ", cmd);
#endif

    sim800L_err_t res = sim800L->send_string(cmd);
    res = sim800L->send_string("\r\n");
    if (res != SIM800L_OK)
    {
#ifdef SIM800L_DEBUG
        printf("\n        ---> send_string failed\n");
#endif
        return res;
    }

    res = SIM800L_WAIT_FOR_RESPONSE(sim800L, response, max_len, lines, timeout);

#ifdef SIM800L_DEBUG

    printf("\n");
    if (res != SIM800L_OK)
    {
        printf("        <--- error: %u \n", res);
    } /*
    else
    {
        //printf("        <--- \"%s\"\n", response);
    }*/
#endif

    return res;
}
