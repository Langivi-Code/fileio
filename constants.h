//
// Created by user on 22.09.21.
//
#define HANDLE_MAP_SIZE 1024

#include "stdlib.h"
#include "string.h"

#ifndef LOG
#define LOG(fmt, arg...)    \
if(strcmp(getenv("ENV"),"DEV") == 0) printf("DEBUG - " LOG_TAG "\t::\t" fmt "\n", ##arg);
#endif
