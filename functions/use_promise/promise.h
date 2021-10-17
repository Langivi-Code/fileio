//
// Created by admin on 12.10.2021.
//


typedef struct {
    char *name;
    short length;
} field_def;


static field_def status_f = {"status", sizeof("status") - 1};
static field_def dataStore_f = {"dataStore", sizeof("dataStore") - 1};
static field_def promiseFinalized_f = {"promiseFinalised", sizeof("promiseFinalised") - 1,};


