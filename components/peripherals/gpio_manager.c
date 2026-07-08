#include "gpio_manager.h"
#include "nvs_manager.h"

extern SemaphoreHandle_t reset_sem;

void IRAM_ATTR gpio_isr_handler(void *arg) {
  BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(reset_sem, &pxHigherPriorityTaskWoken);
  if (pxHigherPriorityTaskWoken)
    portYIELD_FROM_ISR();
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

  // Variable reset execution
  gpio_config_t io_conf = {.intr_type = GPIO_INTR_NEGEDGE,
                           .pin_bit_mask = (1ULL << 27),
                           .mode = GPIO_MODE_INPUT,
                           .pull_up_en = GPIO_PULLUP_ENABLE,
                           .pull_down_en = GPIO_PULLDOWN_DISABLE};

  // INT1 to turn on the micro when slepping
  // gpio_config_t io_conf2 = {.intr_type = GPIO_INTR_POSEDGE,
  //                           .pin_bit_mask = (1ULL << 16),
  //                           .mode = GPIO_MODE_INPUT,
  //                           .pull_up_en = GPIO_PULLUP_ENABLE,
  //                           .pull_down_en = GPIO_PULLDOWN_DISABLE};
  gpio_config(&io_conf);

  gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
  gpio_isr_handler_add(27, gpio_isr_handler, (void *)27);
}
