# log

Simple logging library.

Here's the example of using:
```c
#include <stdio.h>
#include <stdlib.h>
#include "log.h"

int main(int argc, char* argv[argc + 1]) {
	FILE* logfile = fopen("log.log", "a");
	FILE* json_logfile = fopen("log.json", "a");
	log_add_file(logfile, INFO);
	log_add_json_file(json_logfile, INFO);

	LOG_TRACE("something bad happened on line %d", 11);
	LOG_DEBUG("something bad happened");
	LOG_INFO("something bad happened");
	LOG_ERROR("something bad happened");
	LOG_FATAL("something bad happened");

	fclose(logfile);
	fclose(json_logfile);
	return EXIT_SUCCESS;
}
```

`To see colored output compile with USE_LOG_COLOR macro defined.` 
For example: clang -DUSE_LOG_COLOR -o <output> -Wall -Werror <source_files> log.c

stdout (NO COLORS):
```bash
15:46:36 TRACE <main.c:11> something bad happened on line 11
15:46:36 DEBUG <main.c:12> something bad happened
15:46:36 INFO  <main.c:13> something bad happened
15:46:36 ERROR <main.c:14> something bad happened
15:46:36 FATAL <main.c:15> something bad happened
```

log.log:
```
2024:05:08 15:46:36 INFO  <main.c:13> something bad happened
2024:05:08 15:46:36 ERROR <main.c:14> something bad happened
2024:05:08 15:46:36 FATAL <main.c:15> something bad happened
```

log.json:
```json
{ "datetime": "2024:05:08 15:46:36", "level": "INFO", "caller": "main.c:13", "message": "something bad happened" }
{ "datetime": "2024:05:08 15:46:36", "level": "ERROR", "caller": "main.c:14", "message": "something bad happened" }
{ "datetime": "2024:05:08 15:46:36", "level": "FATAL", "caller": "main.c:15", "message": "something bad happened" }
```

## Standard callbacks

Provide logging using standard or json format.

Available using `log_add_file(FILE*, log_level)` and `log_add_json_file(FILE*, log_level)` functions.

## Quiet mode
Turns off standart logging, leaving only callbacks defined by user.
```c
log_set_quiet(1);
```

## User-defined log callbacks

1. Define callback
```c
void log_file_callback(log* l) {
	char datetime[64];
	size_t datetime_len = strftime(datetime, sizeof(datetime), "%Y:%m:%d %H:%M:%S", l->time);
	datetime[datetime_len] = '\0';
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
```

2. Add callback
```c
FILE* somefile = fopen("somefile", "a");
log_add_callback(log_file_callback, somefile, INFO);

...

fclose(somefile);
```
