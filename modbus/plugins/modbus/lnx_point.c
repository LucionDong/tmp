/*
 *
 * Copyright (C) 2023-10-17 20:03 Lucion <dongbin0625@126.com>
 *
 */
#include <neuron/errcodes.h>
#include <neuron/tag_sort.h>
#include <neuron/utils/utarray.h>
#include <neuron/utils/utlist.h>

#include "lnx_point.h"

struct LnxSortCtx {
    int cmdNum;
};

static bool tag_sort(neu_tag_sort_t *sort, void *tag, void *tag_to_be_sorted);
static int tag_cmp(neu_tag_sort_elem_t *tag1, neu_tag_sort_elem_t *tag2);

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
