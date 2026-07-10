#pragma once
#include "dns_server.h"
#include "nvs_manager.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/err.h"
#include "lwip/inet.h"
#include "lwip/sys.h"
#include "nvs_flash.h"
#include <string.h>

#define EXAMPLE_ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY CONFIG_ESP_MAXIMUM_RETRY

// Station
void sta_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
                       void *event_data);
esp_err_t wifi_init_sta(void);

// SoftAp
void softap_event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data);
void wifi_init_softap(void);
