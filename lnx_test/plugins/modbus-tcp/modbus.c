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
#include <netinet/in.h>
// #include <neuron/neuron.h>

#include "modbus.h"

void lnx_header_wrap(neu_protocol_pack_buf_t *buf, char *ipAddr) {
    printf("modbus_header_wrap\n");
    assert(neu_protocol_pack_buf_unused_size(buf) >= sizeof(struct Header_t));
    struct Header_t *header = (struct Header_t *) neu_protocol_pack_buf(buf, sizeof(struct He ader_t));

    memcpy(header->ipAddr, ipAddr, strlen(ipAddr));
    printf("after lnx_header_wrap\n");
}

int lnx_header_unwrap(neu_protocol_buf_t *buf, Header *outHeader) {
    struct Header_t *header = (struct Header_t *) neu_protocol_unpack_buf(buf, sizeof(struct Header_t));

    if (header == NULL) {
        return 0;
    }

    // printf("header unwrap first %s\n", header->ipAddr);
    //     const char delim[] = "[]";
    //     outHeader->ipAddr = strtok(header->ipAddr, delim);
    *outHeader = *header;
    // printf("header unwrap %s\n", outHeader->ipAddr);
    return sizeof(struct Header_t);
}

void lnx_command_wrap(neu_protocol_buf_t *buf, char *commandCode) {
    assert(neu_protocol_pack_buf_unused_size(buf) >= sizeof(struct Command_t));
    struct Command_t *command = (struct Command_t *) neu_protocol_pack_buf(buf, sizeof(struct Command_t));

    memcpy(command->commandCode, commandCode, strlen(commandCode));
    // command->commandCode = strdup(commandCode);

    //     if (!strcmp(commandCode, "REQ")) {
    //     command->commandCode = REQ;
    //     printf("REQ\n");
    // } else if (!strcmp(commandCode, "ACK")) {
    //     command->commandCode = ACK;
    //     printf("ACK\n");
    //     }

    // strcpy(command->commandCode, commandCode);
}

int lnx_command_unwrap(neu_protocol_buf_t *buf, Command *outCommand) {
    struct Command_t *command = (struct Command_t *) neu_protocol_unpack_buf(buf, sizeof(stru ct Command_t));

    if (command == NULL) {
        return 0;
    }

    *outCommand = *command;
    // printf("command unwrap %s\n", command->commandCode);
    return sizeof(struct Command_t);
}

void lnx_data_wrap(neu_protocol_pack_buf_t *buf, char *msg, int msgLen) {
    assert(neu_protocol_pack_buf_unused_size(buf) >= sizeof(struct Data_t));
    uint8_t *data = neu_protocol_pack_buf(buf, msgLen);
    memcpy(data, msg, msgLen);

    if (msgLen > 2) {
        struct Data_t *lData = (struct Data_t *) neu_protocol_pack_buf(buf, sizeof(struct Dat a_t));
        lData->msgLen = msgLen;
    }
    //     struct Data_t* data = (struct Data_t*) neu_protocol_pack_buf(buf, sizeof(struct Da  ta_t));
    //
    // data->msg = msg;
    //     printf("data wrap %s\n", data->msg);
    // strcpy(data->msg, msg);
}

int lnx_data_unwrap(neu_protocol_buf_t *buf, Data *outData) {
    struct Data_t *data = (struct Data_t *) neu_protocol_unpack_buf(buf, sizeof(struct Data_t));

    if (data == NULL) {
        return 0;
    }

    *outData = *data;
    // printf("data unwrap %s\n", data->msg);
    return sizeof(struct Data_t);
}
