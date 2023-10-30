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
#ifndef _NEU_PLUGIN_MODBUS_POINT_H_
#define _NEU_PLUGIN_MODBUS_POINT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <neuron.h>
#include <stdint.h>

#include "modbus.h"

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

int modbus_tag_to_point(const neu_datatag_t *tag, LnxPoint *point);
int modbus_write_tag_to_point(const neu_plugin_tag_value_t *tag, LnxPointWrite *point);

typedef struct LnxReadCmd_t {
    char *ipAddr;
    char *command;
    char *msg;

    UT_array *tags;  // modbus_point_t ptr;
} LnxReadCmd;

typedef struct LnxReadCmdSet_t {
    uint16_t cmdNum;
    LnxReadCmd *cmd;
} LnxReadCmdSet;

void lnx_tag_sort_free(LnxReadCmdSet *result);
LnxReadCmdSet *lnx_tag_sort(UT_array *tags, int maxByte);

// typedef struct modbus_write_cmd {
//     uint8_t slave_id;
//     modbus_area_e area;
//     uint16_t start_address;
//     uint16_t n_register;
//     uint8_t n_byte;
//     uint8_t *bytes;
//
//     UT_array *tags;
// } modbus_write_cmd_t;
//
// typedef struct modbus_write_cmd_sort {
//     uint16_t n_cmd;
//     modbus_write_cmd_t *cmd;
// } modbus_write_cmd_sort_t;
//
// modbus_read_cmd_sort_t *modbus_tag_sort(UT_array *tags, uint16_t max_byte);
// modbus_write_cmd_sort_t *modbus_write_tags_sort(UT_array *tags);
// void modbus_tag_sort_free(modbus_read_cmd_sort_t *cs);
//
#ifdef __cplusplus
}
#endif

#endif
