#pragma once
#include "so_thread.h"
#include <stdlib.h>
#include <string.h>
#define MALLOC_NULL_POINTER 12
#define QUEUE_SUCCESS 0
#define INVALID_INFO NULL

struct q_node {
	struct so_thread *info;
	struct q_node *next;
};

struct queue {
	struct q_node *beginning, *end;
};

struct queue *initialize_queue();

int destroy_queue(struct queue *queue);

int insert_into_queue(struct so_thread *thread, struct queue *queue);

struct so_thread *pop_queue(struct queue *queue);

