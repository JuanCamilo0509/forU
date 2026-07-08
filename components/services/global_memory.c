#include "global_memory.h"

int load_internal_variables() {
  nvs_handle_t nvs_handle;
  esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
  if (err != ESP_OK) {
    ESP_LOGE("NVS_LOAD", "Can't open the nvs");
    return 0;
  }

  size_t size_ssid = sizeof(g_config.wifi_ssid);
  size_t size_wpass = sizeof(g_config.wifi_pass);
  size_t size_murl = sizeof(g_config.mqtt_url);
  size_t size_muser = sizeof(g_config.mqtt_user);
  size_t size_mpass = sizeof(g_config.mqtt_pass);
  size_t size_topic = sizeof(g_config.topic);
  size_t size_anni = sizeof(g_config.anniversary);

  esp_err_t r_ssid, r_wpass, r_murl, r_muser, r_mpass, r_topic, r_anni;
  esp_err_t result = ESP_OK;

  r_ssid = nvs_get_str(nvs_handle, "wifi_ssid", g_config.wifi_ssid, &size_ssid);
  r_wpass =
      nvs_get_str(nvs_handle, "wifi_pass", g_config.wifi_pass, &size_wpass);
  r_murl = nvs_get_str(nvs_handle, "mqtt_url", g_config.mqtt_url, &size_murl);
  r_muser =
      nvs_get_str(nvs_handle, "mqtt_user", g_config.mqtt_user, &size_muser);
  r_mpass =
      nvs_get_str(nvs_handle, "mqtt_pass", g_config.mqtt_pass, &size_mpass);
  r_topic = nvs_get_str(nvs_handle, "topic", g_config.topic, &size_topic);
  r_anni =
      nvs_get_str(nvs_handle, "anniversary", g_config.anniversary, &size_anni);

  nvs_close(nvs_handle);

  result |= check_empty_string(r_ssid, g_config.wifi_ssid, "wifi_ssid");
  result |= check_empty_string(r_wpass, g_config.wifi_pass, "wifi_pass");
  result |= check_empty_string(r_murl, g_config.mqtt_url, "mqtt_url");
  result |= check_empty_string(r_muser, g_config.mqtt_user, "mqtt_user");
  result |= check_empty_string(r_mpass, g_config.mqtt_pass, "mqtt_pass");
  result |= check_empty_string(r_topic, g_config.topic, "topic");
  result |= check_empty_string(r_anni, g_config.anniversary, "anniversary");

  if (result != ESP_OK) {
    ESP_LOGE("NVS_LOAD", "Some variables aren't set yet");
    return 0;
  }

  ESP_LOGI("NVS_LOAD", "=== Variables Internas Cargadas ===");
  ESP_LOGI("NVS_LOAD", "  WiFi SSID:     %s", g_config.wifi_ssid);
  ESP_LOGI("NVS_LOAD", "  WiFi Password: %s", g_config.wifi_pass);
  ESP_LOGI("NVS_LOAD", "  MQTT URL:      %s", g_config.mqtt_url);
  ESP_LOGI("NVS_LOAD", "  MQTT User:     %s", g_config.mqtt_user);
  ESP_LOGI("NVS_LOAD", "  MQTT Pass:     %s", g_config.mqtt_pass);
  ESP_LOGI("NVS_LOAD", "  MQTT Topic:    %s", g_config.topic);
  ESP_LOGI("NVS_LOAD", "  Anniversary:   %s", g_config.anniversary);
  ESP_LOGI("NVS_LOAD", "===================================");

  ESP_LOGI("NVS_LOAD", "All set");
  return 1;
}

static esp_err_t check_empty_string(esp_err_t res_lectura,
                                    const char *str_leido,
                                    const char *nombre_campo) {
  if (res_lectura != ESP_OK) {
    return res_lectura;
  }
  if (strlen(str_leido) == 0) {
    ESP_LOGW("NVS_LOAD", "El campo '%s' existe en NVS pero esta VACIO.",
             nombre_campo);
    return ESP_ERR_INVALID_STATE;
  }
  return ESP_OK;
}
