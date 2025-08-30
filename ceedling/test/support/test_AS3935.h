#ifndef TEST_AS3935_H
#define TEST_AS3935_H

#include <stddef.h>
#include <stdio.h>
#include "AS3935.h"

void log_timestamp(char *buffer, size_t size);
FILE *as3935_fopen(const char *path, const char *mode);

#endif
