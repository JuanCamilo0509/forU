#include "otaImplementation.h"
#include "esp_crt_bundle.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "provisioning.h"
#include <stdbool.h>

extern TaskHandle_t power_task_handle;
volatile bool ota_update_in_progress = false;

void ota_set_update_in_progress(bool active) {
  ota_update_in_progress = active;
  ESP_LOGW(OTATAG, "OTA state changed: %s", active ? "in progress" : "idle");
}

bool ota_is_update_in_progress(void) { return ota_update_in_progress; }

static void ota_log_partition_state(const char *stage) {
  const esp_partition_t *running = esp_ota_get_running_partition();
  const esp_partition_t *boot = esp_ota_get_boot_partition();
  const esp_partition_t *next = esp_ota_get_next_update_partition(NULL);

  ESP_LOGI(OTATAG, "[%s] running=%s@0x%lx boot=%s@0x%lx next=%s@0x%lx", stage,
           running ? running->label : "none",
           running ? (unsigned long)running->address : 0,
           boot ? boot->label : "none", boot ? (unsigned long)boot->address : 0,
           next ? next->label : "none",
           next ? (unsigned long)next->address : 0);
}

static esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
  switch (evt->event_id) {
  case HTTP_EVENT_ERROR:
    ESP_LOGD(OTATAG, "HTTP_EVENT_ERROR");
    break;
  case HTTP_EVENT_ON_CONNECTED:
    ESP_LOGD(OTATAG, "HTTP_EVENT_ON_CONNECTED");
    break;
  case HTTP_EVENT_HEADER_SENT:
    ESP_LOGD(OTATAG, "HTTP_EVENT_HEADER_SENT");
    break;
  case HTTP_EVENT_ON_HEADER:
    ESP_LOGD(OTATAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key,
             evt->header_value);
    break;
  case HTTP_EVENT_ON_HEADERS_COMPLETE:
    ESP_LOGD(OTATAG, "HTTP_EVENT_ON_HEADERS_COMPLETE");
    break;
  case HTTP_EVENT_ON_DATA:
    ESP_LOGD(OTATAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
    break;
  case HTTP_EVENT_ON_FINISH:
    ESP_LOGD(OTATAG, "HTTP_EVENT_ON_FINISH");
    break;
  case HTTP_EVENT_DISCONNECTED:
    ESP_LOGD(OTATAG, "HTTP_EVENT_DISCONNECTED");
    break;
  case HTTP_EVENT_REDIRECT:
    ESP_LOGD(OTATAG, "HTTP_EVENT_REDIRECT");
    break;
  default:
    break;
  }
  return ESP_OK;
}

static void ota_task(void *pvParameter) {
  char *url = (char *)pvParameter;
  ESP_LOGI(OTATAG, "OTA task started");
  ESP_LOGI(OTATAG, "Downloading firmware from: %s", url);
  ota_set_update_in_progress(true);
  reset_inactivity_timer();
  ota_log_partition_state("before_download");

  esp_http_client_config_t config = {
      .url = url,
      .crt_bundle_attach = esp_crt_bundle_attach,
      .event_handler = _http_event_handler,
      .keep_alive_enable = true,
      .buffer_size = 8192,
      .buffer_size_tx = 1024,
      .disable_auto_redirect = false,
      .max_redirection_count = 5,
  };

  esp_https_ota_config_t ota_config = {
      .http_config = &config,
  };

  esp_err_t ret = esp_https_ota(&ota_config);
  if (ret == ESP_OK) {
    ESP_LOGI(OTATAG, "OTA completed successfully, rebooting now");
    ota_log_partition_state("after_success");
    esp_restart();
  } else {
    ESP_LOGE(OTATAG, "OTA failed: %s", esp_err_to_name(ret));
    ota_log_partition_state("after_failure");
    ota_set_update_in_progress(false);
    reset_inactivity_timer();
    if (power_task_handle != NULL) {
      vTaskResume(power_task_handle);
      ESP_LOGW(OTATAG, "Sleep monitor resumed after OTA failure");
    }
  }

  free(url);
  ESP_LOGI(OTATAG, "OTA task finished");
  vTaskDelete(NULL);
}

void start_ota_update(const char *download_url) {
  if (download_url == NULL || download_url[0] == '\0') {
    ESP_LOGE(OTATAG, "OTA request ignored: empty URL");
    ESP_LOGI(OTATAG, "%s", download_url);
    return;
  }

  if (ota_is_update_in_progress()) {
    ESP_LOGW(OTATAG, "OTA request ignored: update already in progress");
    return;
  }

  char *url_copy = malloc(strlen(download_url) + 1);
  if (url_copy == NULL) {
    ESP_LOGE(OTATAG, "Not enough memory");
    return;
  }
  strcpy(url_copy, download_url);

  ESP_LOGI(OTATAG, "Queueing OTA update from: %s", download_url);
  xTaskCreate(&ota_task, "ota_task", 8192, (void *)url_copy, 5, NULL);
}
