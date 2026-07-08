#include "esp_log.h"
#include "captivePortal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "nvs_manager.h"
#include "oledScreen.h"

extern QueueHandle_t screenQueue;

void testProtocol();
void startCaptivePortal();
