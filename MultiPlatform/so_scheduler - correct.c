#include <stdio.h>
#include "queue.h"
#include "so_scheduler_def.h"

static struct so_scheduler *scheduler;

#ifdef __linux__

DECL_PREFIX int so_init(unsigned int time_quantum, unsigned int io)
{
	unsigned int i, j;

	if (scheduler)
		return SCHED_ALREADY_INITIALIZED;
	scheduler = (struct so_scheduler *) malloc(sizeof(struct so_scheduler));
	if (scheduler == NULL)
		return FAILED_MALLOC;

	scheduler->time_quantum = time_quantum;
	scheduler->supported_io_num = io;

	scheduler->thread_hash = initialize_hash(HASH_SIZE);
	if (scheduler->thread_hash == (struct hash *)FAILED_MALLOC) {
		free(scheduler);
		return FAILED_MALLOC;
	}

	if (pthread_mutex_init(&scheduler->scheduler_opperation, NULL)) {
		destroy_hash(scheduler->thread_hash);
		free(scheduler);
		return MUTEX_INIT_FAILURE;
	}

	scheduler->all_threads = initialize_queue();
	if (scheduler->all_threads == (struct queue *)MALLOC_NULL_POINTER) {
		destroy_hash(scheduler->thread_hash);
		pthread_mutex_destroy(&scheduler->scheduler_opperation);
		free(scheduler);
		return FAILED_MALLOC;
	}

	scheduler->wait_queue = (struct queue **) malloc
						(io * sizeof(struct queue *));

	if (scheduler->wait_queue == NULL) {
		destroy_hash(scheduler->thread_hash);
		pthread_mutex_destroy(&scheduler->scheduler_opperation);
		destroy_queue(scheduler->all_threads);
		free(scheduler);
		return FAILED_MALLOC;
	}

	scheduler->ready_queue = (struct queue **) malloc(
				(SO_MAX_PRIO + 1) * sizeof(struct queue *));
	if (scheduler->ready_queue == NULL) {
		free(scheduler->wait_queue);
		destroy_hash(scheduler->thread_hash);
		pthread_mutex_destroy(&scheduler->scheduler_opperation);
		destroy_queue(scheduler->all_threads);
		free(scheduler);
		return FAILED_MALLOC;
	}

	for (i = 0; i < io; i++) {
		scheduler->wait_queue[i] = initialize_queue();
		if (scheduler->wait_queue[i]
			== (struct queue *)MALLOC_NULL_POINTER) {
			for (j = i + 1; j < io; j++)
				destroy_queue(scheduler->wait_queue[i]);
			free(scheduler->wait_queue);
			destroy_hash(scheduler->thread_hash);
			pthread_mutex_destroy(&scheduler->scheduler_opperation);
			destroy_queue(scheduler->all_threads);
			free(scheduler);
			return FAILED_MALLOC;
		}
	}

	for (i = 0; i <= SO_MAX_PRIO; i++) {
		scheduler->ready_queue[i] = initialize_queue();
		if (scheduler->ready_queue[i]
			== (struct queue *)MALLOC_NULL_POINTER) {
			for (j = i + 1; j <= SO_MAX_PRIO; j++)
				destroy_queue(scheduler->ready_queue[i]);
			free(scheduler->ready_queue);
			for (j = 0; j <= scheduler->supported_io_num; j++)
				destroy_queue(scheduler->wait_queue[i]);
			free(scheduler->wait_queue);
			destroy_hash(scheduler->thread_hash);
			pthread_mutex_destroy(&scheduler->scheduler_opperation);
			destroy_queue(scheduler->all_threads);
			free(scheduler);
			return FAILED_MALLOC;
		}
	}
	scheduler->current_thread = NULL;
	return SCHED_INITIALIZE_SUCCESS;
}

#elif defined(_WIN32)

DECL_PREFIX int so_init(unsigned int time_quantum, unsigned int io)
{
	unsigned int i, j;
	BOOL crit_section_ret;

	if (scheduler)
		return SCHED_ALREADY_INITIALIZED;
	scheduler = (struct so_scheduler *) malloc(sizeof(struct so_scheduler));
	if (scheduler == NULL)
		return FAILED_MALLOC;

	scheduler->time_quantum = time_quantum;
	scheduler->supported_io_num = io;

	scheduler->thread_hash = initialize_hash(HASH_SIZE);
	if (scheduler->thread_hash == (struct hash *)FAILED_MALLOC) {
		free(scheduler);
		return FAILED_MALLOC;
	}

	crit_section_ret = InitializeCriticalSectionAndSpinCount(
		&scheduler->scheduler_opperation,
		0x400);
	if (crit_section_ret != TRUE) {
		destroy_hash(scheduler->thread_hash);
		free(scheduler);
		return MUTEX_INIT_FAILURE;
	}

	scheduler->all_threads = initialize_queue();
	if (scheduler->all_threads == (struct queue *)MALLOC_NULL_POINTER) {
		destroy_hash(scheduler->thread_hash);
		DeleteCriticalSection(&scheduler->scheduler_opperation);
		free(scheduler);
		return FAILED_MALLOC;
	}

	scheduler->wait_queue = (struct queue **) malloc
						(io * sizeof(struct queue *));

	if (scheduler->wait_queue == NULL) {
		destroy_hash(scheduler->thread_hash);
		DeleteCriticalSection(&scheduler->scheduler_opperation);
		destroy_queue(scheduler->all_threads);
		free(scheduler);
		return FAILED_MALLOC;
	}

	scheduler->ready_queue = (struct queue **) malloc(
				(SO_MAX_PRIO + 1) * sizeof(struct queue *));
	if (scheduler->ready_queue == NULL) {
		free(scheduler->wait_queue);
		destroy_hash(scheduler->thread_hash);
		DeleteCriticalSection(&scheduler->scheduler_opperation);
		destroy_queue(scheduler->all_threads);
		free(scheduler);
		return FAILED_MALLOC;
	}

	for (i = 0; i < io; i++) {
		scheduler->wait_queue[i] = initialize_queue();
		if (scheduler->wait_queue[i]
			== (struct queue *)MALLOC_NULL_POINTER) {
			for (j = i + 1; j < (int)io; j++)
				destroy_queue(scheduler->wait_queue[i]);
			free(scheduler->wait_queue);
			destroy_hash(scheduler->thread_hash);
			DeleteCriticalSection(&scheduler->scheduler_opperation);
			destroy_queue(scheduler->all_threads);
			free(scheduler);
			return FAILED_MALLOC;
		}
	}

	for (i = 0; i<= SO_MAX_PRIO; i++) {
		scheduler->ready_queue[i] = initialize_queue();
		if (scheduler->ready_queue[i]
			== (struct queue *)MALLOC_NULL_POINTER) {
			for (j = i + 1; j <= SO_MAX_PRIO; j++)
				destroy_queue(scheduler->ready_queue[i]);
			free(scheduler->ready_queue);
			for (j = 0; j <= scheduler->supported_io_num; j++)
				destroy_queue(scheduler->wait_queue[i]);
			free(scheduler->wait_queue);
			destroy_hash(scheduler->thread_hash);
			DeleteCriticalSection(&scheduler->scheduler_opperation);
			destroy_queue(scheduler->all_threads);
			free(scheduler);
			return FAILED_MALLOC;
		}
	}
	scheduler->current_thread = NULL;
	return SCHED_INITIALIZE_SUCCESS;
}

#else
#error "Unknown platform"
#endif

static struct so_thread *pop_next_running_thread(void)
{
	int i;

	for (i = SO_MAX_PRIO; i >= 0; i--)
		if (scheduler->ready_queue[i]->beginning != NULL)
			return pop_queue(scheduler->ready_queue[i]);
	return NULL;
}

static struct so_thread *get_next_running_thread(void)
{
	int i;

	for (i = SO_MAX_PRIO; i >= 0; i--)
		if (scheduler->ready_queue[i]->beginning != NULL)
			return scheduler->ready_queue[i]->beginning->info;
	return NULL;
}

/*
 * Planifica unde ajunge un thread
 */
#ifdef __linux__

static void check_scheduler(struct so_thread *myself)
{
	struct so_thread *crt_thread, *next_thread;

	pthread_mutex_lock(&scheduler->scheduler_opperation);
	if (myself == NULL || myself->status == TERMINATED) {
		scheduler->current_thread = pop_next_running_thread();
		goto end;
	}

	crt_thread = scheduler->current_thread;

	if (crt_thread == myself) {
		next_thread = get_next_running_thread();
		if (myself->time_quantum == scheduler->time_quantum
		|| (next_thread != NULL &&
			next_thread->priority > myself->priority)) {
			myself->time_quantum = 1;
			if (myself->status == RUNNING) {
				myself->status = READY;
				insert_into_queue(myself,
				scheduler->ready_queue[myself->priority]);
			}
			scheduler->current_thread = pop_next_running_thread();
		} else
			myself->time_quantum++;
		goto end;
	}
end:
	if (scheduler->current_thread != NULL) {
		if (scheduler->current_thread->status == READY)
			scheduler->current_thread->status = RUNNING;
	} else {
		myself->time_quantum = 1;
		scheduler->current_thread = myself;
	}

	sem_post(&scheduler->current_thread->semaphore);
	pthread_mutex_unlock(&scheduler->scheduler_opperation);
}
#elif defined(_WIN32)
static void check_scheduler(struct so_thread *myself)
{
	struct so_thread *crt_thread, *next_thread;

	EnterCriticalSection(&scheduler->scheduler_opperation);
	
	if (myself == NULL || myself->status == TERMINATED) {
		scheduler->current_thread = pop_next_running_thread();
		goto end;
	}

	crt_thread = scheduler->current_thread;

	if (crt_thread == myself) {
		next_thread = get_next_running_thread();
		if (myself->time_quantum == scheduler->time_quantum
		|| (next_thread != NULL &&
			next_thread->priority > myself->priority)) {
			myself->time_quantum = 1;
			if (myself->status == RUNNING) {
				myself->status = READY;
				insert_into_queue(myself,
				scheduler->ready_queue[myself->priority]);
			}
			scheduler->current_thread = pop_next_running_thread();
		} else
			myself->time_quantum++;
		goto end;
	}
end:
	if (scheduler->current_thread != NULL) {
		if (scheduler->current_thread->status == READY)
			scheduler->current_thread->status = RUNNING;
	} else {
		myself->time_quantum = 1;
		scheduler->current_thread = myself;
	}

	ReleaseSemaphore(scheduler->current_thread->semaphore,
						1,
						NULL);
	LeaveCriticalSection(&scheduler->scheduler_opperation);
}
#else
#error "Unknown platform"
#endif

#ifdef __linux__

static void *thread_function_wrapper(void *arg)
{
	struct so_thread *this_thread;

	this_thread = (struct so_thread *)arg;
	pthread_mutex_lock(&scheduler->scheduler_opperation);
	hash_insert(scheduler->thread_hash, this_thread);

	insert_into_queue(this_thread,
		scheduler->ready_queue[this_thread->priority]);

	insert_into_queue(this_thread, scheduler->all_threads);
	pthread_mutex_unlock(&scheduler->scheduler_opperation);
	sem_post(&this_thread->fork_sem);
	sem_wait(&this_thread->semaphore);
	(this_thread->handler)(this_thread->priority);
	this_thread->status = TERMINATED;
	pthread_mutex_lock(&scheduler->scheduler_opperation);
	hash_delete(scheduler->thread_hash, this_thread->tid);
	pthread_mutex_unlock(&scheduler->scheduler_opperation);
	check_scheduler(this_thread);
	return NULL;
}

#elif defined(_WIN32)

static DWORD WINAPI thread_function_wrapper(LPVOID arg)
{
	struct so_thread *this_thread;

	this_thread = (struct so_thread *)arg;
	EnterCriticalSection(&scheduler->scheduler_opperation);
	hash_insert(scheduler->thread_hash, this_thread);

	insert_into_queue(this_thread,
		scheduler->ready_queue[this_thread->priority]);

	insert_into_queue(this_thread, scheduler->all_threads);
	LeaveCriticalSection(&scheduler->scheduler_opperation);

	ReleaseSemaphore(this_thread->fork_sem,
						1,
						NULL);
	WaitForSingleObject(this_thread->semaphore,
						INFINITE);

	(this_thread->handler)(this_thread->priority);
	this_thread->status = TERMINATED;
	EnterCriticalSection(&scheduler->scheduler_opperation);
	hash_delete(scheduler->thread_hash, this_thread->tid);
	LeaveCriticalSection(&scheduler->scheduler_opperation);
	check_scheduler(this_thread);
	return 0;
}

#else
#error "Unknown platform"
#endif

#ifdef __linux__

DECL_PREFIX tid_t so_fork(so_handler *func, unsigned int priority)
{
	struct so_thread *myself;
	struct so_thread *thread;

	if (func ==  NULL)
		return INVALID_TID;
	if (priority > SO_MAX_PRIO)
		return INVALID_TID;
	pthread_mutex_lock(&scheduler->scheduler_opperation);
	myself = hash_get_value(scheduler->thread_hash, pthread_self());
	pthread_mutex_unlock(&scheduler->scheduler_opperation);

	thread = initialize_so_thread(priority, func);
	pthread_create(&thread->tid, NULL, thread_function_wrapper, thread);
	sem_wait(&thread->fork_sem);
	check_scheduler(myself);

	if (myself != NULL)
		sem_wait(&myself->semaphore);

	return thread->tid;
}
#elif defined(_WIN32)
DECL_PREFIX tid_t so_fork(so_handler *func, unsigned int priority)
{
	struct so_thread *myself;
	struct so_thread *thread;

	if (func ==  NULL)
		return INVALID_TID;
	if (priority > SO_MAX_PRIO)
		return INVALID_TID;
	EnterCriticalSection(&scheduler->scheduler_opperation);
	myself = hash_get_value(scheduler->thread_hash, GetCurrentThreadId());
	LeaveCriticalSection(&scheduler->scheduler_opperation);

	thread = initialize_so_thread(priority, func);
	CreateThread(NULL,
		0,
		(LPTHREAD_START_ROUTINE)thread_function_wrapper,
		thread,
		0,
		&thread->tid);

	WaitForSingleObject(thread->fork_sem,
						INFINITE);
	check_scheduler(myself);

	if (myself != NULL)
		WaitForSingleObject(myself->semaphore,
						INFINITE);

	return thread->tid;
}
#else
#error "Unknown platform"
#endif

#ifdef __linux__

DECL_PREFIX int so_wait(unsigned int io)
{
	struct so_thread *myself;

	if (scheduler->supported_io_num <= io)
		return NO_SUCH_IO_DEVICE;

	pthread_mutex_lock(&scheduler->scheduler_opperation);
	myself = hash_get_value(scheduler->thread_hash, pthread_self());
	myself->time_quantum = scheduler->time_quantum;
	myself->status = WAITING;
	insert_into_queue(myself, scheduler->wait_queue[io]);
	pthread_mutex_unlock(&scheduler->scheduler_opperation);
	check_scheduler(myself);
	sem_wait(&myself->semaphore);
	return WAIT_SUCCESS;
}

#elif defined(_WIN32)

DECL_PREFIX int so_wait(unsigned int io)
{
	struct so_thread *myself;

	if (scheduler->supported_io_num <= io)
		return NO_SUCH_IO_DEVICE;

	EnterCriticalSection(&scheduler->scheduler_opperation);
	myself = hash_get_value(scheduler->thread_hash, GetCurrentThreadId());
	myself->time_quantum = scheduler->time_quantum;
	myself->status = WAITING;
	insert_into_queue(myself, scheduler->wait_queue[io]);
	LeaveCriticalSection(&scheduler->scheduler_opperation);
	check_scheduler(myself);
	WaitForSingleObject(myself->semaphore,
						INFINITE);
	return WAIT_SUCCESS;
}

#else
#error "Unknown platform"
#endif

#ifdef __linux__

DECL_PREFIX int so_signal(unsigned int io)
{
	struct so_thread *myself, *crt_thread;
	int num_woken;

	if (scheduler->supported_io_num <= io)
		return NO_SUCH_IO_DEVICE;
	num_woken = 0;
	pthread_mutex_lock(&scheduler->scheduler_opperation);
	myself = hash_get_value(scheduler->thread_hash, pthread_self());
	crt_thread = pop_queue(scheduler->wait_queue[io]);
	while (crt_thread != INVALID_INFO) {
		crt_thread->status = READY;
		insert_into_queue(crt_thread,
			scheduler->ready_queue[crt_thread->priority]);
		crt_thread = pop_queue(scheduler->wait_queue[io]);
		num_woken++;
	}
	pthread_mutex_unlock(&scheduler->scheduler_opperation);
	check_scheduler(myself);
	sem_wait(&myself->semaphore);
	return num_woken;
}

#elif defined(_WIN32)

DECL_PREFIX int so_signal(unsigned int io)
{
	struct so_thread *myself, *crt_thread;
	int num_woken;

	if (scheduler->supported_io_num <= io)
		return NO_SUCH_IO_DEVICE;
	num_woken = 0;
	EnterCriticalSection(&scheduler->scheduler_opperation);
	myself = hash_get_value(scheduler->thread_hash, GetCurrentThreadId());
	crt_thread = pop_queue(scheduler->wait_queue[io]);
	while (crt_thread != INVALID_INFO) {
		crt_thread->status = READY;
		insert_into_queue(crt_thread,
			scheduler->ready_queue[crt_thread->priority]);
		crt_thread = pop_queue(scheduler->wait_queue[io]);
		num_woken++;
	}
	LeaveCriticalSection(&scheduler->scheduler_opperation);
	check_scheduler(myself);
	WaitForSingleObject(myself->semaphore,
						INFINITE);
	return num_woken;
}

#else
#error "Unknown platform"
#endif

#ifdef __linux__

DECL_PREFIX void so_exec(void)
{
	struct so_thread *myself;

	pthread_mutex_lock(&scheduler->scheduler_opperation);
	myself = hash_get_value(scheduler->thread_hash, pthread_self());
	pthread_mutex_unlock(&scheduler->scheduler_opperation);
	check_scheduler(myself);
	sem_wait(&myself->semaphore);
}

#elif defined(_WIN32)

DECL_PREFIX void so_exec(void)
{
	struct so_thread *myself;

	EnterCriticalSection(&scheduler->scheduler_opperation);
	myself = hash_get_value(scheduler->thread_hash, GetCurrentThreadId());
	LeaveCriticalSection(&scheduler->scheduler_opperation);
	check_scheduler(myself);
	WaitForSingleObject(myself->semaphore,
						INFINITE);
}

#else
#error "Unknown platform"
#endif

#ifdef __linux__

DECL_PREFIX void so_end(void)
{
	int i;
	struct so_thread *crt_so_thread;

	if (scheduler == NULL)
		return;

	pthread_mutex_lock(&scheduler->scheduler_opperation);
	crt_so_thread = pop_queue(scheduler->all_threads);
	pthread_mutex_unlock(&scheduler->scheduler_opperation);

	while (crt_so_thread != NULL) {
		pthread_join(crt_so_thread->tid, NULL);
		pthread_mutex_lock(&scheduler->scheduler_opperation);
		destroy_so_thread(crt_so_thread);
		crt_so_thread = pop_queue(scheduler->all_threads);
		pthread_mutex_unlock(&scheduler->scheduler_opperation);
	}

	for (i = 0; i < scheduler->supported_io_num; i++)
		destroy_queue(scheduler->wait_queue[i]);
	free(scheduler->wait_queue);

	for (i = 0; i <= SO_MAX_PRIO; i++)
		destroy_queue(scheduler->ready_queue[i]);
	free(scheduler->ready_queue);

	pthread_mutex_destroy(&scheduler->scheduler_opperation);
	destroy_queue(scheduler->all_threads);
	destroy_hash(scheduler->thread_hash);
	free(scheduler);
	scheduler = NULL;
}
#elif defined(_WIN32)

DECL_PREFIX void so_end(void)
{
	unsigned int i;
	struct so_thread *crt_so_thread;
	HANDLE thread_handle;

	if (scheduler == NULL)
		return;

	EnterCriticalSection(&scheduler->scheduler_opperation);
	crt_so_thread = pop_queue(scheduler->all_threads);
	LeaveCriticalSection(&scheduler->scheduler_opperation);

	while (crt_so_thread != NULL) {
		thread_handle = OpenThread(THREAD_ALL_ACCESS,
							FALSE,
							crt_so_thread->tid);

		WaitForSingleObject(OpenThread(THREAD_ALL_ACCESS,
							FALSE,
							crt_so_thread->tid),
							INFINITE);
		EnterCriticalSection(&scheduler->scheduler_opperation);
		destroy_so_thread(crt_so_thread);
		crt_so_thread = pop_queue(scheduler->all_threads);
		LeaveCriticalSection(&scheduler->scheduler_opperation);
	}

	for (i = 0; i < scheduler->supported_io_num; i++)
		destroy_queue(scheduler->wait_queue[i]);
	free(scheduler->wait_queue);

	for (i = 0; i <= SO_MAX_PRIO; i++)
		destroy_queue(scheduler->ready_queue[i]);
	free(scheduler->ready_queue);

	DeleteCriticalSection(&scheduler->scheduler_opperation);
	destroy_queue(scheduler->all_threads);
	destroy_hash(scheduler->thread_hash);
	free(scheduler);
	scheduler = NULL;
}

#else
#error "Unknown platform"
#endif
