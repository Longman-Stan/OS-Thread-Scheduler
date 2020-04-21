#include "so_scheduler.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static void bonus_test_wait4(unsigned int priority)
{
	so_exec();
	so_signal(1);
	so_exec();
}

static void bonus_test_wait3(unsigned int priority)
{
	so_fork(bonus_test_wait4, rand() % 5);
	so_exec();
	so_wait(2);
	so_exec();
	so_signal(0);
}

static void bonus_test_wait2(unsigned int priority)
{
	so_exec();
	so_wait(1);
	so_signal(2);
	so_exec();
}

static void bonus_test_wait1(unsigned int priority)
{
	so_fork(bonus_test_wait2, rand() % 5);
	so_fork(bonus_test_wait3, rand() % 5);
	so_exec();
	so_exec();
	so_exec();
	so_wait(0);
	so_exec();
	so_exec();
}

int main(void)
{
	srand((unsigned int)time(NULL));
	so_init(5, 256);

	so_fork(bonus_test_wait1, rand() % 5);

	so_end();
	return 0;
}
