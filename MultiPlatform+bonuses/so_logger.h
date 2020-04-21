#pragma once
#include <stdio.h>
#include <time.h>
#include "so_scheduler.h"
#include "queue.h"

#define INVALID_LOGGER NULL

struct so_logger {
	unsigned int window_size;
	struct queue *log_queue;
	unsigned int crt_size;
	FILE *log_file;
};

struct so_logger *initialize_logger(unsigned int window_size);

void log_fork(struct so_logger *logger, tid_t thread_id,
	      tid_t child, unsigned int priority);

void log_terminate(struct so_logger *logger, tid_t thread_id);

void log_preempt(struct so_logger *logger, tid_t thread_id);

void log_exec(struct so_logger *logger, tid_t thread_id);

void log_wait(struct so_logger *logger, tid_t thread_id, unsigned int io);

void log_signal(struct so_logger *logger, tid_t thread_id, unsigned int io);

void log_error(struct so_logger *logger, const char *msg);

void flush_log(struct so_logger *logger);

void destroy_logger(struct so_logger *logger);
