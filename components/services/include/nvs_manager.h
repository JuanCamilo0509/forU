#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include <nvs_flash.h>

int8_t read_passed_from_nvs(void);

int save_passed_to_nvs(int8_t value);

void erase_variables(void);
