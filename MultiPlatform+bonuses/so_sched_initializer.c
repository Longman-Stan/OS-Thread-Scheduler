#include "so_sched_initializer.h"

struct sched_info *initialize_sched_info(void)
{
	struct sched_info *sched_info;
	FILE *file;

	sched_info = (struct sched_info *) malloc(sizeof(struct sched_info));
	if (sched_info == NULL)
		return INVALID_SCHED_INFO_INITIALIZE;

	file = fopen(".config", "r");
	if (file == NULL) {
		free(sched_info);
		return INVALID_SCHED_INFO_INITIALIZE;
	}

	fscanf(file, "%u", &sched_info->use_file);
	fscanf(file, "%u", &sched_info->time_quantum);
	fscanf(file, "%u", &sched_info->supported_io_num);
	fscanf(file, "%u", &sched_info->enable_log);
	fscanf(file, "%u", &sched_info->log_window_size);
	fscanf(file, "%u", &sched_info->use_fcfs);

	fclose(file);
	return sched_info;
}

void set_use_file(struct sched_info *sched_info, enum BOOL use_file)
{
	sched_info->use_file = use_file;
}

void set_time_quantum(struct sched_info *sched_info,
		      unsigned int time_quantum)
{
	sched_info->time_quantum = time_quantum;
}

void set_supported_io_num(struct sched_info *sched_info,
			  unsigned int supported_io_num)
{
	sched_info->supported_io_num = supported_io_num;
}

void set_log_enable(struct sched_info *sched_info, enum BOOL log_enable)
{
	sched_info->enable_log = log_enable;
}

void set_log_window_size(struct sched_info *sched_info,
			 unsigned int log_window_size)
{
	sched_info->log_window_size = log_window_size;
}

void set_use_fcfs(struct sched_info *sched_info, enum BOOL use_fcfs)
{
	sched_info->use_fcfs = use_fcfs;
}

int write_data(struct sched_info *sched_info)
{
	FILE *file = fopen(".config", "w");

	if (file == NULL)
		return FAILED_WRITE;

	fprintf(file, "%u\n", sched_info->use_file);
	fprintf(file, "%u\n", sched_info->time_quantum);
	fprintf(file, "%u\n", sched_info->supported_io_num);
	fprintf(file, "%u\n", sched_info->enable_log);
	fprintf(file, "%u\n", sched_info->log_window_size);
	fprintf(file, "%u\n", sched_info->use_fcfs);

	fclose(file);
	return SUCCESSFUL_WRITE;
}
