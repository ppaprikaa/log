#pragma once

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <time.h>

enum LogLevel {
	TRACE, DEBUG, INFO, WARN, ERROR, FATAL
};

typedef struct {
	// writing destination
	FILE* writer;
	// log information
	int lvl;
	struct tm *time;
	// caller information
	char* caller_file;
	int caller_line;
	// message and arguments
	char* fmt;
	va_list args;
} log;

typedef void (*log_callback)(log *l);

void log_log(int level, char *file, int line, char *fmt, ...);

#endif
