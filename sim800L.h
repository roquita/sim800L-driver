#pragma once

#define SIM800L_DEBUG

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        SIM800L_OK,
        SIM800L_FAIL,
        SIM800L_TIMEOUT,
    } sim800L_err_t;

    typedef sim800L_err_t (*sim800L_send_cmd_t)(char *cmd, char *response, int timeout);
    typedef sim800L_err_t (*sim800L_enable_t)(void);
    typedef sim800L_err_t (*sim800L_disable_t)(void);
    typedef sim800L_err_t (*sim800L_reset_t)(void);
    typedef sim800L_err_t (*sim800L_turn_on_t)(void);
    typedef sim800L_err_t (*sim800L_turn_off_t)(void);

    typedef struct
    {
        sim800L_send_cmd_t send_cmd;
        sim800L_enable_t enable;
        sim800L_disable_t disable;
        sim800L_reset_t reset;
        sim800L_turn_on_t turn_on;
        sim800L_turn_off_t turn_off;
    } sim800L_t;

    sim800L_err_t sim800L_init(sim800L_t *sim800L);
    sim800L_err_t sim800_link_net(sim800L_t *sim800L);
    sim800L_err_t sim800L_unlink_net(sim800L_t *sim800L);
    sim800L_err_t sim800L_tcp_connect(sim800L_t *sim800L, char *ip, int port, bool secure);
    sim800L_err_t sim800L_tcp_send(sim800L_t *sim800L, char *data, int len);


#ifdef __cplusplus
}
#endif