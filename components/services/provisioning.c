#include "provisioning.h"

void testProtocol() {
  screen_event_t event;
  strcpy(event.text, "Testing Wifi");
  xQueueSend(screenQueue, &event, portMAX_DELAY);

  if (wifi_init_sta() != ESP_OK) {
    strcpy(event.TAG, "WIFI");
    event.type = ERROR;
    strcpy(event.text, "Cannot connect to network");
    xQueueSend(screenQueue, &event, portMAX_DELAY);
    startCaptivePortal();
    return;
  }

  strcpy(event.TAG, "WIFI");
  event.type = OK_WIFI;
  xQueueSend(screenQueue, &event, portMAX_DELAY);

  if (mqtt_app_start(1) != ESP_OK) {
    strcpy(event.TAG, "MQTT");
    event.type = ERROR;
    strcpy(event.text, "Cannot connect to server");
    xQueueSend(screenQueue, &event, portMAX_DELAY);
    startCaptivePortal();
    return;
  }

  strcpy(event.TAG, "SERVER");
  event.type = OK_MQTT;
  xQueueSend(screenQueue, &event, portMAX_DELAY);

  if (save_passed_to_nvs(1)) {
    vTaskDelay(pdMS_TO_TICKS(2000));
    esp_restart();
  } else {
    startCaptivePortal();
  }
}

void startCaptivePortal() {
  esp_wifi_disconnect();
  esp_wifi_stop();
  esp_wifi_deinit();

  wifi_init_softap();
  captivePortal();
}
