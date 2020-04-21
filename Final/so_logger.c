#include "so_logger.h"
#include <string.h>

struct so_logger *initialize_logger(unsigned int window_size)
{
	struct so_logger *logger;
	time_t crt_time;
	char *log_name;
	size_t len;
#ifdef _WIN32
	unsigned int i;
#endif

	log_name = (char *) calloc(MESSAGE_SIZE, sizeof(char));
	if (log_name == NULL)
		return INVALID_LOGGER;

	logger = (struct so_logger *) malloc(sizeof(struct so_logger));
	if (logger == NULL) {
		free(log_name);
		return INVALID_LOGGER;
	}

	logger->window_size = window_size;
	strcat(log_name, "logger-");

	crt_time = time(NULL);
	strcat(log_name, ctime(&crt_time));
	len = strlen(log_name);
	log_name[len - 1] = '\0';

#ifdef _WIN32
	for (i = 0; i < len; i++)
		if (log_name[i] == ':')
			log_name[i] = ';';
	strcat(log_name, ".txt");
#endif

	logger->log_file = fopen(log_name, "w");

	if (logger->log_file == NULL) {
		free(log_name);
		free(logger);
		return INVALID_LOGGER;
	}

	logger->log_queue = initialize_queue();
	if (logger->log_queue == (struct queue *)MALLOC_NULL_POINTER) {
		free(log_name);
		fclose(logger->log_file);
		free(logger);
		return INVALID_LOGGER;
	}

	logger->crt_size = 0;
	free(log_name);
	return logger;
}

static void process_message(struct so_logger *logger, char *message)
{
	char *msg;

	insert_into_queue((struct so_thread *)message, logger->log_queue);
	logger->crt_size++;

	msg = NULL;
	if (logger->crt_size == logger->window_size + 1)
		msg = (char *)pop_queue(logger->log_queue);

	if (msg != NULL) {
		logger->crt_size--;
		free(msg);
	}
}

static void append_num(char *string, unsigned long number)
{
	unsigned int len;
	unsigned long mirrored;

	len = strlen(string);
	if (number == 0) {
		string[len] = '0';
		string[len + 1] = '\0';
		return;
	}

	mirrored = 0;
	while (number) {
		mirrored = mirrored * 10 + number % 10;
		number /= 10;
	}

	while (mirrored) {
		string[len] = mirrored % 10 + '0';
		mirrored /= 10;
		len++;
	}

	string[len] = '\0';
}

void log_fork(struct so_logger *logger,
	      tid_t thread_id,
	      tid_t child,
	      unsigned int priority)
{
	char *log_message;

	log_message = (char *)calloc(MESSAGE_SIZE, sizeof(char));
	if (log_message == NULL)
		return;

	append_num(log_message, thread_id);
	strcat(log_message, " forked thread ");
	append_num(log_message, child);
	strcat(log_message, " with priority ");
	append_num(log_message, priority);

	process_message(logger, log_message);
}

void log_terminate(struct so_logger *logger, tid_t thread_id)
{
	char *log_message;

	log_message = (char *)calloc(MESSAGE_SIZE, sizeof(char));
	if (log_message == NULL)
		return;

	append_num(log_message, thread_id);
	strcat(log_message, " terminated");

	process_message(logger, log_message);
}

void log_preempt(struct so_logger *logger, tid_t thread_id)
{
	char *log_message;

	log_message = (char *)calloc(MESSAGE_SIZE, sizeof(char));
	if (log_message == NULL)
		return;

	append_num(log_message, thread_id);
	strcat(log_message, " was preempted");

	process_message(logger, log_message);
}

void log_exec(struct so_logger *logger, tid_t thread_id)
{
	char *log_message;

	log_message = (char *)calloc(MESSAGE_SIZE, sizeof(char));
	if (log_message == NULL)
		return;

	append_num(log_message, thread_id);
	strcat(log_message, " executed");

	process_message(logger, log_message);
}

void log_wait(struct so_logger *logger, tid_t thread_id, unsigned int io)
{
	char *log_message;

	log_message = (char *)calloc(MESSAGE_SIZE, sizeof(char));
	if (log_message == NULL)
		return;

	append_num(log_message, thread_id);
	strcat(log_message, " waits after ");
	append_num(log_message, io);

	process_message(logger, log_message);
}

void log_signal(struct so_logger *logger, tid_t thread_id, unsigned int io)
{
	char *log_message;

	log_message = (char *)calloc(MESSAGE_SIZE, sizeof(char));
	if (log_message == NULL)
		return;

	append_num(log_message, thread_id);
	strcat(log_message, " signals ");
	append_num(log_message, io);

	process_message(logger, log_message);
}

void log_error(struct so_logger *logger, const char *msg)
{
	char *log_message;

	log_message = (char *)calloc(MESSAGE_SIZE, sizeof(char));
	if (log_message == NULL)
		return;

	strcpy(log_message, msg);
	process_message(logger, log_message);
}

void flush_log(struct so_logger *logger)
{
	unsigned int i;
	char *log_message;
	struct q_node *crt_node;

	crt_node = logger->log_queue->beginning;
	for (i = 0; i < logger->crt_size; i++)
		if (crt_node != NULL) {
			log_message = (char *) crt_node->info;
			if (log_message != INVALID_POP_INFO)
				fprintf(logger->log_file, "%s\n", log_message);
			crt_node = crt_node->next;
		} else
			break;

	fseek(logger->log_file, 0, SEEK_SET);
}

void destroy_logger(struct so_logger *logger)
{
	unsigned int i;
	char *msg;
	struct q_node *crt_node;

	flush_log(logger);
	fclose(logger->log_file);

	crt_node = logger->log_queue->beginning;
	for (i = 0; i < logger->crt_size; i++)
		if (crt_node != NULL) {
			msg = (char *) crt_node->info;
			if (msg != INVALID_POP_INFO)
				free(msg);
			crt_node = crt_node->next;
		} else
			break;
	destroy_queue(logger->log_queue);
	free(logger);
}
