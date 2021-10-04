#include <stdio.h>
#include "sim800L.h"
#include "macros.h"
#include "at_cmd.h"

sim800L_err_t sim800L_init(sim800L_t *sim800L)
{

    printf("hi from sim800L library\n");
    SIM800L_TURN_ON(sim800L);
    SIM800L_ENABLE(sim800L);

    sim800L->delay_ms(5000);

    SIM800L_AT(sim800L);
    // disable echo (ATE)

    return SIM800L_OK;
}