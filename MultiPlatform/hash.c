#include "hash.h"

struct hash *initialize_hash(int size)
{
	int i, j;
	struct hash *hash;

	hash = (struct hash *) malloc(sizeof(struct hash));
	if (hash == NULL)
		return (struct hash *)FAILED_MALLOC;

	hash->queues = (struct queue **) malloc(size * sizeof(struct queue *));
	if (hash->queues == NULL) {
		free(hash);
		return (struct hash *)FAILED_MALLOC;
	}

	for (i = size - 1; i >= 0; i--) {
		hash->queues[i] = initialize_queue();
		if (hash->queues[i] == NULL) {
			for (j = i + 1; j < size; j++)
				destroy_queue(hash->queues[i]);
			free(hash->queues);
			free(hash);
			return (struct hash *)FAILED_MALLOC;
		}
	}
	hash->size = size;
	return hash;
}

void destroy_hash(struct hash *hash)
{
	int i;

	for (i = 0; i < hash->size; i++)
		destroy_queue(hash->queues[i]);

	free(hash->queues);
	free(hash);
}

int hash_insert(struct hash *hash, struct so_thread *thread)
{
	unsigned long key;

	key = thread->tid % hash->size;
	return insert_into_queue(thread, hash->queues[key]);
}

struct so_thread *hash_get_value(struct hash *hash, tid_t tid)
{
	unsigned long key;
	struct q_node *node_crt;

	key = tid % hash->size;

	node_crt = hash->queues[key]->beginning;
	while (node_crt) {
		if (node_crt->info->tid == tid)
			return node_crt->info;
		node_crt = node_crt->next;
	}
	return NULL;
}

void hash_delete(struct hash *hash, tid_t tid)
{
	unsigned long key;
	struct q_node *node_crt, *prec;

	key = tid % hash->size;
	prec = NULL;

	node_crt = hash->queues[key]->beginning;
	while (node_crt) {
		if (node_crt->info->tid == tid) {
			if (prec == NULL) {
				if (hash->queues[key]->beginning->next == NULL)
					hash->queues[key]->end = NULL;
				hash->queues[key]->beginning = node_crt->next;
			} else
				prec->next = node_crt->next;
			free(node_crt);
			return;
		}
		prec = node_crt;
		node_crt = node_crt->next;
	}
}
