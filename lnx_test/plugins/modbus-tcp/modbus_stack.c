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
#include <assert.h>
#include <neuron.h>

#include "modbus_req.h"
#include "modbus_stack.h"

LnxStack *lnx_stack_create(void *ctx, Code_e code, lnx_stack_send sendFn, lnx_stack_value valueFn) {
    printf("modbus_stack_create\n");
    modbus_stack_t *stack = calloc(1, sizeof(LnxStack));

    stack->ctx = ctx;
    stack->sendFn = sendFn;
    stack->valueFn = valueFn;
    return stack;
}

void lnx_stack_destroy(LnxStack *stack) {
    printf("modbus_stack_destroy\n");
    free(stack);
}

int lnx_stack_recv(LnxStack *stack, neu_protocol_unpack_buf_t *buf, char *ipAddr) {
    printf("modbus_stack_recv\n");
    printf("lnx_stack_recv\n");
    struct Header_t header = {0};
    struct Command_t command = {0};
    struct Data_t data = {0};
    int ret = 0;

    if (strcmp(ipAddr, IPADDR)) {
        plog_error((neu_plugin_t *) stack->ctx, "ipAddr error");
        plog_info((neu_plugin_t *) stack->ctx, "ipAddr error");
        return -1;
    }

    if ((ret = lnx_header_unwrap(buf, &header)) <= 0) {
        plog_error((neu_plugin_t *) stack->ctx, "header unwrap error");
        return -1;
    }

    if ((ret = lnx_command_unwrap(buf, &command)) <= 0) {
        plog_error((neu_plugin_t *) stack->ctx, "command unwrap error");
        return -1;
    }

    switch (strcmp(command.commandCode, "REQ")) {
        case REQ:
            // req message
            if ((ret = lnx_data_unwrap(buf, &data)) <= 0) {
                plog_error((neu_plugin_t *) stack->ctx, "data unwrap error");
                return -1;
            }
            stack->sendFn(stack->ctx, data.msg, data.msgLen);
            break;
        case ACK:
            if ((ret = lnx_data_unwrap(buf, &data)) <= 0) {
                plog_error((neu_plugin_t *) stack->ctx, "data unwrap error");
                return -1;
            }
            uint8_t *bytes = NULL;
            bytes = neu_protocol_unpack_buf(buf, data.msgLen);
            stack->valueFn(stack->ctx, header.ipAddr, (char *) bytes, data.msgLen);
            break;
    }
    return neu_protocol_unpack_buf_used_size(buf);
}

int lnx_stack_write(LnxStack *stack, char *ipAddr, char *command, char *msg, int msgLen) {
    printf("lnx_stack_write\n");
    static __thread neu_protocol_pack_buf_t pbuf = {0};

    bzero(stack->buf, stack->bufSize);
    lnx_data_wrap(&pbuf, msg, msgLen);
    lnx_command_wrap(&pbuf, command);
    lnx_header_wrap(&pbuf, ipAddr);

    int ret = stack->sendFn(stack->ctx, msg, msgLen);
    return ret;
}

int lnx_stack_read(LnxStack *stack, char *ipAddr, char *command, char *msg, int msgLen, uint16_t *respo nseSize) {
    printf("lnx_stack_read\n");
    static __thread uint8_t buf[16] = {0};
    static __thread neu_protocol_pack_buf_t pbuf = {0};
    int ret = 0;
    *responseSize = 512;

    neu_protocol_pack_buf_init(&pbuf, buf, sizeof(buf));
    lnx_header_wrap(&pbuf, ipAddr);
    lnx_command_wrap(&pbuf, command);
    lnx_data_wrap(&pbuf, msg, msgLen);

    ret = stack->sendFn(stack->ctx, (char *) neu_protocol_pack_buf_get(&pbuf), neu_protocol_pack_buf_us ed_size(&pbuf));

    if (ret <= 0) {
        stack->valueFn(stack->ctx, NULL, NULL, 0);
        plog_warn((neu_plugin_t *) stack->ctx, "send read req fail");
    }
    return ret;
}

// bool modbus_stack_is_rtu(modbus_stack_t *stack) {
// printf("modbus_stack_is_rtu\n");
// return stack->protocol == MODBUS_PROTOCOL_RTU;
// }
