#include <stdio.h>
#include <unistd.h>
#include "rtos.h"
#include "main.h"

void sensor_task(void *arg) {

    (void)arg;

    while (1) {

        rtos_mutex_lock(&buf_mutex);
        printf("[Sensor] Reading sensor data...\n");
        fflush(stdout);
        rtos_mutex_unlock(&buf_mutex);
        rtos_sleep(50000); // 50 ms

    }

}

void comm_task(void *arg) {

    (void)arg;

    while (1) {

        rtos_mutex_lock(&buf_mutex);
        printf("[Comm] Transmitting data...\n");
        fflush(stdout);
        rtos_mutex_unlock(&buf_mutex);
        rtos_sleep(80000); // 80 ms

    }

}

void background_task(void *arg) {

    (void)arg;

    while (1) {

        printf("[Background] Low-priority task running...\n");
        fflush(stdout);
        rtos_sleep(100000); // 100 ms

    }

}

