//
// Created by portable on 16.05.21.
//

#include <uv.h>
#include "event_bindings.h"
#include "php_fileio.h"


void init_file_io() {
    if (fileio_globals.loop == NULL) {
        fileio_globals.loop=uv_default_loop();
	}
    uv_run(fileio_globals.loop, run_mode);
}
