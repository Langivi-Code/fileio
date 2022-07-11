//
// Created by admin on 14.09.2021.
//
#include <php.h>
#include <php_ini.h>
#include "zend.h"
#include <zend_API.h>
#include <zend_closures.h>
#include "../common/callback_interface.h"
#include "../../constants.h"
#include "../../php_fileio.h"
#include "zend_enum.h"
#include "promise.h"
#include "../http/request.h"
#include "../common/register_property.h"
#include "standard/php_var.h"
#include "../common/fill_event_handle.h"
#include "../common/call_php_fn.h"
#include "../../rustlib/include/stdasync_lib.h"

zend_function *promise_resolve;
zend_function *promise_reject;
static const zend_function_entry enum_PromiseStatus_methods[] = {
        ZEND_FE_END
};
static int res_num;

void then_cb(uv_prepare_t *handle);

void put_data_in_new_promise(zend_object *data, zval *retval);

void *get_resource(zend_object *this);

void init_new_promise_prepare_loop(zend_object *next) {
    zend_fcall_info fci = empty_fcall_info;
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
    zval * _then = zend_read_property(next->ce, next, PROP("_then"), 0, NULL);
    GC_TRY_ADDREF(next);
    uv_prepare_t *prepare_handle = emalloc(sizeof(uv_prepare_t));
    puts("Setting new then pending prepare");
    uv_prepare_init(MODULE_GL(loop), prepare_handle);
    then_t *handle_data = emalloc(sizeof(then_t));
    zend_fcall_info_init(_then, 0, &fci, &fcc, NULL, NULL);
    init_cb(&fci, &fcc, &handle_data->then_cb);
    puts("Setting init_new_promise_prepare_loop prepare");
    handle_data->this = next;
    php_var_dump(_then, 1);
    prepare_handle->data = handle_data;
    uv_prepare_start(prepare_handle, then_cb); /// how to pass smth from promise to promise
    puts("init_new_promise_prepare_loop started");

    printf("size of ev-queue %d(Active = %d), loop address:=%p\n", uv_loop_alive(MODULE_GL(loop)),
           MODULE_GL(loop)->active_handles, MODULE_GL(loop));
    uv_run(MODULE_GL(loop), UV_RUN_DEFAULT);
}

static void my_res_dtor(zend_resource *rsrc) {
    remove_promise_list(zend_fetch_resource(rsrc, "my_res", res_num));
}

zend_resource *reg_handler(void *fp) {
    zval my_val;
/* module_number should be your PHP extension number here */
    return (zend_resource *) zend_register_resource(fp, res_num);
}

void fn_idle(uv_idle_t *handle) {

#define LOG_TAG "promise_construct"
    LOG("Setting idle in Promise construct ...\n");
    then_t *data_handle = (then_t *) handle->data;
    zval retval;
    zval * params = emalloc(2 * sizeof(zval));
    zval func;
    zval this;
    ZVAL_OBJ(&this, data_handle->this);

    zend_create_fake_closure(&func, promise_resolve, MODULE_GL(promise_class), MODULE_GL(promise_class),
                             &this);
    ZVAL_COPY(&params[0], &func);
    zend_create_fake_closure(&func, promise_reject, MODULE_GL(promise_class), MODULE_GL(promise_class),
                             &this);
    ZVAL_COPY(&params[1], &func);
//    ZVAL_STRING(&dstr, "callback fn");
    //    zend_call_method_with_1_params(NULL, NULL, NULL, "print_r", &retval, &dstr);
    uv_idle_stop(handle);
    LOG("Promise handler call back is called");
    call_php_fn(&data_handle->then_cb, 2, params, &retval, "promise_handler");

//    efree(data_handle->then_cb.fci.params);
    efree(data_handle);
//    php_var_dump(params, 1);
    efree(handle);
}

void copy_promise_vals(zval *source, zend_object *target) {
    zval * data_store_ret = zend_read_property(Z_OBJCE_P(source), Z_OBJ_P(source), PROP("dataStore"), 0,
                                               NULL);
    zval * status = zend_read_property(Z_OBJCE_P(source), Z_OBJ_P(source), PROP("status"), 0,
                                       NULL);
    zval * closure = zend_read_property(Z_OBJCE_P(source), Z_OBJ_P(source), PROP("closure"), 0,
                                        NULL);
    zval * finalized = zend_read_property(Z_OBJCE_P(source), Z_OBJ_P(source), PROP("promiseFinalised"), 0,
                                          NULL);
    zend_update_property(target->ce, target, PROP("dataStore"), data_store_ret);
    zend_update_property(target->ce, target, PROP("status"), status);
    zend_update_property(target->ce, target, PROP("closure"), closure);
    zend_update_property(target->ce, target, PROP("promiseFinalised"), finalized);
}

//static zend_object_handlers enum_handlers;
//zend_object *zend_enum_new(zval *result, zend_class_entry *ce, zend_string *case_name, zval *backing_value_zv)
//{
//    zend_object *zobj = zend_objects_new(ce);
//    ZVAL_OBJ(result, zobj);
//
//    ZVAL_STR_COPY(OBJ_PROP_NUM(zobj, 0), case_name);
//    if (backing_value_zv != NULL) {
//        ZVAL_COPY(OBJ_PROP_NUM(zobj, 1), backing_value_zv);
//    }
//
//    zobj->handlers = &enum_handlers;
//
//    return zobj;
//}
zend_object *create_new_promise(zend_object *prev_object) {
    zval status, promise;
    object_init_ex(&promise, MODULE_GL(promise_class));
    zend_object * pending = zend_enum_get_case_cstr(MODULE_GL(promise__status_enum), "None");
    ZVAL_OBJ(&status, pending);
//    zend_update_property(MODULE_GL(promise_class), Z_OBJ(resolved_promise), PROP("dataStore"), param);
//    Z_TYPE_INFO(resolved_promise) = IS_OBJECT;
    printf("retval reslove %d\n", Z_TYPE_INFO(promise));
    zval * my_val = zend_read_property(MODULE_GL(promise_class), prev_object, PROP("_list"), 0, NULL);
    zend_update_property(MODULE_GL(promise_class), Z_OBJ(promise), PROP("_list"), my_val);
    zend_update_property(MODULE_GL(promise_class), Z_OBJ(promise), PROP("status"), &status);
    zend_object * obj = Z_OBJ(promise);
    return obj;
}

PHP_FUNCTION (use_promise) {
//    zval property___status_default_value;
//    zval back;
//    ZVAL_NULL(&back);
//    zend_string * str= zend_string_init("Pending", sizeof("Pending") - 1, 1);
//        zend_enum_new(&property___status_default_value, promise_enum, str, &back);
//    RETURN_OBJ(&property___status_default_value);
    RETURN_BOOL(INI_BOOL("file_io.use_promise"));
}


zend_class_entry *create_PromiseStatus_enum(void) {
    MODULE_GL(promise__status_enum) = zend_register_internal_enum("PromiseStatus", IS_LONG,
                                                                  enum_PromiseStatus_methods);
    zval * none = pemalloc(sizeof(zval), 1);
    ZVAL_LONG(none, -1);
    zval * pending = pemalloc(sizeof(zval), 1);
    ZVAL_LONG(pending, 0);
    zval * resolved = pemalloc(sizeof(zval), 1);
    ZVAL_LONG(resolved, 1);
    zval * rejected = pemalloc(sizeof(zval), 1);
    ZVAL_LONG(rejected, 2);
    zend_enum_add_case_cstr(MODULE_GL(promise__status_enum), "None", none);
    zend_enum_add_case_cstr(MODULE_GL(promise__status_enum), "Pending", pending);
    zend_enum_add_case_cstr(MODULE_GL(promise__status_enum), "Resolved", resolved);
    zend_enum_add_case_cstr(MODULE_GL(promise__status_enum), "Rejected", rejected);
    return MODULE_GL(promise__status_enum);
}

//
//static zend_class_entry *promise_class_entry = NULL;
//
PHP_METHOD (Promise, __construct) {
    zval * callback;
    zend_resource *my_res;
    zval my_val;
    zend_fcall_info fci = empty_fcall_info;
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
    zval * this = ZEND_THIS;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(callback)ZEND_PARSE_PARAMETERS_END();

    zend_update_property(MODULE_GL(promise_class), Z_OBJ_P(ZEND_THIS), PROP("closure"), callback);
    zend_fcall_info_init(callback,
                         0, &fci, &fcc, NULL, NULL);
#define LOG_TAG "PROMISE"
    uv_idle_t *idleHandle = emalloc(sizeof(uv_idle_t));
    printf("ZEND THIS handle size %p %d\n", ZEND_THIS, ZEND_THIS == NULL);
    then_t *data_handle = emalloc(sizeof(then_t));
    init_cb(&fci, &fcc, &data_handle->then_cb);
    data_handle->this = Z_OBJ_P(ZEND_THIS);
    void *list = init_promise_list(data_handle->this);
    my_res = reg_handler(list);
    ZVAL_RES(&my_val, my_res);
    zend_update_property(MODULE_GL(promise_class), Z_OBJ_P(ZEND_THIS), PROP("_list"), &my_val);
//    GC_TRY_ADDREF(data_handle->this);
    GC_TRY_ADDREF (Z_OBJ_P(callback));
    idleHandle->data = data_handle;
    LOG("Setting idle ...\n");
    uv_idle_init(MODULE_GL(loop), idleHandle);
    zval status;
    zend_object * pending = zend_enum_get_case_cstr(MODULE_GL(promise__status_enum), "Pending");
    ZVAL_OBJ(&status, pending);
    zend_update_property(MODULE_GL(promise_class), Z_OBJ_P(ZEND_THIS), PROP("status"), &status);

//    zend_call_known_function(promise_resolve, NULL, FILE_IO_GLOBAL(promise_class), &callable3, 0, NULL, NULL);
    uv_idle_start(idleHandle, fn_idle
    );
}

PHP_METHOD (Promise, resolve) {
    zval * param;
    zval status, resolved_promise, closure;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL_OR_NULL(param)ZEND_PARSE_PARAMETERS_END();
    object_init_ex(&resolved_promise, MODULE_GL(promise_class));
    zend_object * resolved = zend_enum_get_case_cstr(MODULE_GL(promise__status_enum), "Resolved");
    ZVAL_OBJ(&status, resolved);
    zend_update_property(MODULE_GL(promise_class),
                         Z_OBJ(resolved_promise), PROP("dataStore"), param);
//    Z_TYPE_INFO(resolved_promise) = IS_OBJECT;
    object_init_ex(&closure, zend_ce_closure);
    printf("retval reslove %d\n",
           Z_TYPE_INFO(resolved_promise)
    );
    zend_update_property(MODULE_GL(promise_class),
                         Z_OBJ(resolved_promise), PROP("closure"), &closure);

    zend_update_property(MODULE_GL(promise_class),
                         Z_OBJ(resolved_promise), PROP("status"), &status);
    zend_object * obj = Z_OBJ(resolved_promise);

    RETURN_OBJ(obj);
}

PHP_METHOD (Promise, reject
) {
    zval * param;
    zval status, rejected_promise, closure;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL_OR_NULL(param)ZEND_PARSE_PARAMETERS_END();

    object_init_ex(&rejected_promise, MODULE_GL(promise_class));
    zend_object * resolved = zend_enum_get_case_cstr(MODULE_GL(promise__status_enum), "Rejected");
    ZVAL_OBJ(&status, resolved);
    zend_update_property(MODULE_GL(promise_class),
                         Z_OBJ(rejected_promise), PROP("dataStore"), param);

    object_init_ex(&closure, zend_ce_closure);

    zend_update_property(MODULE_GL(promise_class),
                         Z_OBJ(rejected_promise), PROP("closure"), &closure);

    zend_update_property(MODULE_GL(promise_class),
                         Z_OBJ(rejected_promise), PROP("status"), &status);
    zend_object * obj = Z_OBJ(rejected_promise);

    RETURN_OBJ(obj);
}

PHP_METHOD (Promise, resolved) {
    printf("it resolved\n");
    zval * param;
    zval status;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL_OR_NULL(param)ZEND_PARSE_PARAMETERS_END();
    zend_object * resolved = zend_enum_get_case_cstr(MODULE_GL(promise__status_enum), "Resolved");
    ZVAL_OBJ(&status, resolved);

    zend_update_property(MODULE_GL(promise_class), Z_OBJ_P(ZEND_THIS), PROP("dataStore"), param);
    zend_update_property(MODULE_GL(promise_class), Z_OBJ_P(ZEND_THIS), PROP("status"), &status);
    switch (
            Z_TYPE_P(param)
            ) {
        case IS_STRING:
//            php_printf("resolved %s val", ZSTR_VAL(Z_STR_P(param)));
            break;
        case IS_NULL:
            break;

    }
    printf("resolved finished\n");
//    GC_TRY_DELREF(Z_OBJ_P(ZEND_THIS));
//    RETURN_OBJ(resolved);
}

PHP_METHOD (Promise, rejected
) {
    printf("it rejected\n");
    zval * param;
    zval * status = emalloc(sizeof(zval));
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL_OR_NULL(param)ZEND_PARSE_PARAMETERS_END();

    zend_object * rejected = zend_enum_get_case_cstr(MODULE_GL(promise__status_enum), "Rejected");

    ZVAL_OBJ(status, rejected
    );
    zend_update_property(MODULE_GL(promise_class), Z_OBJ_P(ZEND_THIS), PROP("dataStore"), param);
    zval rv;
    zend_update_property(MODULE_GL(promise_class), Z_OBJ_P(ZEND_THIS), PROP("status"), status);
    zend_read_property(MODULE_GL(promise_class), Z_OBJ_P(ZEND_THIS), PROP("status"), 0, &rv);

    printf("%p, %p,\n", rejected, &
            Z_OBJ(rv)
    );
    switch (
            Z_TYPE_P(param)
            ) {
        case IS_STRING:
            php_printf("rejected with  %s val",

                       ZSTR_VAL (Z_STR_P(param))

            );
            break;
        case IS_NULL:
            break;

    }
//    GC_TRY_DELREF(Z_OBJ_P(ZEND_THIS));
    RETURN_OBJ(rejected);
}

void then_cb(uv_prepare_t *handle) {
    puts("I have started");
    then_t *data = handle->data;
    zval * promiseFinalized, *status;
    short promiseFinalized_bool, status_val;
    puts("Uv loop in then cb");
    uv_prepare_stop(handle);
    zval zval2;
    ZVAL_OBJ(&zval2, data->this);
    php_var_dump(&zval2, 1);
    puts("conflict4");
    promiseFinalized = zend_read_property(data->this->ce, data->this, PROP("promiseFinalised"), 0, NULL);
    puts("conflict5");
    status = zend_read_property(data->this->ce, data->this, PROP("status"), 0, NULL);
    promiseFinalized_bool = Z_TYPE_INFO_P(promiseFinalized);
    status_val = Z_LVAL_P(OBJ_PROP_NUM(Z_OBJ_P(status), 1));
    printf("status value: %d\n", status_val);
    puts("conflict3");
    if (promiseFinalized_bool == IS_TRUE) {
        printf("Promise is finalized\n");//maybe!
        efree(data);
        efree(handle);
        return;
    } else if (status_val == Resolved) {
        printf("In resolve section of then_cb\n");
        puts("Current this:");
        zval * data_store = zend_read_property(data->this->ce, data->this, PROP("dataStore"), 0,
                                               NULL);

        zval retval;
        zend_fcall_info fci = empty_fcall_info;
        zend_fcall_info_cache fcc = empty_fcall_info_cache;
        php_var_dump(data_store, 1);
        LOG("Then call back is called");
        call_php_fn(&data->then_cb, 1, data_store, &retval, "then");
        puts("return value from then closure:");
        php_var_dump(&retval, 1);
        void *resourse = get_resource(data->this);
        puts("List recieved");
        printf("resourse pointer List=%p Current=%p\n", resourse, data->this);
        if (Z_TYPE(retval) == IS_OBJECT && instanceof_function(Z_OBJCE(retval), MODULE_GL(promise_class))) {
            GC_TRY_ADDREF(Z_OBJ(retval));
            puts("Current RET VAL:");
            php_var_dump(&retval, 1);
//            GC_TRY_ADDREF(data);
//            copy_promise_vals(retval, data);
            puts("CLEARING PROMISES CAUSE OF NEW PROMISE");
            zend_object * target_obj = Z_OBJ(retval);
            ZVAL_OBJ(data->then_retval, target_obj);
            zend_resource * new_resource = get_resource(target_obj);

            if (move_to_another_list(resourse, new_resource, data->this)) {
                remove_promise_list(resourse);
                if (has_next_promise_from_list(new_resource, target_obj)) {
                    zend_object * source_obj = get_next_promise_from_list(new_resource, target_obj);
                    zval * _then = zend_read_property(source_obj->ce, source_obj, PROP("_then"), 0, NULL);
                    zend_update_property(target_obj->ce, target_obj, PROP("_then"), _then);
                    init_new_promise_prepare_loop(target_obj);
                }

                //reassign _list
            } else {
                puts("something went wrong");
            }
            /// HERE WE MUST STOP it and return existing promise! or stop it???
        } else {
            if (has_next_promise_from_list(resourse, data->this)) {
                puts("Has next");
                zend_object * next = get_next_promise_from_list(resourse, data->this);
                ZVAL_OBJ(&zval2, next);
                puts("Next recieved");
                put_data_in_new_promise(next, &retval);
                php_var_dump(&zval2, 1);
                if (has_next_promise_from_list(resourse, next)) {
                    init_new_promise_prepare_loop(next);
                } else {
                    puts("CLEARING PROMISES");
                    remove_promise_list(resourse);
                }

                //Restart then_cb loop!
            }
        }
        efree(data);
        efree(handle);
    } else if (status_val == Rejected) {
        efree(data);
        efree(handle);
    } else if (status_val == Pending) {
        uv_prepare_t *idleHandle = emalloc(sizeof(uv_prepare_t));
        puts("Setting new then pending prepare");
        uv_prepare_init(MODULE_GL(loop), idleHandle);
        idleHandle->data = handle->data;
        efree(handle);
        uv_prepare_start(idleHandle, then_cb); /// how to pass smth from promise to promise
    }
}

void put_data_in_new_promise(zend_object *data, zval *retval) {
    zval zval1;
    ZVAL_OBJ(&zval1, data);
    zend_object * status_obj = zend_enum_get_case_cstr(MODULE_GL(promise__status_enum), "Resolved");
    zval status;
    ZVAL_OBJ(&status, status_obj);
    zend_update_property(data->ce, data, PROP("dataStore"), retval);
    zend_update_property(data->ce, data, PROP("status"), &status);

}

void *get_resource(zend_object *this) {
    zval * res = zend_read_property(MODULE_GL(promise_class), this, PROP("_list"), 0, NULL);
    void *resourse = zend_fetch_resource(Z_RES_P(res), "my_res", res_num);
    return resourse;
}

PHP_METHOD (Promise, then) {
    zend_fcall_info fci = empty_fcall_info;
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
    zval status_ret;
    zval * then_callback;
//    zval_get_long()
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(then_callback)ZEND_PARSE_PARAMETERS_END();
    zend_object * this = Z_OBJ_P(ZEND_THIS);
    zval * promiseFinalized = zend_read_property(MODULE_GL(promise_class), this, PROP("promiseFinalised"),
                                                 0, NULL);
    zval * status = zend_read_property(MODULE_GL(promise_class), this, PROP("status"), 0, NULL);

    GC_TRY_ADDREF (Z_OBJ_P(then_callback));
    zend_object * next_promise = create_new_promise(Z_OBJ_P(ZEND_THIS));
    short promiseFinalized_bool = Z_TYPE_INFO_P(promiseFinalized);
    short status_val = Z_LVAL_P(OBJ_PROP_NUM(Z_OBJ_P(status), 1));
    zend_update_property(MODULE_GL(promise_class), this, PROP("_then"), then_callback);
    zend_fcall_info_init(then_callback, 0, &fci, &fcc, NULL, NULL);

    if (promiseFinalized_bool == IS_TRUE) {
        printf("I am finalized");
    } else if (status_val == Resolved) {//WORKS IN IN SYNC STACK IS RESOLVED ALREADY
        zval retval;
        puts("running idle");
        zval * data = zend_read_property(MODULE_GL(promise_class), this, PROP("dataStore"), 0, NULL);
        uv_cb_type then_cb_s = {0};
        init_cb(&fci, &fcc, &then_cb_s);
        LOG("Then call back is called");
        call_php_fn(&then_cb_s, 1, data, &retval, "then");
        put_data_in_new_promise(next_promise, &retval);
        zend_update_property(MODULE_GL(promise_class), next_promise, PROP("status"), status);

    } else if (status_val == Pending) {
        puts("Setting new prepare");
        uv_prepare_t *idleHandle = emalloc(sizeof(uv_prepare_t));
        uv_prepare_init(MODULE_GL(loop), idleHandle);
        then_t *handle_data = emalloc(sizeof(then_t));
        init_cb(&fci, &fcc, &handle_data->then_cb);
        puts("Setting new prepare5");
        handle_data->this = this;
        handle_data->then_retval = return_value;
//        GC_TRY_ADDREF(this);
        LOG("Setting then prepare...\n");
        idleHandle->data = handle_data;
        puts("Setting new prepare8");
        uv_prepare_start(idleHandle, then_cb);
        puts("Setting new prepare9");
    }
    void *resourse = get_resource(this);
    GC_TRY_ADDREF(this);
    GC_TRY_ADDREF(next_promise);
    add_promise_to_list(resourse, this, next_promise);
    RETURN_OBJ(next_promise);
}

void catch_cb(uv_idle_t *handle) {

    then_t *data = handle->data;

    zval * promiseFinalized = zend_read_property(data->this->ce, data->this, PROP("promiseFinalised"), 0, NULL);
    zval * status = zend_read_property(data->this->ce, data->this, PROP("status"), 0, NULL);
    short promiseFinalized_bool = Z_TYPE_INFO_P(promiseFinalized);
    short status_val = Z_LVAL_P(OBJ_PROP_NUM(Z_OBJ_P(status), 1));
    uv_idle_stop(handle);
    if (promiseFinalized_bool == IS_TRUE) {
        printf("Promise is finalized already cb new idle");

        efree(data);
        efree(handle);
    } else if (status_val == Rejected) {
        printf("runing cb  new idle");
        zval * data_store = zend_read_property(data->this->ce, data->this, PROP("dataStore"), 0, NULL);

        zval retval;
        LOG("Catch call back is called");
        call_php_fn(&data->then_cb, 1, data_store, &retval, "catch");
        uv_idle_stop(handle);
        efree(data);
        efree(handle);
    } else if (status_val == Resolved) {
        uv_idle_stop(handle);
        efree(data);
        efree(handle);
    }
}

PHP_METHOD (Promise, catch
) {
    zend_fcall_info fci = empty_fcall_info;
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_FUNC(fci, fcc)ZEND_PARSE_PARAMETERS_END();

    zval * promiseFinalized = zend_read_property(MODULE_GL(promise_class), Z_OBJ_P(ZEND_THIS), PROP("promiseFinalised"),
                                                 0, NULL);
    zval * status = zend_read_property(MODULE_GL(promise_class), Z_OBJ_P(ZEND_THIS), PROP("status"), 0, NULL);
    short promiseFinalized_bool = Z_TYPE_INFO_P(promiseFinalized);
    short status_val = Z_LVAL_P(OBJ_PROP_NUM(Z_OBJ_P(status), 1));

    if (promiseFinalized_bool == IS_TRUE) {
        printf("Promise is finalized already cb new idle");
    } else if (status_val == Rejected) {
        zval retval;
        printf("running catch idle");
        zval * data = zend_read_property(MODULE_GL(promise_class), Z_OBJ_P(ZEND_THIS), PROP("dataStore"), 0,
                                         NULL);
        uv_cb_type then_cb_s = {0};
        init_cb(&fci, &fcc, &then_cb_s);
        LOG("Catch call back is called");
        call_php_fn(&then_cb_s, 1, data, &retval, "catch");

    } else if (status_val == Pending) {
        printf("setting new catch idle");
        uv_idle_t *idleHandle = emalloc(sizeof(uv_idle_t));
        uv_idle_init(MODULE_GL(loop), idleHandle);
        then_t *handle_data = emalloc(sizeof(then_t));
        init_cb(&fci, &fcc, &handle_data->then_cb);
        handle_data->this = Z_OBJ_P(ZEND_THIS);
        LOG("Setting catch idle ...\n");
        idleHandle->data = handle_data;
        uv_idle_start(idleHandle, catch_cb);
    }
}

void finally_cb(uv_idle_t *handle) {

    then_t *data = handle->data;

    zval * promiseFinalized = zend_read_property(data->this->ce, data->this, PROP("promiseFinalised"), 0, NULL);
    zval * status = zend_read_property(data->this->ce, data->this, PROP("status"), 0, NULL);
    short promiseFinalized_bool = Z_TYPE_INFO_P(promiseFinalized);
    short status_val = Z_LVAL_P(OBJ_PROP_NUM(Z_OBJ_P(status), 1));
    uv_idle_stop(handle);
    if (promiseFinalized_bool == IS_FALSE && status_val != Pending) {
        printf("I am finalized");
        zval retval;
        call_php_fn(&data->then_cb, 0, NULL, &retval, "finally");
        zend_update_property_bool(data->this->ce, data->this, PROP("promiseFinalised"), 1);
        uv_idle_stop(handle);
        efree(data);
        efree(handle);
    }
}

PHP_METHOD (Promise, finally) {
    zend_fcall_info fci = empty_fcall_info;
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_FUNC(fci, fcc)ZEND_PARSE_PARAMETERS_END();

    zval retval;
    zval * promiseFinalized = zend_read_property(MODULE_GL(promise_class), Z_OBJ_P(ZEND_THIS), PROP("promiseFinalised"),
                                                 0, NULL);
    zval * status = zend_read_property(MODULE_GL(promise_class), Z_OBJ_P(ZEND_THIS), PROP("status"), 0, NULL);
    short promiseFinalized_bool = Z_TYPE_INFO_P(promiseFinalized);
    short status_val = Z_LVAL_P(OBJ_PROP_NUM(Z_OBJ_P(status), 1));

    if (promiseFinalized_bool == IS_FALSE && status_val != Pending) {
        printf("I am finalized");
        uv_cb_type then_cb_s = {0};
        init_cb(&fci, &fcc, &then_cb_s);
        LOG("Finally call back is called");
        call_php_fn(&then_cb_s, 0, NULL, &retval, "finally");

    } else if (status_val == Pending) {
        printf("setting finally idle");
        uv_idle_t *idleHandle = emalloc(sizeof(uv_idle_t));
        uv_idle_init(MODULE_GL(loop), idleHandle);
        then_t *handle_data = emalloc(sizeof(then_t));
        init_cb(&fci, &fcc, &handle_data->then_cb);
        handle_data->
                this = Z_OBJ_P(ZEND_THIS);
//        fill_idle_handle_with_data(idleHandle, &fci, &fcc);
        LOG("Setting finally idle ...\n");
        idleHandle->
                data = handle_data;
        uv_idle_start(idleHandle, finally_cb
        );
    }
}


//
ZEND_BEGIN_ARG_INFO(arginfo_promise_construct, 0)
                ZEND_ARG_TYPE_INFO(0, closure, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(arginfo_promise_resolve, 0)
                ZEND_ARG_TYPE_INFO(0, data, IS_MIXED, 1)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(arginfo_promise_reject, 0)
                ZEND_ARG_TYPE_INFO(0, reason, IS_MIXED, 1)
ZEND_END_ARG_INFO()


static const zend_function_entry class_Promise_methods[] = {
        ZEND_ME(Promise, __construct, arginfo_promise_construct, ZEND_ACC_PUBLIC)
        ZEND_ME(Promise, resolve, arginfo_promise_resolve, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        ZEND_ME(Promise, resolved, arginfo_promise_resolve, ZEND_ACC_PRIVATE)
        ZEND_ME(Promise, reject, arginfo_promise_reject, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        ZEND_ME(Promise, rejected, arginfo_promise_reject, ZEND_ACC_PRIVATE)
        ZEND_ME(Promise, then, arginfo_promise_construct, ZEND_ACC_PUBLIC)
        ZEND_ME(Promise, catch, arginfo_promise_construct, ZEND_ACC_PUBLIC)
        ZEND_ME(Promise, finally, arginfo_promise_construct, ZEND_ACC_PUBLIC)
        PHP_FE_END
};

zend_class_entry *register_class_Promise(void) {
    zend_class_entry ce;
    res_num = zend_register_list_destructors_ex(my_res_dtor, NULL, "my_list", 1);
    INIT_CLASS_ENTRY(ce, "Promise", class_Promise_methods);
    MODULE_GL(promise_class) = zend_register_internal_class_ex(&ce, NULL);


//    zend_declare_class_constant_long(promise_class_entry,
//                                     "DEFAULT_FACTOR", sizeof("DEFAULT_FACTOR")-1, DEFAULT_SCALE_FACTOR);
//    zend_declare_property_long(promise_class_entry,
//                               "status", sizeof("status")-1, DEFAULT_SCALE_FACTOR, ZEND_ACC_PRIVATE);
//    zval property_service_default_value;
//    ZVAL_NULL(&property_service_default_value);
//    zend_string *property_service_name = zend_string_init("service", sizeof("service") - 1, 1);
//    zend_declare_property_ex(class_entry, property_service_name, &property_service_default_value, ZEND_ACC_PRIVATE, NULL);
//    zend_string_release(property_service_name);

//    zend_string * property__status_class_PromiseStatus = zend_string_init("PromiseStatus", sizeof("PromiseStatus") - 1,
//
//                                                                          /**   status  **/1);
//    zend_declare_property_long(FILE_IO_GLOBAL(promise_class), PROP("status"), 0, ZEND_ACC_PRIVATE);
    /**   $status  **/
    zval property___status_default_value;
    ZVAL_NULL(&property___status_default_value);
    register_class_property(MODULE_GL(promise_class), PROP("status"), &property___status_default_value,
                            ZEND_ACC_PRIVATE, PROP("PromiseStatus"));


    /**   $promiseFinalised  **/
    zend_declare_property_bool(MODULE_GL(promise_class), PROP("promiseFinalised"), 0,
                               ZEND_ACC_PRIVATE);

    /**   $dataStore  **/
    zval property___dataStore_default_value;
    ZVAL_NULL(&property___dataStore_default_value);
    register_property(MODULE_GL(promise_class), PROP("dataStore"), &property___dataStore_default_value,
                      ZEND_ACC_PRIVATE, MAY_BE_ANY);
    /**   $_then  **/
    zval property____internal_default_value;
    ZVAL_NULL(&property____internal_default_value);
    register_property(MODULE_GL(promise_class), PROP("_then"), &property____internal_default_value,
                      ZEND_ACC_PRIVATE, MAY_BE_ANY);

    /**   $closure  **/
    zval prop___closure_default_value;
    ZVAL_NULL(&prop___closure_default_value);
    register_class_property(MODULE_GL(promise_class), PROP("closure"), &prop___closure_default_value,
                            ZEND_ACC_PRIVATE, PROP("Closure"));

//    zend_string * prop__closure_class_Closure = zend_string_init("Closure", sizeof("Closure") - 1, 1);
//    zend_string * prop__closure_name = zend_string_init("closure", sizeof("closure") - 1,
    //       1);
//    zend_declare_typed_property(FILE_IO_GLOBAL(promise_class), prop__closure_name, &prop___closure_default_value,
//                                ZEND_ACC_PUBLIC, NULL,
//                                (zend_type) ZEND_TYPE_INIT_CLASS(prop__closure_class_Closure, 0, MAY_BE_CALLABLE));
//    zend_string_release(prop__closure_name);

    return MODULE_GL(promise_class);
}
