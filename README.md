# RTOS simulator

A lightweight **RTOS (Real-Time Operating System) simulator** implemented in C, designed to run purely on Linux without the need for hardware timers. This simulates concepts such as **scheduling**, **context switching**, **mutex synchronization**, and **tick-based sleeping** -- similar to embedded RTOS kernels e.g. FreeRTOS or Zephyr.

## Components

- **Preemptive scheduling** using timer interrupts ('SIGALRM')
- **Task creation** with associated priorities
- **Tick-based sleeping** ('rtos_sleep') for periodic tasks
- **Mutex** for synchronization

