#define _GNU_SOURCE
#include "rtos.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

#define STACK_SIZE (64*1024) // 64KB stack

// globals
static rtos_task_t tasks[MAX_TASKS];
static int task_count = 0;
static rtos_task_t *current = NULL;
static ucontext_t scheduler_ctx;
static rtos_task_t *ready_head = NULL;
static int g_tick_ms = 50; // tick period (ms)
static volatile unsigned long g_tick_count = 0; // global tick counter

// ready queue
static void ready_enqueue(rtos_task_t *t) {

    t->next = NULL;

    if (!ready_head) {

        ready_head = t;
        return;

    }

    rtos_task_t *prev = NULL, *it = ready_head;

    while (it && it->priority >= t->priority) {

        prev = it;
        it = it->next;

    }

    if (!prev) {

        t->next = ready_head;
        ready_head = t;

    }

    else {

        prev->next = t;
        t->next = it;

    }

}

static void ready_remove(rtos_task_t *t) {

    rtos_task_t *prev = NULL, *it = ready_head;

    while (it) {

        if (it == t) {

            if (!prev) ready_head = it->next;
            else prev->next = it->next;
            t->next = NULL;
            return;

        }

        prev = it;
        it = it->next;

    }

}

static rtos_task_t *pick_next_ready_task(void) {

    return ready_head;

}

// scheduler tick
static void scheduler_tick(int signum) {

    (void)signum;
    g_tick_count++;

    // wake sleeping tasks on timer
    for (int i = 0; i < task_count; i++) {

        if (tasks[i].state == TASK_BLOCKED && tasks[i].wakeup_tick <= g_tick_count) {

            tasks[i].state = TASK_READY;
            ready_enqueue(&tasks[i]);

        }

    }

    // start first task
    if (!current) {

        rtos_task_t *next = pick_next_ready_task();

        if (next) {

            ready_remove(next);
            current = next;
            current->state = TASK_RUNNING;
            setcontext(&current->ctx);

        }

        return;

    }

    // preempt if higher priority task
    if (getcontext(&current->ctx) == 0) {

        rtos_task_t *next = pick_next_ready_task();

        if (next && next != current) {

            rtos_task_t *prev = current;

            if (prev->state == TASK_RUNNING) {

                prev->state = TASK_READY;
                ready_enqueue(prev);

            }

            ready_remove(next);
            current = next;
            current->state = TASK_RUNNING;
            swapcontext(&prev->ctx, &current->ctx);

        }

    } 

    else {

    }

}

// initialization
int rtos_init(void) {

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = scheduler_tick;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGALRM, &sa, NULL) < 0) {

        perror("sigaction");
        return -1;

    }

    struct itimerval tv;
    tv.it_interval.tv_sec = 0;
    tv.it_interval.tv_usec = g_tick_ms * 1000;
    tv.it_value = tv.it_interval;

    if (setitimer(ITIMER_REAL, &tv, NULL) < 0) {

        perror("setitimer");
        return -1;

    }

    return 0;

}

// task creation
static void task_trampoline(void (*fn)(void *), void *arg) {

    fn(arg);
    current->state = TASK_EXITED;
    setcontext(&scheduler_ctx);

}

int rtos_task_create(void (*fn)(void *), void *arg, int priority) {

    if (task_count >= MAX_TASKS) return -1;

    rtos_task_t *t = &tasks[task_count];
    memset(t, 0, sizeof(*t));
    t->id = task_count;
    t->priority = priority;
    t->base_priority = priority;
    t->state = TASK_READY;
    t->wakeup_tick = 0;

    t->stack = malloc(STACK_SIZE);
    if (!t->stack) return -1;

    getcontext(&t->ctx);
    t->ctx.uc_stack.ss_sp = t->stack;
    t->ctx.uc_stack.ss_size = STACK_SIZE;
    t->ctx.uc_link = &scheduler_ctx;
    makecontext(&t->ctx, (void(*)(void))task_trampoline, 2, fn, arg);

    ready_enqueue(t);
    task_count++;
    return t->id;

}

// core API
void rtos_start(void) {

    rtos_init();
    while (1) pause(); // All scheduling happens in signal handler

}

void rtos_yield(void) {

    raise(SIGALRM);

}

// sleep API
void rtos_sleep(unsigned int ms) {

    unsigned long ticks = ms / g_tick_ms;
    if (ticks == 0) ticks = 1; // min 1 tick

    current->wakeup_tick = g_tick_count + ticks;
    current->state = TASK_BLOCKED;
    rtos_yield();

}

// mutex API
void rtos_mutex_init(rtos_mutex_t *m) {

    m->owner = NULL;
    m->waiters = NULL;

}

void rtos_mutex_lock(rtos_mutex_t *m) {

    if (!m->owner) {

        m->owner = current;
        return;

    }

    // block current task and add to wait list
    current->state = TASK_BLOCKED;
    current->next = m->waiters;
    m->waiters = current;
    rtos_yield();

}

void rtos_mutex_unlock(rtos_mutex_t *m) {

    if (m->owner != current) return;
    m->owner = NULL;

    // wake available waiter
    if (m->waiters) {

        rtos_task_t *w = m->waiters;
        m->waiters = w->next;
        w->next = NULL;
        w->state = TASK_READY;
        ready_enqueue(w);

    }

}
