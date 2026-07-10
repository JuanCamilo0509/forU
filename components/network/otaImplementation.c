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
#include "string.h"

static esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
  switch (evt->event_id) {
  case HTTP_EVENT_ERROR:
    ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
    break;
  case HTTP_EVENT_ON_CONNECTED:
    ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
    break;
  case HTTP_EVENT_HEADER_SENT:
    ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
    break;
  case HTTP_EVENT_ON_HEADER:
    ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key,
             evt->header_value);
    break;
  case HTTP_EVENT_ON_HEADERS_COMPLETE:
    ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADERS_COMPLETE");
    break;
  case HTTP_EVENT_ON_DATA:
    ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
    break;
  case HTTP_EVENT_ON_FINISH:
    ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
    break;
  case HTTP_EVENT_DISCONNECTED:
    ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
    break;
  case HTTP_EVENT_REDIRECT:
    ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
    break;
  default:
    break;
  }
  return ESP_OK;
}

static void ota_task(void *pvParameter) {
  char *url = (char *)pvParameter;
  ESP_LOGI(TAG, "Iniciando descarga OTA desde: %s", url);

  esp_http_client_config_t config = {
      .url = url,
      .crt_bundle_attach = esp_crt_bundle_attach,
      .event_handler = _http_event_handler,
      .keep_alive_enable = true,
      .buffer_size = 4096,
      .disable_auto_redirect = false,
      .max_redirection_count = 5,
  };

  esp_https_ota_config_t ota_config = {
      .http_config = &config,
  };

  esp_err_t ret = esp_https_ota(&ota_config);
  if (ret == ESP_OK) {
    ESP_LOGW(TAG, "Everything Done");
    esp_restart();
  } else {
    ESP_LOGE(TAG, "Error(Fail to update the esp): %s", esp_err_to_name(ret));
  }

  free(url);
  vTaskDelete(NULL);
}

void start_ota_update(const char *download_url) {
  char *url_copy = malloc(strlen(download_url) + 1);
  if (url_copy == NULL) {
    ESP_LOGE(TAG, "Not enough memory");
    return;
  }
  strcpy(url_copy, download_url);

  xTaskCreate(&ota_task, "ota_task", 8192, (void *)url_copy, 5, NULL);
}
