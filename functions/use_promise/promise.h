//
// Created by admin on 12.10.2021.
//

typedef struct then_struct {
    uv_cb_type then_cb;
    zend_object * this;
    zval * retval;
} then_t;

enum Promise{
    Pending,
    Resolved,
    Rejected
};