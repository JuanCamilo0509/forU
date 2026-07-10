#pragma once
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>

typedef struct {
  char wifi_ssid[32];
  char wifi_pass[64];
  char mqtt_url[64];
  char mqtt_user[32];
  char mqtt_pass[64];
  char topic[32];
  char anniversary[16];
} configuration_variables;

extern configuration_variables g_config;

int load_internal_variables(void);

static esp_err_t check_empty_string(esp_err_t res_lectura,
                                    const char *str_leido,
                                    const char *nombre_campo);

int8_t read_passed_from_nvs(void);

int save_passed_to_nvs(int8_t value);

void erase_variables(void);
