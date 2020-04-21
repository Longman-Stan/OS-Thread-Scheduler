/*
 * Threads scheduler header
 *
 * 2017, Operating Systems
 */
#ifndef SO_SCHEDULER_H_
#define SO_SCHEDULER_H_

/* OS dependent stuff */
#ifdef __linux__
#include <pthread.h>
#include <semaphore.h>

#define SEM_T sem_t
#define SEM_DESTROY sem_destroy
#define SEM_FAIL -1

#define MUTEX pthread_mutex_t
#define CRITICAL_SECTION_ENTER pthread_mutex_lock
#define CRITICAL_SECTION_LEAVE pthread_mutex_unlock
#define INITIALIZE_MUTEX pthread_mutex_init
#define MUTEX_PARAM NULL
#define DESTROY_MUTEX pthread_mutex_destroy
#define MUTEX_INIT_SUCCESS 0

#define SELF_TID pthread_self()

#define WRAPPER_RET void*
#define WRAPPER_API
#define WRAPPER_SUCCESS NULL

#define DECL_PREFIX

typedef pthread_t tid_t;
#elif defined(_WIN32)
#include <windows.h>

#define SEM_T HANDLE
#define SEM_DESTROY CloseHandle
#define SEM_FAIL NULL

#define MUTEX CRITICAL_SECTION
#define CRITICAL_SECTION_ENTER EnterCriticalSection
#define CRITICAL_SECTION_LEAVE LeaveCriticalSection
#define DESTROY_MUTEX DeleteCriticalSection
#define MUTEX_PARAM 0x040
#define MUTEX_INIT_SUCCESS 0

#define SELF_TID GetCurrentThreadId()

#define WRAPPER_RET DWORD
#define WRAPPER_API WINAPI
#define WRAPPER_SUCCESS 0

#ifdef DLL_IMPORTS
#define DECL_PREFIX __declspec(dllimport)
#else
#define DECL_PREFIX __declspec(dllexport)
#endif

typedef DWORD tid_t;
#else
#error "Unknown platform"
#endif

/*
 * error codes
 */
#define FAILED_MALLOC -12
#define SCHED_INITIALIZE_SUCCESS 0
#define SCHED_ALREADY_INITIALIZED -1
#define MUTEX_INIT_FAILURE -1
#define BARRIER_INIT_FAILURE -1
#define NO_SUCH_IO_DEVICE -1
#define WAIT_SUCCESS 0
#define BAD_INIT_PARAMS -1

#define FCFS_QUEUE 0
/*
 * constants
 */
#define HASH_SIZE 1013

/*
 * the maximum priority that can be assigned to a thread
 */
#define SO_MAX_PRIO 5
/*
 * the maximum number of events
 */
#define SO_MAX_NUM_EVENTS 256

/*
 * return value of failed tasks
 */
#define INVALID_TID ((tid_t)0)

#ifdef __cplusplus
extern "C" {
#endif

/*
 * handler prototype
 */
typedef void (so_handler)(unsigned int);

/*
 * creates and initializes scheduler
 * + time quantum for each thread
 * + number of IO devices supported
 * returns: 0 on success or negative on error
 */
DECL_PREFIX int so_init(unsigned int time_quantum, unsigned int io);

/*
 * creates a new so_task_t and runs it according to the scheduler
 * + handler function
 * + priority
 * returns: tid of the new task if successful or INVALID_TID
 */
DECL_PREFIX tid_t so_fork(so_handler *func, unsigned int priority);

/*
 * waits for an IO device
 * + device index
 * returns: -1 if the device does not exist or 0 on success
 */
DECL_PREFIX int so_wait(unsigned int io);

/*
 * signals an IO device
 * + device index
 * return the number of tasks woke or -1 on error
 */
DECL_PREFIX int so_signal(unsigned int io);

/*
 * does whatever operation
 */
DECL_PREFIX void so_exec(void);

/*
 * destroys a scheduler
 */
DECL_PREFIX void so_end(void);

#ifdef __cplusplus
}
#endif

#endif /* SO_SCHEDULER_H_ */
