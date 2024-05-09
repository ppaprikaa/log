#include "log.h"

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>

#define MAX_LOG_CALLBACKS 32

#ifndef __STDC_NO_ATOMICS__
#include "stdatomic.h"
#endif

#ifdef USE_LOG_COLOR

#define DEFAULT_COLOR "\e[0;0m"
#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define PUR "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"

const char *level_colors[] = {
	CYN, BLU, GRN, YEL, RED, PUR
};

#endif

const char *level_strings[] = {
	"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

const char* log_level_string(log_level lvl) {
	return level_strings[lvl];
}

static void lock();
static void unlock();
void log_std_callback(log* l);
void log_std_file_callback(log* l);
void log_std_json_callback(log* l);

typedef struct {
	log_callback func;
	log_level lvl;
	FILE *writer;
} callback;

static struct {
	// locking
#ifdef __STDC_NO_ATOMICS__
	int lock;
#else
	atomic_int lock;
#endif

	int quiet;
	log_level lvl;
	callback cbs[MAX_LOG_CALLBACKS];
	fatal_log_callback fatal_cb;
} logger;

void init_log(log* l, FILE* writer) {
	if (!l->time) {
		time_t t = time(NULL);
		l->time = localtime(&t);
	}
	l->writer = writer;
}

void log_log(log_level level, char *file, int line, char *fmt, ...) {
	log l = {
		.lvl = level,
		.caller_file = file,
		.caller_line = line,
		.fmt = fmt,
	};

	lock();

	if (level < ERROR) init_log(&l, stdout);
	else init_log(&l, stderr);
	if (!logger.quiet && level >= logger.lvl) {
		va_start(l.args, fmt);
		log_std_callback(&l);
		va_end(l.args);
	}

	for (size_t i = 0; i < MAX_LOG_CALLBACKS && logger.cbs[i].func; i++) {
		callback* cb = &logger.cbs[i];
		if (level >= cb->lvl) {
			l.writer = cb->writer;
			va_start(l.args, fmt);
			cb->func(&l);
			va_end(l.args);
		}
	}

	if (level >= FATAL && logger.fatal_cb) {
		logger.fatal_cb();
	}

	unlock();
}

int log_add_callback(log_callback func, FILE *writer, log_level lvl) {
	for (size_t i = 0; i < MAX_LOG_CALLBACKS; i++) {
		if (!logger.cbs[i].func) {
			logger.cbs[i] = (callback){.func = func, .writer = writer, .lvl = lvl};
			return 0;
		}
	}

	return -1;
}

void log_add_fatal_log_callback(fatal_log_callback func) {
	logger.fatal_cb = func;	
}

int log_add_file(FILE *f, log_level lvl) {
	return log_add_callback(log_std_file_callback, f, lvl);
}

int log_add_json_file(FILE *f, log_level lvl) {
	return log_add_callback(log_std_json_callback, f, lvl);
}

void log_std_callback(log* l) {
	char time[16];
	time[strftime(time, sizeof(time), "%H:%M:%S", l->time)] = '\0';
#ifdef USE_LOG_COLOR
	fprintf(
			l->writer, 
			"%s %s%-5s%s <%s:%d> ",
			time,
			level_colors[l->lvl], level_strings[l->lvl], DEFAULT_COLOR,
			l->caller_file, l->caller_line
			);
#else
	fprintf(
			l->writer, 
			"%s %-5s <%s:%d> ",
			time,
			level_strings[l->lvl],
			l->caller_file, l->caller_line
			);
#endif
	vfprintf(l->writer, l->fmt, l->args);
	fprintf(l->writer, "\n");
	fflush(l->writer);
}

void log_std_file_callback(log* l) {
	char datetime[64];
	datetime[strftime(datetime, sizeof(datetime), "%Y:%m:%d %H:%M:%S", l->time)] = '\0';
	fprintf(
			l->writer, 
			"%s %-5s <%s:%d> ",
			datetime,
			level_strings[l->lvl],
			l->caller_file, l->caller_line
			);
	vfprintf(l->writer, l->fmt, l->args);
	fprintf(l->writer, "\n");
	fflush(l->writer);
}

void log_std_json_callback(log* l) {
	char datetime[64];
	datetime[strftime(datetime, sizeof(datetime), "%Y:%m:%d %H:%M:%S", l->time)] = '\0';
	int message_len = vsnprintf(NULL, 0, l->fmt, l->args);
	char *message = malloc(message_len + 1);
	vsnprintf(message, message_len + 1, l->fmt, l->args);
	fprintf(
			l->writer, 
			"{ \"datetime\": \"%s\", \"level\": \"%s\", \"caller\": \"%s:%d\", \"message\": \"%s\" }\n",
			datetime,
			level_strings[l->lvl],
			l->caller_file, l->caller_line,
			message
			);
	fflush(l->writer);

	free(message);
}

void log_set_quiet(int quiet) {
	logger.quiet = quiet;
}

void log_set_level(log_level lvl) {
	logger.lvl = lvl;
}

// in the absence of atomics, I want to provide at least some guarantees -.-
void lock() {
#ifdef __STDC_NO_ATOMICS__
	while (logger.lock);
	logger.lock = 1;
#else
	while(atomic_exchange(&logger.lock, 1));
#endif
}

void unlock() {
#ifdef __STDC_NO_ATOMICS__
	logger.lock = 0;
#else
	atomic_store(&logger.lock, 0);
#endif
}

