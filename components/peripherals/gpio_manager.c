#include "gpio_manager.h"
#include "esp_system.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/gpio_types.h"
#include "nvs_manager.h"

SemaphoreHandle_t reset_sem = NULL;
SemaphoreHandle_t wakeup_sem = NULL;

void IRAM_ATTR wakeUp_handler(void *arg) {
  BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(wakeup_sem, &pxHigherPriorityTaskWoken);
  if (pxHigherPriorityTaskWoken)
    portYIELD_FROM_ISR();
}

void IRAM_ATTR reset_handler(void *arg) {
  BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(reset_sem, &pxHigherPriorityTaskWoken);
  if (pxHigherPriorityTaskWoken)
    portYIELD_FROM_ISR();
}

volatile int wakeCounter = 0;

void wakeUp_task(void *arg) {
  for (;;) {
    if (xSemaphoreTake(wakeup_sem, portMAX_DELAY) == pdTRUE) {
      wakeCounter++;
      ESP_LOGI("GIROSCOPIO", "contador=%d", wakeCounter);
      vTaskDelay(pdMS_TO_TICKS(500));
    }
  }
}

void reset_task(void *arg) {
  for (;;) {
    if (xSemaphoreTake(reset_sem, portMAX_DELAY) == pdTRUE) {
      erase_variables();
      esp_restart();
    }
  }
}

void initGpios() {
  reset_sem = xSemaphoreCreateBinary();
  xTaskCreate(reset_task, "reset_task", 3072, NULL, 10, NULL);

  wakeup_sem = xSemaphoreCreateBinary();
  xTaskCreate(wakeUp_task, "wakeUp_task", 3072, NULL, 10, NULL);

  gpio_config_t io_conf = {0};
  io_conf.intr_type = GPIO_INTR_NEGEDGE;
  io_conf.pin_bit_mask = (1ULL << 16), io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
  gpio_config(&io_conf);

  io_conf.intr_type = GPIO_INTR_POSEDGE;
  io_conf.pin_bit_mask = (1ULL << 21), io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
  gpio_config(&io_conf);

  gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
  gpio_isr_handler_add(16, reset_handler, (void *)16);
  gpio_isr_handler_add(21, wakeUp_handler, (void *)21);
}
