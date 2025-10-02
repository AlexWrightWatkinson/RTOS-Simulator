CC = gcc
CFLAGS = -Wall -O2 -g
OBJS = main.o rtos.o demo.o

all: rtos-sim

rtos-sim: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) rtos-sim

