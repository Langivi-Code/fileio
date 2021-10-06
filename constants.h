//
// Created by user on 22.09.21.
//
#define HANDLE_MAP_SIZE 1024

#include "string.h"
#include "stdlib.h"
#ifndef LOG
#define LOG(fmt, arg...)    \
if(getenv("ENV")!=NULL&&strcmp(getenv("ENV"),"DEV") == 0) printf("DEBUG - " LOG_TAG "\t::\t" fmt "\n", ##arg);
#endif
