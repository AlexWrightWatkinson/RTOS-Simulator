#ifndef RTOS_H
#define RTOS_H

#include <ucontext.h>

#define MAX_TASKS 16

typedef enum {

    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_EXITED

} task_state_t;

typedef struct rtos_task {

    int id;
    int priority;
    int base_priority;
    task_state_t state;
    ucontext_t ctx;
    void *stack;
    struct rtos_task *next;
    unsigned long wakeup_tick;

} rtos_task_t;

typedef struct rtos_mutex {

    struct rtos_task *owner;
    struct rtos_task *waiters;

} rtos_mutex_t;

// core rtos api
int rtos_init(void);
int rtos_task_create(void (*fn)(void *), void *arg, int priority);
void rtos_start(void);
void rtos_yield(void);
void rtos_sleep(unsigned int ms);

// mutex api
void rtos_mutex_init(rtos_mutex_t *m);
void rtos_mutex_lock(rtos_mutex_t *m);
void rtos_mutex_unlock(rtos_mutex_t *m);

#endif

