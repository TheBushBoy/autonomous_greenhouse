/**
* Captures logs into circular buffer for HTTP access
* Doc : https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/log.html#_CPPv419esp_log_set_vprintf14vprintf_like_t
*/

#include "logs.h"

#define LOG_BUFFER_SIZE 8192

static const char* TAG = "LOGS";

typedef struct {
    char buffer[LOG_BUFFER_SIZE];
    size_t head;
    size_t tail;
    size_t count;
    SemaphoreHandle_t mutex;
} log_buffer_t;

static log_buffer_t log_buffer = {0};

static int log_vprintf(const char *fmt, va_list args) {
    int ret = vprintf(fmt, args); // Print to UART as usual
    
    if (log_buffer.mutex && xSemaphoreTake(log_buffer.mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        char temp[256];
        int len = vsnprintf(temp, sizeof(temp), fmt, args);
        
        if (len > 0 && len < sizeof(temp)) {
            for (int i = 0; i < len; i++) {
                log_buffer.buffer[log_buffer.head] = temp[i];
                log_buffer.head = (log_buffer.head + 1) % LOG_BUFFER_SIZE;
                
                // Overwrite oldest data if buffer full
                if (log_buffer.count < LOG_BUFFER_SIZE) {
                    log_buffer.count++;
                } else {
                    log_buffer.tail = (log_buffer.tail + 1) % LOG_BUFFER_SIZE;
                }
            }
        }
        
        xSemaphoreGive(log_buffer.mutex);
    }
    
    return ret;
}

void logs_init(void) {
    log_buffer.mutex = xSemaphoreCreateMutex();
    log_buffer.head = 0;
    log_buffer.tail = 0;
    log_buffer.count = 0;
    
    esp_log_set_vprintf(log_vprintf);
    
    ESP_LOGI(TAG, "Log capture initialized (buffer: %d bytes)", LOG_BUFFER_SIZE);
}

size_t logs_get_content(char* dest, size_t max_size) {
    if (!log_buffer.mutex || !dest || max_size == 0) {
        return 0;
    }
    
    size_t copied = 0;
    
    if (xSemaphoreTake(log_buffer.mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (log_buffer.count > 0) {
            size_t pos = log_buffer.tail;
            size_t to_copy = (log_buffer.count < max_size - 1) ? log_buffer.count : max_size - 1;
            
            for (size_t i = 0; i < to_copy; i++) {
                dest[copied++] = log_buffer.buffer[pos];
                pos = (pos + 1) % LOG_BUFFER_SIZE;
            }
            
            dest[copied] = '\0';
        }
        
        xSemaphoreGive(log_buffer.mutex);
    }
    
    return copied;
}

size_t logs_get_count(void) {
    size_t count = 0;
    
    if (log_buffer.mutex && xSemaphoreTake(log_buffer.mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        count = log_buffer.count;
        xSemaphoreGive(log_buffer.mutex);
    }
    
    return count;
}

void logs_clear(void) {
    if (log_buffer.mutex && xSemaphoreTake(log_buffer.mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        log_buffer.head = 0;
        log_buffer.tail = 0;
        log_buffer.count = 0;
        xSemaphoreGive(log_buffer.mutex);
        
        ESP_LOGI(TAG, "Logs cleared");
    }
}