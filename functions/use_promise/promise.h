//
// Created by admin on 12.10.2021.
//
#ifndef PROMISE
#define PROMISE
typedef struct then_struct {
    uv_cb_type then_cb;
    zend_object * this;
} then_t;

enum Promise{
    Pending,
    Resolved,
    Rejected
};
zval * promisify(zval * constructor_closure);
#endif
