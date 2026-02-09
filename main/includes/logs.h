#ifndef LOGS_H
#define LOGS_H

#include "esp_log.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

void logs_init(void);
size_t logs_get_content(char* dest, size_t max_size);
size_t logs_get_count(void);
void logs_clear(void);

#endif