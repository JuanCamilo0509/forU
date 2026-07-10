#pragma once
#include "captivePortal.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "nvs_manager.h"
#include "oledScreen.h"

#define INACTIVITY_TIMEOUT_MS (15 * 1000)
extern QueueHandle_t screenQueue;
extern volatile uint32_t last_activity_time;

void testProtocol(void);
void startCaptivePortal(void);
void reset_inactivity_timer(void);
void power_management_task(void *args);
