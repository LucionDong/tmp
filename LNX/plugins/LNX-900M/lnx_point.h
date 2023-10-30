/*
 * lnx_point.h
 * Copyright (C) 2023 Lucion <dongbin0625@126.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef LNX_POINT_H
#define LNX_POINT_H

#include <neuron/neuron.h>
#include <neuron/tag.h>

typedef struct LnxPoint_t {
    char *ipAddr;
    char *command;
    char *msg;

    neu_datatag_addr_option_u option;
} LnxPoint;

typedef struct LnxPointWrite_t {
    LnxPoint point;
    neu_value_u value;
} LnxPointWrite;

int lnx_tag_to_point(const neu_datatag_t *tag, LnxPoint *point);
int lnx_write_tag_to_point(const neu_plugin_tag_value_t *tag, LnxPointWrite *point);

typedef struct LnxReadCmd_t {
    char *ipAddr;
    char *command;
    char *msg;

    UT_array *tags;
} LnxReadCmd;

typedef struct LnxReadCmdSet_t {
    uint16_t cmdNum;
    LnxReadCmd *cmd;
} LnxReadCmdSet;

void lnx_tag_sort_free(LnxReadCmdSet *result);
LnxReadCmdSet *lnx_tag_sort(UT_array *tags, int maxByte);
// typedef struct LnxWriteCmd_t {
// char *ipAddr;
// char *command;
// char *msg;
//
// UT_array *tags;
// } LnxWriteCmd;

#endif /* !LNX_POINT_H */
