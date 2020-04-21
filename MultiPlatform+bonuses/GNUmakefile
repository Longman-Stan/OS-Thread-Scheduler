CC = gcc
CFLAGS = -fPIC -Wall
build: libscheduler.so

bonus_tests: bonus_test_exec bonus_test_wait config_params

bonus_test_exec: libscheduler.so bonus_test_exec.c
	$(CC) -Wall -o bonus_test_exec bonus_test_exec.c -L . -lscheduler -lpthread -lrt

bonus_test_wait: libscheduler.so bonus_test_wait.c
	$(CC) -Wall -o bonus_test_wait bonus_test_wait.c -L . -lscheduler -lpthread -lrt

config_params: config_params.c
	$(CC) -Wall -o config_params config_params.c so_sched_initializer.c

libscheduler.so: so_scheduler.o queue.o hash.o so_thread.o so_sched_initializer.o so_logger.o
	$(CC) -shared -o libscheduler.so so_scheduler.o queue.o hash.o so_thread.o so_logger.o so_sched_initializer.o
	
hash.o: queue.o hash.c
	$(CC) $(CFLAGS) -c hash.c -o hash.o

queue.o: so_thread.o queue.c
	$(CC) $(CFLAGS) -c queue.c -o queue.o

so_thread.o: so_scheduler.o so_thread.c
	$(CC) $(CFLAGS) -c so_thread.c -o so_thread.o

so_logger.o: so_scheduler.o so_logger.c
	$(CC) $(CFLAGS) -c so_logger.c -o so_logger.o

so_scheduler.o: so_scheduler.c
	$(CC) $(CFLAGS) -c so_scheduler.c -o so_scheduler.o

so_sched_initializer.o: so_sched_initializer.c
	$(CC) $(CFLAGS) -c so_sched_initializer.c -o so_sched_initializer.o

clean:
	rm so_scheduler.o queue.o hash.o so_thread.o so_logger.o so_sched_initializer.o libscheduler.so

clean_bonus:
	rm bonus_test_exec bonus_test_wait config_params