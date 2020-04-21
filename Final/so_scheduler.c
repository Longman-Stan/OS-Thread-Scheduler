#include <stdio.h>
#include "so_scheduler_def.h"

static struct so_scheduler *scheduler;

static WRAPPER_RET WRAPPER_API thread_function_wrapper(void *arg);

#ifdef __linux__

static inline void join(tid_t tid)
{
	pthread_join(tid, NULL);
}

static inline void semaphore_wait(SEM_T *semaphore)
{
	sem_wait(semaphore);
}

static inline void thread_create(struct so_thread *thread)
{
	pthread_create(&thread->tid, NULL, thread_function_wrapper, thread);
}

static inline void semaphore_post(SEM_T *semaphore)
{
	sem_post(semaphore);
}

#elif defined(_WIN32)

static __inline void join(tid_t tid)
{
	WaitForSingleObject(OpenThread(THREAD_ALL_ACCESS,
				       FALSE,
				       tid),
			    INFINITE);
}

static __inline void semaphore_wait(SEM_T *semaphore)
{
	WaitForSingleObject(*semaphore, INFINITE);
}

static __inline void thread_create(struct so_thread *thread)
{
	CreateThread(NULL,
		     0,
		     (LPTHREAD_START_ROUTINE)thread_function_wrapper,
		     thread,
		     0,
		     &thread->tid);
}

static __inline void semaphore_post(SEM_T *semaphore)
{
	ReleaseSemaphore(*semaphore,
			 1,
			 NULL);
}

#else
#error "Unknown platform"
#endif

static void use_file(void)
{
	struct sched_info *sched_info;

	sched_info = initialize_sched_info();
	if (sched_info != INVALID_SCHED_INFO_INITIALIZE) {
		if (sched_info->use_file) {
			scheduler->time_quantum = sched_info->time_quantum;
			scheduler->supported_io_num =
				sched_info->supported_io_num;
			scheduler->enable_log = sched_info->enable_log;
			if (scheduler->enable_log) {
				scheduler->logger = initialize_logger(
					sched_info->log_window_size);
				if (scheduler->logger == INVALID_LOGGER)
					scheduler->enable_log = False;
				else
					printf("logging enabled\n");
			}

			scheduler->use_fcfs = sched_info->use_fcfs;
			if (scheduler->use_fcfs == True) {
				printf("First come first served enabled\n");
				scheduler->time_quantum = 1;
			}
		}
	}

	free(sched_info);
}

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
	scheduler->use_fcfs = False;
	scheduler->enable_log = False;

#ifdef BONUS
	use_file();
	printf("%u %u\n", scheduler->time_quantum, scheduler->supported_io_num);
#endif

	scheduler->thread_hash = initialize_hash(HASH_SIZE);
	if (scheduler->thread_hash == (struct hash *)FAILED_MALLOC) {
		free(scheduler);
		return FAILED_MALLOC;
	}

	if (INITIALIZE_MUTEX(&scheduler->scheduler_opperation, MUTEX_PARAM)
	    != MUTEX_INIT_SUCCESS) {
		destroy_hash(scheduler->thread_hash);
		free(scheduler);
		return MUTEX_INIT_FAILURE;
	}

	scheduler->all_threads = initialize_queue();
	if (scheduler->all_threads == (struct queue *)MALLOC_NULL_POINTER) {
		destroy_hash(scheduler->thread_hash);
		DESTROY_MUTEX(&scheduler->scheduler_opperation);
		free(scheduler);
		return FAILED_MALLOC;
	}

	scheduler->wait_queue = (struct queue **) malloc
				(io * sizeof(struct queue *));

	if (scheduler->wait_queue == NULL) {
		destroy_hash(scheduler->thread_hash);
		DESTROY_MUTEX(&scheduler->scheduler_opperation);
		destroy_queue(scheduler->all_threads);
		free(scheduler);
		return FAILED_MALLOC;
	}

	scheduler->ready_queue = (struct queue **) malloc(
			(SO_MAX_PRIO + 1) * sizeof(struct queue *));
	if (scheduler->ready_queue == NULL) {
		free(scheduler->wait_queue);
		destroy_hash(scheduler->thread_hash);
		DESTROY_MUTEX(&scheduler->scheduler_opperation);
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
			DESTROY_MUTEX(&scheduler->scheduler_opperation);
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
			DESTROY_MUTEX(&scheduler->scheduler_opperation);
			destroy_queue(scheduler->all_threads);
			free(scheduler);
			return FAILED_MALLOC;
		}
	}

	scheduler->current_thread = NULL;
	return SCHED_INITIALIZE_SUCCESS;
}

static struct so_thread *pop_next_running_thread(void)
{
	int i;

	if (scheduler->use_fcfs == True)
		return pop_queue(scheduler->ready_queue[FCFS_QUEUE]);

	for (i = SO_MAX_PRIO; i >= 0; i--)
		if (scheduler->ready_queue[i]->beginning != NULL)
			return pop_queue(scheduler->ready_queue[i]);

	return NULL;
}

static struct so_thread *get_next_running_thread(void)
{
	int i;

	if (scheduler->use_fcfs == True)
		return scheduler->ready_queue[FCFS_QUEUE]->beginning->info;

	for (i = SO_MAX_PRIO; i >= 0; i--)
		if (scheduler->ready_queue[i]->beginning != NULL)
			return scheduler->ready_queue[i]->beginning->info;
	return NULL;
}

/*
 * Planifica unde ajunge un thread
 */
static void check_scheduler(struct so_thread *myself)
{
	struct so_thread *crt_thread, *next_thread;

	CRITICAL_SECTION_ENTER(&scheduler->scheduler_opperation);
	if (myself == HASH_VALUE_NOT_FOUND || myself->status == TERMINATED) {
		scheduler->current_thread = pop_next_running_thread();
		goto end;
	}

	crt_thread = scheduler->current_thread;

	if (crt_thread == myself) {
		next_thread = get_next_running_thread();
		if (myself->time_quantum == scheduler->time_quantum
		    || (next_thread != INVALID_POP_INFO &&
			next_thread->priority > myself->priority)) {
			myself->time_quantum = 1;
			if (myself->status == RUNNING) {
				myself->status = READY;

		/*indentarea proasta de la
		 *insert e din cauza checkstyle
		 *altfel imi zice "more than 80 chars..."
		 */
				if (scheduler->use_fcfs == True)
					insert_into_queue(myself,
					scheduler->ready_queue[FCFS_QUEUE]);
				else
					insert_into_queue(myself,
				scheduler->ready_queue[myself->priority]);
			}
			scheduler->current_thread = pop_next_running_thread();
		} else
			myself->time_quantum++;
		goto end;
	}
end:
	if (scheduler->current_thread != INVALID_POP_INFO) {
		if (scheduler->current_thread->status == READY)
			scheduler->current_thread->status = RUNNING;
	} else {
		myself->time_quantum = 1;
		scheduler->current_thread = myself;
	}

	if (scheduler->current_thread != myself && scheduler->enable_log
	    && myself != HASH_VALUE_NOT_FOUND)
		log_preempt(scheduler->logger, myself->tid);

	semaphore_post(&scheduler->current_thread->semaphore);
	CRITICAL_SECTION_LEAVE(&scheduler->scheduler_opperation);
}


static WRAPPER_RET WRAPPER_API thread_function_wrapper(void *arg)
{
	struct so_thread *this_thread;

	this_thread = (struct so_thread *)arg;
	CRITICAL_SECTION_ENTER(&scheduler->scheduler_opperation);
	hash_insert(scheduler->thread_hash, this_thread);

	if (scheduler->use_fcfs == True)
		insert_into_queue(this_thread,
			scheduler->ready_queue[FCFS_QUEUE]);
	else
		insert_into_queue(this_thread,
			scheduler->ready_queue[this_thread->priority]);

	insert_into_queue(this_thread, scheduler->all_threads);
	CRITICAL_SECTION_LEAVE(&scheduler->scheduler_opperation);
	semaphore_post(&this_thread->fork_sem);
	semaphore_wait(&this_thread->semaphore);
	(this_thread->handler)(this_thread->priority);
	this_thread->status = TERMINATED;
	CRITICAL_SECTION_ENTER(&scheduler->scheduler_opperation);
	hash_delete(scheduler->thread_hash, this_thread->tid);

	if (scheduler->enable_log)
		log_terminate(scheduler->logger, this_thread->tid);

	CRITICAL_SECTION_LEAVE(&scheduler->scheduler_opperation);
	check_scheduler(this_thread);
	return WRAPPER_SUCCESS;
}

DECL_PREFIX tid_t so_fork(so_handler *func, unsigned int priority)
{
	struct so_thread *myself;
	struct so_thread *thread;

	if (func ==  NULL) {
		if (scheduler->enable_log)
			log_error(scheduler->logger,
				"invalid function handler");
		return INVALID_TID;
	}

	if (priority > SO_MAX_PRIO) {
		if (scheduler->enable_log)
			log_error(scheduler->logger,
				  "priority more than maximum");
		return INVALID_TID;
	}

	CRITICAL_SECTION_ENTER(&scheduler->scheduler_opperation);
	myself = hash_get_value(scheduler->thread_hash, SELF_TID);
	CRITICAL_SECTION_LEAVE(&scheduler->scheduler_opperation);
	thread = initialize_so_thread(priority, func);

	thread_create(thread);
	semaphore_wait(&thread->fork_sem);

	if (scheduler->enable_log) {
		log_fork(scheduler->logger, (myself == NULL) ? 0 : myself->tid,
			 thread->tid, thread->priority);
	}

	if (scheduler->use_fcfs == False || myself == HASH_VALUE_NOT_FOUND)
		check_scheduler(myself);

	if (scheduler->use_fcfs == False && myself != HASH_VALUE_NOT_FOUND)
		semaphore_wait(&myself->semaphore);

	return thread->tid;
}


DECL_PREFIX int so_wait(unsigned int io)
{
	struct so_thread *myself;

	if (scheduler->supported_io_num <= io) {
		if (scheduler->enable_log)
			log_error(scheduler->logger,
				"Device for wait does not exist");
		return NO_SUCH_IO_DEVICE;
	}

	CRITICAL_SECTION_ENTER(&scheduler->scheduler_opperation);
	myself = hash_get_value(scheduler->thread_hash, SELF_TID);
	myself->time_quantum = scheduler->time_quantum;
	myself->status = WAITING;
	insert_into_queue(myself, scheduler->wait_queue[io]);

	if (scheduler->enable_log)
		log_wait(scheduler->logger, myself->tid, io);

	CRITICAL_SECTION_LEAVE(&scheduler->scheduler_opperation);
	check_scheduler(myself);
	semaphore_wait(&myself->semaphore);
	return WAIT_SUCCESS;
}

DECL_PREFIX int so_signal(unsigned int io)
{
	struct so_thread *myself, *crt_thread;
	int num_woken;

	if (scheduler->supported_io_num <= io) {
		if (scheduler->enable_log)
			log_error(scheduler->logger,
				"Device for signal does not exist");
		return NO_SUCH_IO_DEVICE;
	}
	num_woken = 0;
	CRITICAL_SECTION_ENTER(&scheduler->scheduler_opperation);
	myself = hash_get_value(scheduler->thread_hash, SELF_TID);
	crt_thread = pop_queue(scheduler->wait_queue[io]);
	while (crt_thread != INVALID_POP_INFO) {
		crt_thread->status = READY;

		if (scheduler->use_fcfs == True)
			insert_into_queue(crt_thread,
				scheduler->ready_queue[FCFS_QUEUE]);
		else
			insert_into_queue(crt_thread,
				scheduler->ready_queue[crt_thread->priority]);

		crt_thread = pop_queue(scheduler->wait_queue[io]);
		num_woken++;
	}

	if (scheduler->enable_log)
		log_signal(scheduler->logger, myself->tid, io);

	CRITICAL_SECTION_LEAVE(&scheduler->scheduler_opperation);

	if (scheduler->use_fcfs == False) {
		check_scheduler(myself);
		semaphore_wait(&myself->semaphore);
	}
	return num_woken;
}


DECL_PREFIX void so_exec(void)
{
	struct so_thread *myself;

	CRITICAL_SECTION_ENTER(&scheduler->scheduler_opperation);
	myself = hash_get_value(scheduler->thread_hash, SELF_TID);

	if (scheduler->enable_log)
		log_exec(scheduler->logger, myself->tid);

	CRITICAL_SECTION_LEAVE(&scheduler->scheduler_opperation);

	if (scheduler->use_fcfs == False) {
		check_scheduler(myself);
		semaphore_wait(&myself->semaphore);
	}
}

DECL_PREFIX void so_end(void)
{
	unsigned int i;
	struct so_thread *crt_so_thread;

	if (scheduler == NULL)
		return;

	CRITICAL_SECTION_ENTER(&scheduler->scheduler_opperation);
	crt_so_thread = pop_queue(scheduler->all_threads);
	CRITICAL_SECTION_LEAVE(&scheduler->scheduler_opperation);

	while (crt_so_thread != NULL) {
		join(crt_so_thread->tid);
		CRITICAL_SECTION_ENTER(&scheduler->scheduler_opperation);
		destroy_so_thread(crt_so_thread);
		crt_so_thread = pop_queue(scheduler->all_threads);
		CRITICAL_SECTION_LEAVE(&scheduler->scheduler_opperation);
	}

	for (i = 0; i < scheduler->supported_io_num; i++)
		destroy_queue(scheduler->wait_queue[i]);
	free(scheduler->wait_queue);

	for (i = 0; i <= SO_MAX_PRIO; i++)
		destroy_queue(scheduler->ready_queue[i]);
	free(scheduler->ready_queue);

	DESTROY_MUTEX(&scheduler->scheduler_opperation);
	destroy_queue(scheduler->all_threads);
	destroy_hash(scheduler->thread_hash);

	if (scheduler->enable_log == True)
		destroy_logger(scheduler->logger);

	free(scheduler);
	scheduler = NULL;
}
