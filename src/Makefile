CFLAGS+=-g -Wall -std=c99 -pedantic
LDFLAGS+=-g
LDLIBS+=-lrt -lm

.PHONY: clean all

all: fstat

fstat: main.o signals.o cpu_stats.o disk_stats.o mem_stats.o net_stats.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

main.o: main.c signals.h cpu_stats.h

cpu_stats.o: cpu_stats.c cpu_stats.h

disk_stats.o: disk_stats.c disk_stats.h

mem_stats.o: mem_stats.c mem_stats.h

net_stats.o: net_stats.c net_stats.h

signals.o: signals.c signals.h

clean:
	$(RM) fstat main.o signals.o cpu_stats.o disk_stats.o mem_stats.o net_stats.o
