CC = gcc
CFLAGS = -fPIC -Wall
build: libscheduler.so

bonus_tests: bonus_test_exec bonus_test_wait config_params libscheduler2.so

bonus_test_exec: libscheduler2.so bonus_test_exec.c
	$(CC) -Wall -o bonus_test_exec bonus_test_exec.c -L . -lscheduler2 -lpthread -lrt

bonus_test_wait: libscheduler2.so bonus_test_wait.c
	$(CC) -Wall -o bonus_test_wait bonus_test_wait.c -L . -lscheduler2 -lpthread -lrt

libscheduler2.so: so_scheduler2.o queue.o hash.o so_thread.o so_sched_initializer.o so_logger.o
	$(CC) -shared -o libscheduler2.so so_scheduler2.o queue.o hash.o so_thread.o so_logger.o so_sched_initializer.o

so_scheduler2.o: so_scheduler.c
	$(CC) $(CFLAGS) -D BONUS -c so_scheduler.c -o so_scheduler2.o

config_params: config_params.c
	$(CC) -Wall -o config_params config_params.c so_sched_initializer.c

libscheduler.so: so_scheduler.o queue.o hash.o so_thread.o so_sched_initializer.o so_logger.o
	$(CC) -shared -o libscheduler.so so_scheduler.o queue.o hash.o so_thread.o so_logger.o so_sched_initializer.o
	
hash.o: hash.c
	$(CC) $(CFLAGS) -c hash.c -o hash.o

queue.o: queue.c
	$(CC) $(CFLAGS) -c queue.c -o queue.o

so_thread.o: so_thread.c
	$(CC) $(CFLAGS) -c so_thread.c -o so_thread.o

so_logger.o: so_logger.c
	$(CC) $(CFLAGS) -c so_logger.c -o so_logger.o

so_scheduler.o: so_scheduler.c
	$(CC) $(CFLAGS) -c so_scheduler.c -o so_scheduler.o

so_sched_initializer.o: so_sched_initializer.c
	$(CC) $(CFLAGS) -c so_sched_initializer.c -o so_sched_initializer.o

clean:
	rm so_scheduler.o queue.o hash.o so_thread.o so_logger.o so_sched_initializer.o libscheduler.so

clean_bonus:
	rm bonus_test_exec bonus_test_wait config_params *.so *.o