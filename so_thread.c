#include "so_thread.h"

void destroy_so_thread(struct so_thread *thread)
{
	sem_destroy(&thread->semaphore);
	free(thread);
}

struct so_thread *initialize_so_thread(unsigned int priority,
				       so_handler *handler)
{
	struct so_thread *thread;

	thread = (struct so_thread *)malloc(sizeof(struct so_thread));
	if (thread == NULL) {
		return FAILED_THREAD_INIT;
	}
	if (sem_init(&thread->semaphore, 0, 0)) {
		free(thread);
		return (struct so_thread *)BARRIER_INIT_FAILURE;
	}
	if (sem_init(&thread->fork_sem, 0, 0)) {
		sem_destroy(&thread->semaphore);
		free(thread);
		return (struct so_thread *)BARRIER_INIT_FAILURE;
	}

	thread->status = READY;
	thread->time_quantum = 1;
	thread->priority = priority;
	thread->handler = handler;
	return thread;
}