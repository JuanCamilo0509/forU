#include "nvs_manager.h"

int8_t read_passed_from_nvs(void) {
  nvs_handle_t nvs_handle;
  int8_t value = 0;
  esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);

  if (err == ESP_OK) {
    err = nvs_get_i8(nvs_handle, "passed", &value);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
      ESP_LOGW("NVS_LOAD", "Passed isn't defined");
    } else if (err != ESP_OK) {
      ESP_LOGE("NVS_LOAD", "Error with the passed variable: %s",
               esp_err_to_name(err));
    }
    nvs_close(nvs_handle);
  } else {
    ESP_LOGE("NVS_LOAD", "Can't open the NVS namespace: %s",
             esp_err_to_name(err));
  }
  return value;
}

int save_passed_to_nvs(int8_t value) {
  nvs_handle_t nvs_handle;
  esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);

  if (err != ESP_OK) {
    ESP_LOGE("NVS_SAVE", "Can't open the NVS namespace: %s",
             esp_err_to_name(err));
    return 0;
  }

  err = nvs_set_i8(nvs_handle, "passed", value);
  if (err == ESP_OK) {
    err = nvs_commit(nvs_handle);
    if (err == ESP_OK) {
      ESP_LOGI("NVS_SAVE", "Passed saved");
      nvs_close(nvs_handle);
      return 1;
    } else {
      ESP_LOGE("NVS_SAVE", "Error committing to nvs: %s", esp_err_to_name(err));
    }
  } else {
    ESP_LOGE("NVS_SAVE", "Error setting value: %s", esp_err_to_name(err));
  }

  nvs_close(nvs_handle);
  return 0;
}

void erase_variables(void) {
  nvs_handle_t nvs_handle;
  if (nvs_open("storage", NVS_READWRITE, &nvs_handle) == ESP_OK) {
    nvs_set_str(nvs_handle, "wifi_ssid", "");
    nvs_set_str(nvs_handle, "wifi_pass", "");
    nvs_set_str(nvs_handle, "mqtt_url", "");
    nvs_set_str(nvs_handle, "mqtt_user", "");
    nvs_set_str(nvs_handle, "mqtt_pass", "");
    nvs_set_str(nvs_handle, "topic", "");
    nvs_set_str(nvs_handle, "anniversary", "");

    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
  }
}
