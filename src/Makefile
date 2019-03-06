CC=gcc
CFLAGS=-Wall -W -O2 -fno-strict-aliasing
CLIBS=-pthread
OUT=lagscope
OBJS=lagscope.o tcpstream.o controller.o util.o latencies_stats.o main.o

$(OUT): lagscope.o tcpstream.o controller.o util.o latencies_stats.o main.o
	$(CC) $(CFLAGS) $(CLIBS) -o $(OUT) $(OBJS)
lagscope.o: lagscope.c
	$(CC) $(CFLAGS) -c lagscope.c -o lagscope.o
tcpstream.o: tcpstream.c
	$(CC) $(CFLAGS) -c tcpstream.c -o tcpstream.o
controller.o: controller.c
	$(CC) $(CFLAGS) -c controller.c -o controller.o
util.o: util.c
	$(CC) $(CFLAGS) -c util.c -o util.o
main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o
latencies_stats.o: latencies_stats.c
	$(CC) $(CFLAGS) -c latencies_stats.c -o latencies_stats.o


.PHONY: clean
clean:
	rm -rf /usr/bin/$(OUT)
	rm -rf *.o $(OUT)

.PHONY: install
install:
	cp $(OUT) /usr/bin

