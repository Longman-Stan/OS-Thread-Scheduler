#pragma once
#include "so_scheduler.h"
#include "hash.h"

struct so_scheduler {
	int time_quantum;
	unsigned int supported_io_num;
	MUTEX scheduler_opperation;
	struct queue **ready_queue;
	struct queue **wait_queue;
	struct so_thread *current_thread;
	struct hash *thread_hash;
	struct queue *all_threads;
};
