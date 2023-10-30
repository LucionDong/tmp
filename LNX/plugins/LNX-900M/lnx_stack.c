/*
 *
 * Copyright (C) 2023-10-16 16:34 Lucion <dongbin0625@126.com>
 *
 */
#include <neuron/plugin.h>
#include <neuron/utils/protocol_buf.h>

// #include "lnx.h"
#include "lnx_req.h"
#include "lnx_stack.h"

LnxStack *lnx_stack_create(void *ctx, Code_e code, lnx_stack_send sendFn, lnx_stack_value valueFn) {
    LnxStack *stack = calloc(1, sizeof(LnxStack));

    stack->ctx = ctx;
    stack->sendFn = sendFn;
    stack->valueFn = valueFn;

    return stack;
}

void lnx_stack_destroy(LnxStack *stack) {
    free(stack);
}

// from manager
int lnx_stack_recv(LnxStack *stack, neu_protocol_unpack_buf_t *buf, char *ipAddr) {
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

// from LNX
int lnx_stack_write(LnxStack *stack, char *ipAddr, char *command, char *msg, int msgLen) {
    static __thread neu_protocol_pack_buf_t pbuf = {0};

    bzero(stack->buf, stack->bufSize);
    lnx_data_wrap(&pbuf, msg, msgLen);
    lnx_command_wrap(&pbuf, command);
    lnx_header_wrap(&pbuf, ipAddr);

    int ret = stack->sendFn(stack->ctx, msg, msgLen);
    return ret;
}

int lnx_stack_read(LnxStack *stack, char *ipAddr, char *command, char *msg, int msgLen, uint16_t *responseSize) {
    static __thread uint8_t buf[16] = {0};
    static __thread neu_protocol_pack_buf_t pbuf = {0};
    int ret = 0;
    *responseSize = 512;

    neu_protocol_pack_buf_init(&pbuf, buf, sizeof(buf));
    lnx_header_wrap(&pbuf, ipAddr);
    lnx_command_wrap(&pbuf, command);
    lnx_data_wrap(&pbuf, msg, msgLen);

    ret = stack->sendFn(stack->ctx, (char *) neu_protocol_pack_buf_get(&pbuf), neu_protocol_pack_buf_used_size(&pbuf));

    if (ret <= 0) {
        stack->valueFn(stack->ctx, NULL, NULL, 0);
        plog_warn((neu_plugin_t *) stack->ctx, "send read req fail");
    }
    return ret;
}
