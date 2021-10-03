#include <stdio.h>
#include "sim800L.h"

sim800L_err_t sim800L_init(sim800L_t *sim800L)
{
    
    printf("hi from sim800L library\n");

    return SIM800L_OK;
}