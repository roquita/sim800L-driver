#include <stdio.h>
#include <stdbool.h>
#include "sim800L.h"
#include "macros.h"
#include "at_cmd.h"

sim800L_err_t sim800L_init(sim800L_t *sim800L)
{
    printf("hi from sim800L library\n");
    SIM800L_TURN_ON(sim800L);
    sim800L->delay_ms(1000);
    SIM800L_HW_RESET(sim800L);
    sim800L->delay_ms(3000);

    SIM800L_ATE(sim800L); // hold trash data from sim800L
    sim800L_err_t res_ate = SIM800L_ATE(sim800L);
    sim800L_err_t res_at = SIM800L_AT(sim800L);
    bool res = res_ate == SIM800L_OK && res_at == SIM800L_OK;

    return res ? SIM800L_OK : SIM800L_ERROR;
}