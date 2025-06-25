#ifndef LOG_H
#define LOG_H

#include <stddef.h>

void log_timestamp(char *buffer, size_t size);
FILE *as3935_fopen(const char *path, const char *mode);

#endif
