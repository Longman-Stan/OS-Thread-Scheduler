#include "so_thread.h"

void destroy_so_thread(struct so_thread *thread)
{
	SEM_DESTROY(&thread->semaphore);
	free(thread);
}

#ifdef __linux__

static int init_semaphore(struct so_thread *thread, SEM_T *semaphore)
{
	return sem_init(semaphore, 0, 0);
}

#elif defined(_WIN32)

static HANDLE init_semaphore(struct so_thread *thread, SEM_T *semaphore)
{
	*semaphore = CreateSemaphore(
						NULL,
						0,
						1,
						NULL);
	return *semaphore;
}

#else
#error "Unknown platform"
#endif

struct so_thread *initialize_so_thread(unsigned int priority,
				       so_handler *handler)
{
	struct so_thread *thread;

	thread = (struct so_thread *)malloc(sizeof(struct so_thread));
	if (thread == NULL)
		return FAILED_THREAD_INIT;

	if (init_semaphore(thread, &thread->semaphore) == SEM_FAIL) {
		free(thread);
		return (struct so_thread *)BARRIER_INIT_FAILURE;
	}
	if (init_semaphore(thread, &thread->fork_sem) == SEM_FAIL) {
		SEM_DESTROY(&thread->semaphore);
		free(thread);
		return (struct so_thread *)BARRIER_INIT_FAILURE;
	}

	thread->status = READY;
	thread->time_quantum = 1;
	thread->priority = priority;
	thread->handler = handler;
	return thread;
}
