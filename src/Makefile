CFLAGS+=-Wall -std=c99 -pedantic
LDLIBS+=-lrt

.PHONY: clean all

all: fstat

fstat: main.o interrupts.o cpu_stats.o mem_stats.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

main.o: main.c interrupts.h cpu_stats.h

cpu_stats.o: cpu_stats.c cpu_stats.h

mem_stats.o: mem_stats.c mem_stats.h

interrupts.o: interrupts.c interrupts.h

clean:
	$(RM) fstat main.o interrupts.o cpu_stats.o mem_stats.o
