
#pragma once
#include "so_thread.h"
#include "so_scheduler.h"
#include "queue.h"
#define HASH_VALUE_NOT_FOUND NULL
#define HASH_SUCCESS 0

struct hash {
	int size;
	struct queue **queues;
};

struct hash *initialize_hash(int size);

void destroy_hash(struct hash *hash);

int hash_insert(struct hash *hash, struct so_thread *thread);

struct so_thread *hash_get_value(struct hash *hash, tid_t tid);

void hash_delete(struct hash *hash, tid_t tid);
