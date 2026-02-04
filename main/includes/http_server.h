#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"

#include "esp_http_server.h"

void wifi_init_sta(void);

#endif