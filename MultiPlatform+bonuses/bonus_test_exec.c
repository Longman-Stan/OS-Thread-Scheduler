#include "so_scheduler.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define SO_MAX_EXECUTION_TIME 679

static unsigned int tasks_no;
static unsigned int exec_time;

static void bonus_test(unsigned int priority)
{
	unsigned int executed_fork = 0;
	unsigned int rand_iterations;

	if (tasks_no >= SO_MAX_EXECUTION_TIME)
		return;
	/* get a rand number of iterations to do */
	rand_iterations = rand() % 31 + 1;

	while (rand_iterations-- && exec_time < SO_MAX_EXECUTION_TIME) {

		/* fill in history */
		exec_time++;
		/*
		 * here we force all tasks to execute at least one fork
		 * if it was executed previously, then we offer random
		 * chances to execute either fork or exec
		 */
		if ((!executed_fork && rand_iterations == 0) || rand() % 2) {
			/* create new task with random priority */
			so_fork(bonus_test,
				rand() % 5);
			executed_fork = 1;
		} else {
			so_exec();
		}
	}
}

int main(void)
{
	exec_time = 0;
	tasks_no = 0;
	srand((unsigned int)time(NULL));
	so_init(5, 256);

	so_fork(bonus_test, rand() % 5);

	so_end();
	return 0;
}
