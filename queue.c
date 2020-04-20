#include "queue.h"

struct queue *initialize_queue(void)
{
	struct queue *q;

	q = (struct queue *)malloc(sizeof(struct queue));
	if (q == NULL)
		return (struct queue *)MALLOC_NULL_POINTER;
	q->beginning = NULL;
	q->end = NULL;
	return q;
}

int destroy_queue(struct queue *queue)
{
	struct q_node *next_node;

	if (queue == NULL)
		return MALLOC_NULL_POINTER;
	next_node = queue->beginning;
	queue->end = NULL;
	while (next_node != NULL) {
		next_node = next_node->next;
		free(queue->beginning);
		queue->beginning = next_node;
	}
	free(queue);
	return 0;
}

int insert_into_queue(struct so_thread *thread, struct queue *queue)
{
	struct q_node *new_node;

	new_node = (struct q_node *) malloc(sizeof(struct q_node));
	if (new_node == NULL)
		return MALLOC_NULL_POINTER;
	new_node->info = thread;
	new_node->next = NULL;

	if (queue->end)
		queue->end->next  = new_node;

	if (queue->beginning == NULL)
		queue->beginning = new_node;

	queue->end = new_node;
	return QUEUE_SUCCESS;
}

struct so_thread *pop_queue(struct queue *queue)
{
	struct so_thread *result;
	struct q_node *aux_node;

	if (queue == NULL || queue->beginning == NULL)
		return INVALID_INFO;

	result = queue->beginning->info;
	aux_node = queue->beginning;
	queue->beginning = aux_node->next;

	if (queue->beginning == NULL)
		queue->end = NULL;

	free(aux_node);
	return result;
}
