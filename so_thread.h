#pragma once
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include "so_scheduler.h"

#define FAILED_THREAD_INIT NULL

enum state {
	READY = 0,
	RUNNING = 1,
	WAITING = 2,
	TERMINATED = 3
};

struct so_thread {
	sem_t semaphore;
	sem_t fork_sem;
	tid_t tid;
	enum state status;
	unsigned int time_quantum;
	unsigned int priority;
	so_handler *handler;
};

struct so_thread *initialize_so_thread(
	unsigned int priority,
	so_handler *handler);

void destroy_so_thread(struct so_thread *thread);
