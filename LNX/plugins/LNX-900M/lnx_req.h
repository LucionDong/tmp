/*
 * lnx_req.h
 * Copyright (C) 2023 Lucion <dongbin0625@126.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef LNX_REQ_H
#define LNX_REQ_H

#include <neuron/neuron.h>

#include "lnx_stack.h"

typedef struct neu_plugin {
    neu_plugin_common_t common;
    neu_conn_t *conn;
    LnxStack *stack;

    void *pluginGroupData;
    // bool isUdp;
    char *dstIP;

    uint16_t interval;
} neu_plugin, LnxPlugin;

void lnx_conn_connected(void *data, int fd);
void lnx_conn_disconnected(void *data, int fd);

int lnx_send_msg(void *ctx, char *msg, int msgLen);
int lnx_value_handle(void *ctx, char *ipAddr, char *msg, int msgNum);
int lnx_group_timer(neu_plugin_t *plugin, neu_plugin_group_t *group, uint16_t maxType);

static int process_protocol_buf(neu_plugin_t *plugin, char *ipAddr, uint16_t responseSize);

int lnx_write(LnxPlugin *plugin, neu_datatag_t *tag, neu_value_u value, bool response);
int lnx_write_tag(LnxPlugin *plugin, neu_datatag_t *tag, neu_value_u value);

#endif /* !LNX_REQ_H */
