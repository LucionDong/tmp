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
#include <neuron.h>
#include <stdlib.h>

#include "errcodes.h"
#include "modbus_point.h"
#include "modbus_req.h"
#include "modbus_stack.h"

static neu_plugin_t *driver_open(void);

static int driver_close(neu_plugin_t *plugin);
static int driver_init(neu_plugin_t *plugin, bool load);
static int driver_uninit(neu_plugin_t *plugin);
static int driver_start(neu_plugin_t *plugin);
static int driver_stop(neu_plugin_t *plugin);
static int driver_config(neu_plugin_t *plugin, const char *config);
static int driver_request(neu_plugin_t *plugin, neu_reqresp_head_t *head, void *data);

static int driver_tag_validator(const neu_datatag_t *tag);
static int driver_validate_tag(neu_plugin_t *plugin, neu_datatag_t *tag);
static int driver_group_timer(neu_plugin_t *plugin, neu_plugin_group_t *group);

static const neu_plugin_intf_funs_t plugin_intf_funs = {
    .open = driver_open,
    .close = driver_close,
    .init = driver_init,
    .uninit = driver_uninit,
    .start = driver_start,
    .stop = driver_stop,
    .setting = driver_config,
    .request = driver_request,

    .driver.validate_tag = driver_validate_tag,
    .driver.group_timer = driver_group_timer,
    .driver.write_tag = NULL,
    // .driver.write_tag     = driver_write,
    .driver.tag_validator = driver_tag_validator,
    // .driver.write_tags    = NULL,
    // .driver.write_tags    = driver_write_tags,
    .driver.add_tags = NULL,
    .driver.load_tags = NULL,
    .driver.del_tags = NULL,
};

const neu_plugin_module_t neu_plugin_module = {
    .version = NEURON_PLUGIN_VER_1_0,
    .schema = "lnx900",
    .module_name = "lnx900",
    .module_descr =
        "This plugin is used to connect devices using the modbus TCP protocol "
        "Users can choose to connect as a client or a server. Support TCP and "
        "UDP communication methods.",
    .module_descr_zh =
        "该插件用于连接使用 modbus tcp 协议的设备。"
        "用户可选择作为客户端连接，或是服务端连接,支持 TCP 和 UDP 通信方式。",
    .intf_funs = &plugin_intf_funs,
    .kind = NEU_PLUGIN_KIND_SYSTEM,
    .type = NEU_NA_TYPE_DRIVER,
    .display = true,
    .single = false,
};

static neu_plugin_t *driver_open(void) {
    printf("driver_open\n");
    neu_plugin_t *plugin = calloc(1, sizeof(neu_plugin_t));

    neu_plugin_common_init(&plugin->common);

    return plugin;
}

static int driver_close(neu_plugin_t *plugin) {
    printf("driver_close\n");
    free(plugin);

    return 0;
}

static int driver_init(neu_plugin_t *plugin, bool load) {
    printf("driver_init\n");
    (void) load;
    plugin->events = neu_event_new();
    plugin->stack = lnx_stack_create((void *) plugin, REQ, lnx_send_msg, lnx_value_handle);

    plog_notice(plugin, "%s init success", plugin->common.name);
    return 0;
}

static int driver_uninit(neu_plugin_t *plugin) {
    printf("driver_unini\n");
    plog_notice(plugin, "%s uninit start", plugin->common.name);
    if (plugin->conn != NULL) {
        neu_conn_destory(plugin->conn);
    }

    if (plugin->stack) {
        lnx_stack_destroy(plugin->stack);
    }

    neu_event_close(plugin->events);

    plog_notice(plugin, "%s uninit success", plugin->common.name);

    return 0;
}

static int driver_start(neu_plugin_t *plugin) {
    printf("driver_start\n");
    neu_conn_start(plugin->conn);
    plog_notice(plugin, "%s start success", plugin->common.name);
    return 0;
}

static int driver_stop(neu_plugin_t *plugin) {
    printf("driver_stop\n");
    neu_conn_stop(plugin->conn);
    plog_notice(plugin, "%s stop success", plugin->common.name);
    return 0;
}

static int driver_config(neu_plugin_t *plugin, const char *config) {
    printf("driver_config\n");
    int ret = 0;
    char *err_param = NULL;
    neu_json_elem_t port = {.name = "port", .t = NEU_JSON_INT};
    neu_json_elem_t timeout = {.name = "timeout", .t = NEU_JSON_INT};
    neu_json_elem_t host = {.name = "host", .t = NEU_JSON_STR, .v.val_str = NULL};
    neu_json_elem_t dstHost = {.name = "dstHost", .t = NEU_JSON_STR, .v.val_str = NULL};
    neu_json_elem_t dstPort = {.name = "dstPort", .t = NEU_JSON_INT};
    neu_json_elem_t interval = {.name = "interval", .t = NEU_JSON_INT};
    neu_json_elem_t mode = {.name = "connection_mode", .t = NEU_JSON_INT};
    neu_conn_param_t param = {0};

    ret = neu_parse_param((char *) config, &err_param, 4, &port, &timeout, &host, &interval);

    if (ret != 0) {
        plog_error(plugin, "config: %s, decode error: %s", config, err_param);
        free(err_param);
        if (host.v.val_str != NULL) {
            free(host.v.val_str);
        }
        if (dstHost.v.val_str != NULL) {
            free(dstHost.v.val_str);
        }
        return -1;
    }

    if (timeout.v.val_int <= 0) {
        plog_error(plugin, "config: %s, set timeout error: %s", config, err_param);
        free(err_param);
        return -1;
    }

    param.log = plugin->common.log;
    plugin->interval = interval.v.val_int;

    if (mode.v.val_int == 0) {
        param.type = NEU_CONN_UDP_TO;
        param.params.udpto.src_ip = host.v.val_str;
        param.params.udpto.src_port = host.v.val_int;
        param.params.udpto.timeout = timeout.v.val_int;
        plog_notice(plugin, "config: host %s, port: %" PRId64 ", mode: %" PRId64 "", host.v.val_str, port.v.val_int,
                    mode.v.val_int);
    }
    if (mode.v.val_int == 1) {
        param.type = NEU_CONN_UDP;
        param.params.udp.dst_ip = dstHost.v.val_str;
        param.params.udp.dst_port = dstPort.v.val_int;
        param.params.udp.src_ip = host.v.val_str;
        param.params.udp.src_port = host.v.val_int;
        param.params.udp.timeout = timeout.v.val_int;
        plog_notice(plugin, "config: host %s, port: %" PRId64 ", mode: %" PRId64 ",dstIp: %s, dstPort: %" PRId64 "",
                    host.v.val_str, port.v.val_int, mode.v.val_int, dstHost.v.val_str, dstPort.v.val_int);
    }

    if (plugin->conn != NULL) {
        plugin->conn = neu_conn_reconfig(plugin->conn, &param);
    } else {
        plugin->common.link_state = NEU_NODE_LINK_STATE_DISCONNECTED;
        plugin->conn = neu_conn_new(&param, (void *) plugin, lnx_conn_connected, lnx_conn_disconnected);
    }

    free(host.v.val_str);
    free(dstHost.v.val_str);
    return 0;
}

static int driver_request(neu_plugin_t *plugin, neu_reqresp_head_t *head, void *data) {
    printf("driver_request\n");
    (void) plugin;
    (void) head;
    (void) data;
    return 0;
}

static int driver_tag_validator(const neu_datatag_t *tag) {
    printf("driver_tag_validator\n");
    LnxPoint point = {0};
    return lnx_tag_to_point(tag, &point);
}

static int driver_validate_tag(neu_plugin_t *plugin, neu_datatag_t *tag) {
    printf("driver_validate_tag\n");
    LnxPoint point = {0};

    int ret = lnx_tag_to_point(tag, &point);
    if (ret == 0) {
        plog_notice(plugin, "validate tag success,name: %s, address: %s, type: %d, i  pAddr: %s, command: %s, msg: %s",
                    tag->name, tag->address, tag->type, point.ipAddr, point.command, point.msg);

    } else {
        plog_error(plugin, "validate tag success,name: %s, address: %s, type: %d, ip  Addr: %s, command: %s, msg: %s",
                   tag->name, tag->address, tag->type, point.ipAddr, point.command, point.msg);
    }
    return ret;
}

static int driver_group_timer(neu_plugin_t *plugin, neu_plugin_group_t *group) {
    printf("driver_group_timer\n");
    return lnx_group_timer(plugin, group, 0xfa);
}

// static int driver_write(neu_plugin_t *plugin, void *req, neu_datatag_t *tag, neu_value_u value) {
//     printf("driver_write\n");
//     return modbus_write_tag(plugin, req, tag, value);
// }
//
// static int driver_write_tags(neu_plugin_t *plugin, void *req, UT_array *tags) {
//     printf("driver_write_tags\n");
//     return modbus_write_tags(plugin, req, tags);
// }
