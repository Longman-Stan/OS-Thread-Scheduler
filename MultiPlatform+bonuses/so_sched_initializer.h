#pragma once
#include <stdio.h>
#include <stdlib.h>

#define INVALID_SCHED_INFO_INITIALIZE NULL
#define FAILED_WRITE -1
#define SUCCESSFUL_WRITE 0

enum BOOL {
	False = 0,
	True = 1
};

struct sched_info {
	enum BOOL use_file;
	unsigned int time_quantum;
	unsigned int supported_io_num;
	enum BOOL enable_log;
	unsigned int log_window_size;
	enum BOOL use_fcfs;
};

struct sched_info *initialize_sched_info(void);

void set_use_file(struct sched_info *sched_info, enum BOOL use_file);

void set_time_quantum(struct sched_info *sched_info,
	unsigned int time_quantum);

void set_supported_io_num(struct sched_info *sched_info,
	unsigned int supported_io_num);

void set_log_enable(struct sched_info *sched_info, enum BOOL log_enable);

void set_log_window_size(struct sched_info *sched_info,
	unsigned int log_window_size);

void set_use_fcfs(struct sched_info *sched_info, enum BOOL use_fcfs);

int write_data(struct sched_info *sched_info);
