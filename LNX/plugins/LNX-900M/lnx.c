/*
 *
 * Copyright (C) 2023-10-16 14:42 Lucion <dongbin0625@126.com>
 *
 */
#include <neuron/utils/protocol_buf.h>
#include <string.h>

#include "lnx.h"

void lnx_header_wrap(neu_protocol_buf_t *buf, char *ipAddr) {
    assert(neu_protocol_pack_buf_unused_size(buf) >= sizeof(struct Header_t));
    struct Header_t *header = (struct Header_t *) neu_protocol_pack_buf(buf, sizeof(struct Header_t));

    header->ipAddr = ipAddr;
    printf("header wrap %s\n", header->ipAddr);
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
    printf("header unwrap %s\n", outHeader->ipAddr);
    return sizeof(struct Header_t);
}

void lnx_command_wrap(neu_protocol_buf_t *buf, char *commandCode) {
    assert(neu_protocol_pack_buf_unused_size(buf) >= sizeof(struct Command_t));
    struct Command_t *command = (struct Command_t *) neu_protocol_pack_buf(buf, sizeof(struct Command_t));

    command->commandCode = commandCode;
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
    struct Command_t *command = (struct Command_t *) neu_protocol_unpack_buf(buf, sizeof(struct Command_t));

    if (command == NULL) {
        return 0;
    }

    *outCommand = *command;
    printf("command unwrap %s\n", command->commandCode);
    return sizeof(struct Command_t);
}

void lnx_data_wrap(neu_protocol_pack_buf_t *buf, char *msg, int msgLen) {
    assert(neu_protocol_pack_buf_unused_size(buf) >= sizeof(struct Data_t));
    uint8_t *data = neu_protocol_pack_buf(buf, msgLen);
    memcpy(data, msg, msgLen);

    if (msgLen > 2) {
        struct Data_t *lData = (struct Data_t *) neu_protocol_pack_buf(buf, sizeof(struct Data_t));
        lData->msgLen = msgLen;
    }
    //     struct Data_t* data = (struct Data_t*) neu_protocol_pack_buf(buf, sizeof(struct Data_t));
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
    printf("********************\n");

	if(data->msg == NULL){
		printf("data.msg is NULL\n");
		return 0;
	}

    printf("data unwrap: %d\n", data->msgLen);
    printf("data unwrap %s\n", data->msg);
    return sizeof(struct Data_t);
}

#if 1
int main() {
    neu_protocol_pack_buf_t buf;
    uint8_t base = 42;
    neu_protocol_pack_buf_init(&buf, &base, 100);
    char *msg = malloc(7 * sizeof(char));
    strcpy(msg, "[shit]");
    // msg = "[shit]";
    lnx_header_wrap(&buf, msg);
    // lnx_header_wrap(&buf, "[shit]");
    lnx_command_wrap(&buf, "ACK");
    char message[] = "what do";
    lnx_data_wrap(&buf, msg, strlen(msg) + 1);

    Header header = {0};
    Command command = {0};
    Data data = {0};
    // printf("sizeof Data %d\n", (int) sizeof(Data));
    lnx_data_unwrap(&buf, &data);
    // printf("sizeof Data %d\n", (int) strlen(data.msg));
    lnx_command_unwrap(&buf, &command);
    lnx_header_unwrap(&buf, &header);
    return 0;
    //     uint8_t* ret = neu_protocol_unpack_buf(&buf, 10);
    //     printf("ret %s\n", ret);
    // lnx_data_unwrap(&buf, &data);
}
#endif

// void lnx_ack_wrap(neu_protocol_buf_t* buf, char* ipAddr) {
//     assert(neu_protocol_pack_buf_unused_size(buf) >= sizeof(struct Ack_t));
//     struct Ack_t* ack = (struct Ack_t*) neu_protocol_pack_buf(buf, sizeof(struct Ack_t));
//
//     strcpy(ack->ipAddr, ipAddr);
// }
//
// int lnx_ack_unwrap(neu_protocol_buf_t* buf, Ack* outAck) {
//     struct Ack_t* ack = (struct Ack_t*) neu_protocol_unpack_buf(buf, sizeof(struct Ack_t));
//
//     if (ack == NULL) {
//         return 0;
//     }
//
//     *outAck = *ack;
//     return sizeof(struct Ack_t);
// }
