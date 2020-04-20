CC = gcc
CFLAGS = -fPIC -Wall -g
build: libscheduler.so

libscheduler.so: so_scheduler.o queue.o hash.o so_thread.o
	$(CC) -g -shared -o libscheduler.so so_scheduler.o queue.o hash.o so_thread.o

so_sched: so_scheduler.o queue.o hash.o so_thread.o
	$(CC) $(CFLAGS) -o so_sched so_scheduler.o queue.o hash.o so_thread.o -lpthread -lrt

hash.o: queue.o hash.c
	$(CC) $(CFLAGS) -c hash.c -o hash.o

queue.o: so_thread.o queue.c
	$(CC) $(CFLAGS) -c queue.c -o queue.o

so_thread.o: so_scheduler.o so_thread.c
	$(CC) $(CFLAGS) -c so_thread.c -o so_thread.o

so_scheduler.o: so_scheduler.c
	$(CC) $(CFLAGS) -c so_scheduler.c -o so_scheduler.o

clean:
	rm so_scheduler.o queue.o hash.o so_thread.o libscheduler.so