#include "mqtt.h"

const char *MqtTAG = "MQTT conectado";
extern QueueHandle_t screenQueue;
extern configuration_variables g_config;
esp_mqtt_client_handle_t client;
static EventGroupHandle_t s_mqtt_event_group;
#define MQTT_CONNECTED_BIT BIT0
#define MQTT_FAIL_BIT BIT1

void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                        int32_t event_id, void *event_data) {
  esp_mqtt_event_handle_t event = event_data;
  client = event->client;
  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(MqtTAG, "MQTT_EVENT_CONNECTED");

    esp_mqtt_client_subscribe(client, g_config.topic, 0);
    esp_mqtt_client_subscribe(client, g_config.mqtt_user, 0);

    if (s_mqtt_event_group) {
      xEventGroupSetBits(s_mqtt_event_group, MQTT_CONNECTED_BIT);
    }
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGW(MqtTAG, "MQTT_EVENT_DISCONNECTED");
    if (s_mqtt_event_group) {
      xEventGroupSetBits(s_mqtt_event_group, MQTT_FAIL_BIT);
    }
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGE(MqtTAG, "MQTT_EVENT_ERROR");
    if (s_mqtt_event_group) {
      xEventGroupSetBits(s_mqtt_event_group, MQTT_FAIL_BIT);
    }
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI(MqtTAG, "MQTT_EVENT_DATA (%d)", event->data_len);
    screen_event_t screenEvent;
    int size = event->data_len;
    memcpy(screenEvent.text, event->data, size);
    screenEvent.text[size] = '\0';

    if (strncmp(event->topic, g_config.mqtt_user, event->topic_len) == 0) {
      screenEvent.type = USR1;
      ESP_LOGI(MqtTAG, "USR1");
    }

    if (strncmp(event->topic, g_config.topic, event->topic_len) == 0) {
      screenEvent.type = USR2;
      ESP_LOGI(MqtTAG, "USR2");
    }

    xQueueSend(screenQueue, &screenEvent, 0);

    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);
    break;
  default:
    ESP_LOGI(MqtTAG, "Other event id:%d", event->event_id);
    break;
  }
}

esp_err_t mqtt_app_start(int wait) {
  char formatUri[128];
  snprintf(formatUri, sizeof(formatUri), "mqtts://%s", g_config.mqtt_url);

  const esp_mqtt_client_config_t mqtt_cfg = {
      .broker =
          {
              .address.uri = formatUri,
              .verification.crt_bundle_attach = esp_crt_bundle_attach,
          },
      .credentials = {.username = g_config.mqtt_user,
                      .authentication.password = g_config.mqtt_pass}};

  if (wait) {
    s_mqtt_event_group = xEventGroupCreate();
  } else {
    s_mqtt_event_group = NULL;
  }

  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler,
                                 NULL);

  esp_mqtt_client_start(client);

  if (!wait)
    return ESP_OK;

  EventBits_t bits = xEventGroupWaitBits(s_mqtt_event_group,
                                         MQTT_CONNECTED_BIT | MQTT_FAIL_BIT,
                                         pdFALSE, pdFALSE, pdMS_TO_TICKS(8000));
  esp_err_t result = ESP_FAIL;
  if (bits & MQTT_CONNECTED_BIT) {
    ESP_LOGI(MqtTAG, "¡Broker MQTT check!");
    result = ESP_OK;
  } else {
    ESP_LOGE(MqtTAG, "Error or Timeout connecting to mqtt broker.");
    esp_mqtt_client_stop(client);
    esp_mqtt_client_destroy(client);
    result = ESP_FAIL;
  }

  vEventGroupDelete(s_mqtt_event_group);
  s_mqtt_event_group = NULL;

  return result;
}

void mqtt_disconnect(void) {
  if (client != NULL) {
    esp_err_t err = esp_mqtt_client_stop(client);
    if (err == ESP_OK) {
      ESP_LOGI(MqtTAG, "Client closed");
    }
    err = esp_mqtt_client_destroy(client);
    if (err == ESP_OK) {
      ESP_LOGI(MqtTAG, "Free of mqtt resources");
      client = NULL;
    }
  }
}
