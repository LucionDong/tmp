/**
 * NEURON IIoT System for Industry 4.0
 * Copyright (C) 2020-2022 EMQ Technologies Co., Ltd All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/
#ifndef _NEU_M_PLUGIN_MODBUS_REQ_H_
#define _NEU_M_PLUGIN_MODBUS_REQ_H_

#include <neuron.h>

#include "modbus_stack.h"

typedef struct neu_plugin {
    neu_plugin_common_t common;
    neu_conn_t *conn;
    LnxStack *stack;

    void *pluginGroupData;
    char *dstIP;
    neu_events_t *events;

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

#endif
