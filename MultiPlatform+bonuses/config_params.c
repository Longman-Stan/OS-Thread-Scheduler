#include <stdio.h>
#include "so_sched_initializer.h"

int main(void)
{
	struct sched_info *sched_info;

	sched_info = initialize_sched_info();

	sched_info->use_file = False;
	printf("Would you like to use the file for scheduler configuration?\n");
	printf("If so, type '1' and press ENTER\n");
	scanf("%u", &sched_info->use_file);
	if (sched_info->use_file == False)
		goto end;

	printf("Insert the time quantum and press ENTER\n");
	scanf("%u", &sched_info->time_quantum);

	printf("Insert the number of supported io devices and press ENTER\n");
	scanf("%u", &sched_info->supported_io_num);

	sched_info->enable_log = False;
	printf("Would you like to log information about the scheduler?\n");
	printf("If so, type '1' and press ENTER\n");
	scanf("%u", &sched_info->enable_log);
	if (sched_info->enable_log == False)
		goto fcfs_flag;

	printf("How many messages would you like to see in the log file?\n");
	printf("Insert the desired number and press ENTER\n");
	scanf("%u", &sched_info->log_window_size);

fcfs_flag:
	sched_info->use_fcfs = False;
	printf("Would you like to use the First Come First Served scheduling"
	       " algorithm instead of Round Robin?\n");
	printf("If so, type '1' and press ENTER\n");
	scanf("%u", &sched_info->use_fcfs);
end:
	write_data(sched_info);
	return 0;
}
