/*
 *
 * Copyright (C) 2023-10-24 10:47 db <db@126.com>
 *
 */
// #include <neuron/connection/neu_connection.h>
// #include <neuron/define.h>
#include <neuron/neuron.h>
// #include <neuron/plugin.h>
// #include <stdbool.h>
#include <stdlib.h>
// #include <time.h>

#include "lnx_point.h"
#include "lnx_req.h"
#include "lnx_stack.h"

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

    //     .open = NULL,
    // .close = NULL,
    // .init = NULL,
    // .uninit = NULL,
    // .start = NULL,
    // .stop = NULL,
    // .setting = NULL,
    //     .request = NULL,

    .driver.validate_tag = driver_validate_tag,
    .driver.group_timer = driver_group_timer,
    .driver.write_tag = NULL,
    .driver.tag_validator = driver_tag_validator,
    .driver.add_tags = NULL,
    .driver.load_tags = NULL,
    .driver.del_tags = NULL,

    //  .driver.validate_tag = NULL,
    // .driver.group_timer = NULL,
    // .driver.write_tag = NULL,
    // .driver.tag_validator = NULL,
    // .driver.add_tags = NULL,
    // .driver.load_tags = NULL,
    //  .driver.del_tags = NULL,

};

const neu_plugin_module_t neu_plugin_module = {
    .version = NEURON_PLUGIN_VER_1_0,
    .schema = "LNX900",
    .module_name = "LNX900",
    .module_descr = "",
    .intf_funs = &plugin_intf_funs,
    .kind = NEU_PLUGIN_KIND_SYSTEM,
    .type = NEU_NA_TYPE_DRIVER,
    .display = true,
    .single = false,
};

static neu_plugin_t *driver_open(void) {
    neu_plugin_t *plugin = calloc(1, sizeof(neu_plugin_t));

    neu_plugin_common_init(&plugin->common);

    return plugin;
}

static int driver_close(neu_plugin_t *plugin) {
    free(plugin);
    return 0;
}

static int driver_init(neu_plugin_t *plugin, bool load) {
    (void) load;
    plugin->stack = lnx_stack_create((void *) plugin, REQ, lnx_send_msg, lnx_value_handle);

    plog_notice(plugin, "%s init success", plugin->common.name);
    return 0;
}

static int driver_uninit(neu_plugin_t *plugin) {
    plog_notice(plugin, "%s uninit start", plugin->common.name);
    if (plugin->conn != NULL) {
        neu_conn_destory(plugin->conn);
    }

    if (plugin->stack) {
        lnx_stack_destroy(plugin->stack);
    }

    plog_notice(plugin, "%s uninit success", plugin->common.name);
    return 0;
}

static int driver_start(neu_plugin_t *plugin) {
    neu_conn_start(plugin->conn);
    plog_notice(plugin, "%s start success", plugin->common.name);
    return 0;
}

static int driver_stop(neu_plugin_t *plugin) {
    neu_conn_stop(plugin->conn);
    plog_notice(plugin, "%s stop success", plugin->common.name);
    return 0;
}

static int driver_config(neu_plugin_t *plugin, const char *config) {
    int ret = 0;
    char *errParam = NULL;
    neu_json_elem_t port = {.name = "port", .t = NEU_JSON_INT};
    neu_json_elem_t timeout = {.name = "timeout", .t = NEU_JSON_INT};
    neu_json_elem_t host = {.name = "host", .t = NEU_JSON_STR, .v.val_str = NULL};
    neu_json_elem_t dstHost = {.name = "dstHost", .t = NEU_JSON_STR, .v.val_str = NULL};
    neu_json_elem_t dstPort = {.name = "dstPort", .t = NEU_JSON_INT};
    neu_json_elem_t interval = {.name = "interval", .t = NEU_JSON_INT};
    neu_json_elem_t mode = {.name = "connection_mode", .t = NEU_JSON_INT};
    neu_conn_param_t param = {0};

    ret = neu_parse_param((char *) config, &errParam, 4, &port, &timeout, &host, &interval);

    if (ret != 0) {
        plog_error(plugin, "config: %s, decode error: %s", config, errParam);
        free(errParam);
        if (host.v.val_str != NULL) {
            free(host.v.val_str);
        }
        if (dstHost.v.val_str != NULL) {
            free(dstHost.v.val_str);
        }
        return -1;
    }

    if (timeout.v.val_int <= 0) {
        plog_error(plugin, "config: %s, set timeout error: %s", config, errParam);
        free(errParam);
        return -1;
    }

    param.log = plugin->common.log;
    plugin->interval = interval.v.val_int;

    if (mode.v.val_int == 1) {
        param.type = NEU_CONN_UDP;
        param.params.udp.dst_ip = dstHost.v.val_str;
        param.params.udp.dst_port = dstPort.v.val_int;
        param.params.udp.src_ip = host.v.val_str;
        param.params.udp.src_port = host.v.val_int;
        param.params.udp.dst_port = timeout.v.val_int;

        plog_notice(plugin, "config: host: %s, port: %" PRId64 ",mode: %" PRId64 ", dstIp: %s,dstPort: %" PRId64 "",
                    host.v.val_str, port.v.val_int, mode.v.val_int, dstHost.v.val_str, dstPort.v.val_int);
    }
    if (mode.v.val_int == 0) {
        param.type = NEU_CONN_UDP_TO;
        param.params.udpto.src_ip = host.v.val_str;
        param.params.udpto.src_port = host.v.val_int;
        param.params.udpto.timeout = timeout.v.val_int;

        plog_notice(plugin, "config: host: %s, port: %" PRId64 ",mode: %" PRId64 "", host.v.val_str, port.v.val_int,
                    mode.v.val_int);
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
    (void) plugin;
    (void) head;
    (void) data;
    return 0;
}

static int driver_tag_validator(const neu_datatag_t *tag) {
    LnxPoint point = {0};
    return lnx_tag_to_point(tag, &point);
}

static int driver_validate_tag(neu_plugin_t *plugin, neu_datatag_t *tag) {
    LnxPoint point = {0};
    int ret = lnx_tag_to_point(tag, &point);
    if (ret == 0) {
        plog_notice(plugin, "validate tag success,name: %s, address: %s, type: %d, ipAddr: %s, command: %s, msg: %s",
                    tag->name, tag->address, tag->type, point.ipAddr, point.command, point.msg);
    } else {
        plog_error(plugin, "validate tag success,name: %s, address: %s, type: %d, ipAddr: %s, command: %s, msg: %s",
                   tag->name, tag->address, tag->type, point.ipAddr, point.command, point.msg);
    }
    return ret;
}

static int driver_group_timer(neu_plugin_t *plugin, neu_plugin_group_t *group) {
    return lnx_group_timer(plugin, group, 0xfa);
}
