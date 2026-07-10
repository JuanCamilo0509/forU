#pragma once
#include "otaImplementation.h"
#include "wifi.h"
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs_manager.h"
#include "oledScreen.h"
#include "sdkconfig.h"

#include "esp_crt_bundle.h"
#include "mqtt_client.h"

void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                        int32_t event_id, void *event_data);

esp_err_t mqtt_app_start(int wait);
void mqtt_disconnect(void);
