#include "log.h"
#include <stdio.h>

#define MAX_CALLBACKS 32

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

	callback cbs[MAX_CALLBACKS];
} logger;

int log_add_callback(log_callback func, FILE *writer, log_level lvl) {
	for (size_t i = 0; i < MAX_CALLBACKS; i++) {
		if (logger.cbs[i].func == NULL) {
			logger.cbs[i] = (callback){.func = func, .writer = writer, .lvl = lvl};
			return 0;
		}
	}

	return -1;
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

