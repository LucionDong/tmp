/*
 * lnx_stack.h
 * Copyright (C) 2023 Lucion <dongbin0625@126.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef LNX_STACK_H
#define LNX_STACK_H

#include <assert.h>
#include <neuron/neuron.h>

#include "lnx.h"
// #include "lnx_req.h"

#define IPADDR "198.168.133.128"

typedef int (*lnx_stack_send)(void *ctx, char *msg, int msgNum);
typedef int (*lnx_stack_value)(void *ctx, char *ipAddr, char *msg, int msgNum);

typedef struct LnxStack_t {
    void *ctx;
    Code_e commandCode;
    char *ipAddr;
    uint8_t *buf;
    int bufSize;

    lnx_stack_send sendFn;
    lnx_stack_value valueFn;

} LnxStack;

LnxStack *lnx_stack_create(void *ctx, Code_e code, lnx_stack_send sendFn, lnx_stack_value valueFn);
void lnx_stack_destroy(LnxStack *stack);

int lnx_stack_recv(LnxStack *stack, neu_protocol_unpack_buf_t *buf, char *ipAddr);
int lnx_stack_write(LnxStack *stack, char *ipAddr, char *command, char *msg, int msgLen);
int lnx_stack_read(LnxStack *stack, char *ipAddr, char *command, char *msg, int msgLen, uint16_t *responseSize);

#endif /* !LNX_STACK_H */
