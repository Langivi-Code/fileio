//
// Created by admin on 14.01.2022.
//
#include <php.h>
#include <uv.h>
#include "background.h"
#include "../../php_fileio.h"
#include "standard/php_var.h"
#include "../common/callback_interface.h"
#include "../common/call_php_fn.h"

void async_cb(uv_async_t *async, int status) {
    printf("async_cb\n");
    uv_close((uv_handle_t *) async, NULL);
}

void thread_cb(void *arg) {
    uv_async_t *comm = (uv_async_t *) arg;
    printf("P=%p\n", comm->data);
    printf("pre async send\n");
    HashTable * array = comm->data;
    zval *item;
    zval retavl;
//    zend_fcall_info fci = empty_fcall_info;
//    zend_fcall_info_cache fcc = empty_fcall_info_cache;
    uv_cb_type cb = {0};
    cb.fci = empty_fcall_info;
    cb.fcc = empty_fcall_info_cache;
    ZEND_HASH_FOREACH_VAL(array, item)
            zval dup;
            ZVAL_COPY_VALUE(&dup, item);
            zend_fcall_info_init(&dup, 0, &cb.fci, &cb.fcc, NULL, NULL);
            cb.fci.param_count=0;
            cb.fci.params = NULL;
            cb.fci.retval = &retavl;
            zend_call_function(&cb.fci, &cb.fcc);
            php_var_dump(item,1);
    ZEND_HASH_FOREACH_END();
    uv_async_send(comm);
    printf("post async send\n");
}

PHP_FUNCTION (work) {
    zval * array;
    HashTable * myht;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ARRAY(array)ZEND_PARSE_PARAMETERS_END();
    myht = Z_ARRVAL_P(array);
    uv_async_t *async_work = emalloc(sizeof(uv_async_t));
    HashTable * myht_work = emalloc(sizeof(HashTable));
    memcpy(myht_work, myht, sizeof(HashTable));
    async_work->data = myht_work;
    printf("P=%p\n", async_work->data);
    printf("clusures count %zu\n", zend_hash_num_elements(myht_work));
    uv_async_init(MODULE_GL(loop), async_work, async_cb);
    uv_thread_t *thread = emalloc(sizeof(uv_thread_t));
    uv_thread_create(thread, thread_cb, (void *) async_work);

}