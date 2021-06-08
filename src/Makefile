CFLAGS+=-g -Wall -std=c99 -pedantic
LDFLAGS+=-g
LDLIBS+=-lrt -lm

.PHONY: clean all

all: statsh

statsh: main.o config.o signals.o cpu_stats.o disk_stats.o mem_stats.o net_stats.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

main.o: main.c config.h signals.h cpu_stats.h disk_stats.h mem_stats.h net_stats.h

config.o: config.c config.h

cpu_stats.o: cpu_stats.c cpu_stats.h

disk_stats.o: disk_stats.c disk_stats.h

mem_stats.o: mem_stats.c mem_stats.h

net_stats.o: net_stats.c net_stats.h config.h

signals.o: signals.c signals.h

clean:
	$(RM) statsh main.o config.o signals.o cpu_stats.o disk_stats.o mem_stats.o net_stats.o
