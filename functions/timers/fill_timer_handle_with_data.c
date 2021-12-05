#include <zend_API.h>
#include <uv.h>
#include "../../constants.h"
#include "timers_interface.h"
#include "../common/fill_event_handle.h"
void fill_timer_handle_with_data(
        uv_timer_t *handle,
        zend_fcall_info *fci,
        zend_fcall_info_cache *fcc
) {
#define LOG_TAG "fill_timer"
    uv_cb_type uv = {};
    LOG("size of timeout handler %lu, fci  %lu \n\n", sizeof *handle, sizeof *fci);
    handle->data = (uv_cb_type *) emalloc(sizeof(uv_cb_type));
    fill_event_handle(handle, fci, fcc, &uv);
}

#define LOG_TAG "timer_handles"
handle_id_item_t timer_handle_map[HANDLE_MAP_SIZE];

unsigned short count_handles() {
    unsigned short i = 0;
    for (; i < HANDLE_MAP_SIZE; i++) {
        if (timer_handle_map[i].handle_id == 0) {
            LOG("Handles count - %d, last handle_id is %llu", i, timer_handle_map[i].handle_id);
            break;
        }
    }
    return i;
}

unsigned long long add_handle(uv_timer_t *handle) {
    unsigned short handle_count = count_handles();
    timer_handle_map[handle_count] = (handle_id_item_t){.handle_id=uv_hrtime(), .handle=handle};
    return timer_handle_map[handle_count].handle_id;
}

handle_id_item_t * find_handle(unsigned long long handleId) {
    unsigned short i = 0;
    for (; i < HANDLE_MAP_SIZE; i++) {
        if (timer_handle_map[i].handle_id == handleId) {
            LOG("Searching element #%d with handle_id=%llu", i, timer_handle_map[i].handle_id);
            return &timer_handle_map[i];
        }
    }
}

void remove_handle(unsigned long long handleId) {
    handle_id_item_t *tempItems = malloc(1024 * sizeof(handle_id_item_t));
    unsigned short i = 0;
    unsigned short tagret = 0;
    for (; i < HANDLE_MAP_SIZE; i++) {
        if (timer_handle_map[i].handle_id == handleId) {
            printf(" element #%d with handle_id=%llu was removed", i, timer_handle_map[i].handle_id);
            continue;
        }
        tempItems[tagret] = timer_handle_map[i];
        tagret++;
    }
    memcpy(timer_handle_map, tempItems, 1024 * sizeof(handle_id_item_t));
    free(tempItems);
}