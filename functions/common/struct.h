//
// Created by admin on 06.11.2021.
//

#ifndef FILEIO_STRUCT_H
#define FILEIO_STRUCT_H
#include <zend_API.h>
#include <uv.h>
#include "../../constants.h"
#include "../common/fill_event_handle.h"

#define LOG_TAG "timer_handles"
#define ADD_HANDLE_TO_STRUCT(name) name##_id_item_t name##_handle_map[HANDLE_MAP_SIZE];
#define ADD_STRUCT(name, type) typedef struct {                                                                        \
        unsigned long long handle_id;                                                   \
        type * handle;                                                                  \
    } name##_id_item_t;

#define CREATE_HANDLE_LIST(name, type)                                                  \
unsigned short count_##name##_handles(name##_id_item_t * name##_handle_map) {                                                        \
    unsigned short i = 0;                                                               \
    for (; i < HANDLE_MAP_SIZE; i++) {                                                  \
        if (name##_handle_map[i].handle_id == 0) {                                     \
            LOG("Handles count - %d, last handle_id is %llu\n", i, name##_handle_map[i].handle_id); \
            break;                                                                      \
        }                                                                               \
    }                                                                                   \
    return i;                                                                           \
}                                                                                       \
unsigned long long add_##name##_handle(name##_id_item_t * name##_handle_map, type *handle) {                                           \
    unsigned short handle_count = count_##name##_handles(name##_handle_map);            \
    name##_handle_map[handle_count] = (name##_id_item_t){.handle_id=uv_hrtime(), .handle=handle};                                    \
    LOG("Added  - %d, last handle_id is %llu\n", handle_count+1, name##_handle_map[handle_count].handle_id);                          \
    return name##_handle_map[handle_count].handle_id;                                        \
}                                                                                       \
name##_id_item_t * find_##name##_handle(name##_id_item_t * name##_handle_map, unsigned long long handleId) {                           \
    unsigned short i = 0;                                                               \
    for (; i < HANDLE_MAP_SIZE; i++) {                                                  \
        if (name##_handle_map[i].handle_id == handleId) {                                 \
                                                                                        \
            return &name##_handle_map[i];                                                 \
        }                                                                                \
    }                                                                                    \
}                                                                                        \
void remove_##name##_handle(name##_id_item_t * name##_handle_map,unsigned long long handleId) {                                        \
    name##_id_item_t *tempItems = malloc(1024 * sizeof(name##_id_item_t));               \
    unsigned short i = 0;                                                                \
    unsigned short tagret = 0;                                                           \
    for (; i < HANDLE_MAP_SIZE; i++) {                                                   \
        if (name##_handle_map[i].handle_id == handleId) {                                \
            printf("Element #%d with handle_id=%llu was removed\n", i, name##_handle_map[i].handle_id); \
            continue;                                                                    \
        }                                                                                \
        tempItems[tagret] = name##_handle_map[i];                                        \
        tagret++;                                                                        \
    }                                                                                    \
    memcpy(name##_handle_map, tempItems, 1024 * sizeof( name##_id_item_t));              \
    free(tempItems);                                                                    \
}
#endif //FILEIO_STRUCT_H
