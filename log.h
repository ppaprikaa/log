#pragma once

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <time.h>

typedef enum {
	TRACE, DEBUG, INFO, WARN, ERROR, FATAL
} log_level;

#define LOG_TRACE(...) log_log(TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(...) log_log(DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...) log_log(INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...) log_log(WARN, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) log_log(ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) log_log(FATAL, __FILE__, __LINE__, __VA_ARGS__)

typedef struct {
	// writing destination
	FILE* writer;
	// log information
	log_level lvl;
	struct tm *time;
	// caller information
	char* caller_file;
	int caller_line;
	// message and arguments
	char* fmt;
	va_list args;
} log;

typedef void (*log_callback)(log *l);

void log_log(log_level level, char *file, int line, char *fmt, ...);

#endif
