#include "esp_crt_bundle.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "string.h"

static const char *TAG = "OTA_SERVICE";

static esp_err_t _http_event_handler(esp_http_client_event_t *evt);

static void ota_task(void *pvParameter);

void start_ota_update(const char *download_url);
