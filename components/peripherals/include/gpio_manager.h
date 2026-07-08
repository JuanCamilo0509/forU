#pragma once
#define ESP_INTR_FLAG_DEFAULT 0

#include "driver/gpio.h"

void initGpios();

void gpio_isr_handler(void *arg);

void reset_task(void *arg);
