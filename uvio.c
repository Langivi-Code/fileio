//
// Created by portable on 26.02.21.
//
#include <uv.h>
#include "php_fileio.h"
//uv_loop_t* loop;
uv_fs_t read_req, close_req, open_req;
char buf[1024];

cbf read_cgf;
void read_cb(uv_fs_t* req) {
    int result = req->result;

    if (result == -1) {
        fprintf(stderr, "Error at reading file: %s.\n",
                uv_strerror(uv_last_error(loop)));
    }

    uv_fs_req_cleanup(req);
    uv_fs_close(loop, &close_req, open_req.result, NULL);
}

void open_cb(uv_fs_t* req) {
    int result = req->result;
    char * output;
    if (result == -1) {
        sprintf(output, "Error at opening file: %s\n",
                uv_strerror(uv_last_error(loop)));
    }
    if (read_cgf != NULL){
        uv_fs_read(loop, &read_req, result, buf, sizeof(buf), -1, read_cb);
        read_cgf(buf, output);
    }
    uv_fs_req_cleanup(req);

    printf("Successfully opened file.\n");
}

void async_read(char * file, cbf clb){
    uv_loop_init(loop);
    read_cgf = clb;
    uv_fs_cb uv_fs_c = (uv_fs_cb)clb;
  int r =  uv_fs_open(loop, &open_req, file, O_RDONLY, S_IRUSR, uv_fs_c);
  if (r){

  }
}