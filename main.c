#include <stdio.h>
#include "rtos.h"
#include "main.h"
#include "demo.h"

rtos_mutex_t buf_mutex;

int main(void) {

    setvbuf(stdout, NULL, _IONBF, 0); // no buffering mode
    printf("[RTOS] Initializing...\n");
    rtos_mutex_init(&buf_mutex);

    rtos_task_create(sensor_task, NULL, 3);
    rtos_task_create(comm_task, NULL, 2);
    rtos_task_create(background_task, NULL, 1);

    printf("[RTOS] Starting scheduler...\n");
    rtos_start();
    return 0;

}

