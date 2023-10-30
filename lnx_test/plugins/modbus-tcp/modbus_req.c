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
#include <time.h>

#include "modbus_point.h"
#include "modbus_req.h"
#include "modbus_stack.h"

typedef struct LnxGroupData_t {
    UT_array *tags;
    char *group;

    LnxReadCmdSet *cmd;
} LnxGroupData;

static void plugin_group_free(neu_plugin_group_t *pgp);
static int process_protocol_buf(neu_plugin_t *plugin, uint8_t slave_id, uint16_t response_size);

void lnx_conn_connected(void *data, int fd) {
    printf("modbus_conn_connected\n");
    LnxPlugin *plugin = (LnxPlugin *) data;

    (void) fd;

    plugin->common.link_state = NEU_NODE_LINK_STATE_CONNECTED;
}

void lnx_conn_disconnected(void *data, int fd) {
    printf("modubs_conn_disconnected\n");
    LnxPlugin *plugin = (LnxPlugin *) data;
    // struct neu_plugin *plugin = (struct neu_plugin *) data;
    (void) fd;

    plugin->common.link_state = NEU_NODE_LINK_STATE_DISCONNECTED;
}

int lnx_send_msg(void *ctx, char *msg, int msgLen) {
    printf("modbus_send_msg\n");
    neu_plugin_t *plugin = (neu_plugin_t *) ctx;
    int ret = 0;

    plog_send_protocol(plugin, msg, msgLen);

    ret = neu_conn_udp_sendto(plugin->conn, (uint8_t *) msg, msgLen, (char *) plugin->dstIP);

    return ret;
}

int lnx_group_timer(neu_plugin_t *plugin, neu_plugin_group_t *group, uint16_t max_byte) {
    printf("lnx_group_timer\n");
    neu_conn_state_t state = {0};
    neu_adapter_update_metric_cb_t updateMetric = plugin->common.adapter_callbacks->update_metric;
    struct LnxGroupData_t *gd = NULL;
    int64_t rtt = NEU_METRIC_LAST_RTT_MS_MAX;

    if (group->user_data == NULL) {
        gd = calloc(1, sizeof(LnxGroupData));

        group->user_data = gd;
        group->group_free = plugin_group_free;
        utarray_new(gd->tags, &ut_ptr_icd);

        utarray_foreach(group->tags, neu_datatag_t *, tag) {
            LnxPoint *p = calloc(1, sizeof(LnxPoint));
            int ret = lnx_tag_to_point(tag, p);
            assert(ret == 0);

            utarray_push_back(gd->tags, &p);
        }

        gd->group = strdup(group->group_name);
        gd->cmd = lnx_tag_sort(gd->tags, maxType);
    }

    gd = group->user_data;
    plugin->pluginGroupData = gd;

    for (uint16_t i = 0; i < gd->cmd->cmdNum; i++) {
        uint16_t responseSize = 0;
        uint16_t readTms = neu_time_ms();
        int bufRet = 0;
        int stackReadRet = lnx_stack_read(plugin->stack, gd->cmd->cmd[i].ipAddr, gd - > cmd->cmd[i].command,
                                          gd->cmd->cmd[i].msg, strlen(gd->cmd->cmd[i].msg), &responseSize);
        if (stackReadRet > 0) {
            bufRet = process_protocol_buf(plugin, gd->cmd->cmd[i].ipAddr, responseSi ze);
            if (bufRet > 0) {
                rtt = neu_time_ms() - readTms;

            } else if (bufRet < 0) {
                rtt = neu_time_ms() - readTms;
            }
        }
        if (plugin->interval > 0) {
            struct timespec t1 = {.tv_sec = plugin->interval / 1000,
                                  .tv_nsec = 1000 * 1000 * (plugin->interval % 1000)};
            struct timespec t2 = {0};
            nanosleep(&t1, &t2);
        }
    }

    state = neu_conn_state(plugin->conn);
    updateMetric(plugin->common.adapter, NEU_METRIC_SEND_BYTES, state.send_bytes, NULL);
    updateMetric(plugin->common.adapter, NEU_METRIC_RECV_BYTES, state.recv_bytes, NULL);
    updateMetric(plugin->common.adapter, NEU_METRIC_LAST_RTT_MS, rtt, NULL);
    updateMetric(plugin->common.adapter, NEU_METRIC_GROUP_LAST_SEND_MSGS, gd->cmd->cmdNum, group->group_name);
    return 0;
}

int lnx_write(LnxPlugin *plugin, neu_datatag_t *tag, neu_value_u value, bool response) {
    printf("lnx_write\n");
    LnxPoint point = {0};
    int ret = lnx_tag_to_point(tag, &point);
    assert(ret == 0);
    uint8_t nByte = 0;

    switch (tag->type) {
        case NEU_TYPE_STRING: {
            switch (point.option.string.type) {
                case NEU_DATATAG_STRING_TYPE_H:
                    break;
                case NEU_DATATAG_STRING_TYPE_L:
                    neu_datatag_string_ltoh(value.str, point.option.string.length);
                    break;
                case NEU_DATATAG_STRING_TYPE_D:
                    break;
                case NEU_DATATAG_STRING_TYPE_E:
                    break;
            }
            nByte = point.option.string.length;
            break;
        }
        default:
            assert(false);
            break;
    }

    uint16_t responseSize = 0;
    ret = lnx_stack_write(plugin->stack, point.ipAddr, point.command, point.msg, strlen(point.msg));
    if (ret > 0) {
        process_protocol_buf(plugin, point.ipAddr, responseSize);
    }

    return ret;
}

static int process_protocol_buf(neu_plugin_t *plugin, char *ipAddr, uint16_t responseSize) {
    printf("process_protocol_buf\n");
    uint8_t *recvBuf = calloc(responseSize, 1);
    neu_protocol_unpack_buf_t pbuf = {0};
    ssize_t ret = 0;

    ret = neu_conn_udp_recvfrom(plugin->conn, recvBuf,
                                sizeof(struct Header_t) + sizeof(struct Command_t) + sizeof(struct Data_t),
                                plugin->dstIP);
    if (ret == 0 || ret == -1) {
        free(recvBuf);
        return 0;
    }

    if (ret > 0) {
        neu_protocol_unpack_buf_init(&pbuf, recvBuf, ret);
        int stackRet = lnx_stack_recv(plugin->stack, &pbuf, ipAddr);
    }

    free(recvBuf);
    return ret;
}

int lnx_write_tag(LnxPlugin *plugin, neu_datatag_t *tag, neu_value_u value) {
    printf("lnx_write_tag\n");
    int ret = lnx_write(plugin, tag, value, true);
    return ret;
}

int lnx_value_handle(void *ctx, char *ipAddr, char *msg, int msgNum) {
    printf("lnx_value_handle\n");
    if (msgNum == 0) {
        printf("lnx_value_handle(msg is error)\n");
        return 0;
    }
    return 0;
}
