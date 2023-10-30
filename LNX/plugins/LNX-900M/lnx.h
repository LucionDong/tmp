/*
 * lnx.h
 * Copyright (C) 2023 Lucion <dongbin0625@126.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef LNX_H
#define LNX_H

#include <neuron/neuron.h>
#include <stdint.h>
#include <stdio.h>

typedef enum Code_e {
    REQ = 0,
    ACK = 1,
} Code_e;

typedef struct Header_t {
    char *ipAddr;
} Header;

typedef struct Command_t {
    char *commandCode;
} Command;

typedef struct Data_t {
    char *msg;
    int msgLen;
} Data;

// typedef struct Ack_t {
// char* ipAddr;
// int ack;
// } Ack;

void lnx_header_wrap(neu_protocol_buf_t *buf, char *ipAddr);
int lnx_header_unwrap(neu_protocol_buf_t *buf, Header *outHeader);

void lnx_command_wrap(neu_protocol_buf_t *buf, char *commandCode);
int lnx_command_unwrap(neu_protocol_buf_t *buf, Command *outCommand);

void lnx_data_wrap(neu_protocol_buf_t *buf, char *msg, int msgLen);
int lnx_data_unwrap(neu_protocol_buf_t *buf, Data *outData);

// void lnx_ack_wrap(neu_protocol_buf_t* buf, char* ipAddr);
// int lnx_ack_unwrap(neu_protocol_buf_t* buf, Ack* outAck);
#endif /* !LNX_H */
