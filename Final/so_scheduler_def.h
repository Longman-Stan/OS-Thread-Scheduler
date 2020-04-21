#pragma once
#include "so_scheduler.h"
#include "hash.h"
#include "so_sched_initializer.h"
#include "so_logger.h"

struct so_scheduler {
	unsigned int time_quantum;
	unsigned int supported_io_num;
	MUTEX scheduler_opperation;
	struct queue **ready_queue;
	struct queue **wait_queue;
	struct so_thread *current_thread;
	struct hash *thread_hash;
	struct queue *all_threads;
	enum BOOL enable_log;
	struct so_logger *logger;
	enum BOOL use_fcfs;
};
