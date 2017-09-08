CC=gcc

CFLAGS=-I. --std=gnu99 -I./include


HEAD_OBJ=src/head.o
TAIL_OBJ=src/tail.o
WC_OBJ=src/wc.o
TRUE_OBJ=src/true.o
FALSE_OBJ=src/false.o
SLEEP_OBJ=src/sleep.o
YES_OBJ=src/yes.o
DATE_OBJ=src/date.o
NTPC_OBJ=src/ntpc.o
UPTIME_OBJ=src/uptime.o
PING_OBJ=src/ping.o
ALRM_OBJ=src/alarms.o
FILE_OBJ=src/file.o

LDFLAGS=
LDLIBS=


head: $(HEAD_OBJ)
	$(CC) -o $@ $(HEAD_OBJ) $(CFLAGS) $(LDFLAGS) $(LDLIBS)
	rm -f src/head.o

tail: $(TAIL_OBJ)
	$(CC) -o $@ $(TAIL_OBJ) $(CFLAGS) $(LDFLAGS) $(LDLIBS)
	rm -f src/tail.o

wc: $(WC_OBJ)
	$(CC) -o $@ $(WC_OBJ) $(CFLAGS) $(LDFLAGS) $(LDLIBS)
	rm -f src/wc.o

true: $(TRUE_OBJ)
	$(CC) -o $@ $(TRUE_OBJ) $(CFLAGS) $(LDFLAGS) $(LDLIBS)
	rm -f src/true.o

false: $(FALSE_OBJ)
	$(CC) -o $@ $(FALSE_OBJ) $(CFLAGS) $(LDFLAGS) $(LDLIBS)
	rm -f src/false.o

sleep: $(SLEEP_OBJ)
	$(CC) -o $@ $(SLEEP_OBJ) $(CFLAGS) $(LDFLAGS) $(LDLIBS)
	rm -f src/sleep.o

yes: $(YES_OBJ)
	$(CC) -o $@ $(YES_OBJ) $(CFLAGS) $(LDFLAGS) $(LDLIBS)
	rm -f src/yes.o

date: $(DATE_OBJ)
	$(CC) -o $@ $(DATE_OBJ) $(CFLAGS) $(LDFLAGS) $(LDLIBS)
	rm -f src/date.o

ntpc: $(NTPC_OBJ)
	$(CC) -o $@ $(NTPC_OBJ) $(CFLAGS) $(LDFLAGS) $(LDLIBS)
	rm -f src/ntpc.o

ping: $(PING_OBJ)
	$(CC) -o $@ $(PING_OBJ) $(CFLAGS) $(LDFLAGS) $(LDLIBS)
	rm -f src/ping.o

uptime: $(UPTIME_OBJ)
	$(CC) -o $@ $(UPTIME_OBJ) $(CFLAGS) $(LDFLAGS) $(LDLIBS)
	rm -f src/uptime.o

alarms: $(ALRM_OBJ)
	$(CC) -o $@ $(ALRM_OBJ) $(CFLAGS) $(LDFLAGS) $(LDLIBS)
	rm -f src/alarms.o

file: $(FILE_OBJ)
	$(CC) -o $@ $(FILE_OBJ) $(CFLAGS) $(LDFLAGS) $(LDLIBS)
	rm -f src/file.o

all: head tail wc true false sleep yes date ntpc uptime ping alarms file

clean:
	rm -f src/*.o head tail wc true false sleep yes date ntpc uptime ping alarms file
