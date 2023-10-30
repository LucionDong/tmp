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
#include <memory.h>
#include <neuron.h>

#include "modbus_point.h"

struct LnxSortCtx {
    int cmdNum;
};

static int tag_cmp(neu_tag_sort_elem_t *tag1, neu_tag_sort_elem_t *tag2);
static bool tag_sort(neu_tag_sort_t *sort, void *tag, void *tag_to_be_sorted);

int lnx_tag_to_point(const neu_datatag_t *tag, LnxPoint *point) {
    int ret = NEU_ERR_SUCCESS;
    ret = neu_datatag_parse_addr_option(tag, &point->option);

    if (ret != 0) {
        return NEU_ERR_TAG_ADDRESS_FORMAT_INVALID;
    }

    printf("after strtok_r\n");
    char *tmpTag = tag->address;
    char *delimiters = "[]={}";
    // point->ipAddr = strtok_r(tmpTag, delimiters, &tmpTag);
    // point->command = strtok_r(NULL, delimiters, &tmpTag);
    point->command = strtok_r(tmpTag, delimiters, &tmpTag);
    point->msg = strtok_r(NULL, delimiters, &tmpTag);

    //  if ((tag->attribute & NEU_ATTRIBUTE_WRITE) == NEU_ATTRIBUTE_WRITE) {
    // return NEU_ERR_TAG_ATTRIBUTE_NOT_SUPPORT;
    //  }
    return ret;
}

int lnx_write_tag_to_point(const neu_plugin_tag_value_t *tag, LnxPointWrite *point) {
    int ret = NEU_ERR_SUCCESS;
    ret = lnx_tag_to_point(tag->tag, &point->point);
    point->value = tag->value;
    return ret;
}

LnxReadCmdSet *lnx_tag_sort(UT_array *tags, int maxByte) {
    printf("after strtok_r\n");
    neu_tag_sort_result_t *result = neu_tag_sort(tags, tag_sort, tag_cmp);
    LnxReadCmdSet *sortResult = calloc(1, sizeof(LnxReadCmdSet));
    sortResult->cmdNum = result->n_sort;
    sortResult->cmd = calloc(result->n_sort, sizeof(LnxReadCmd));

    neu_tag_sort_free(result);
    return sortResult;
}

// int lnx_write_tag_to_point(const neu_plugin_tag_value_t *tag, LnxPointWrite *point) {
// int ret = NEU_ERR_SUCCESS;
// ret = lnx_tag_to_point(tag->tag, &point->point);
// point->value = tag->value;
// return ret;
// }

void lnx_tag_sort_free(LnxReadCmdSet *result) {
    for (uint16_t i = 0; i < result->cmdNum; ++i) {
        utarray_free(result->cmd[i].tags);
    }
    free(result->cmd);
    free(result);
}

static bool tag_sort(neu_tag_sort_t *sort, void *tag, void *tag_to_be_sorted) {
    return true;
}

static int tag_cmp(neu_tag_sort_elem_t *tag1, neu_tag_sort_elem_t *tag2) {
    return 1;
}
