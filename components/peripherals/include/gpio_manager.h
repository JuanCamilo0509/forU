#pragma once
#define ESP_INTR_FLAG_DEFAULT 0

#include "driver/gpio.h"

void initGpios();

void reset_handler(void *arg);

void reset_task(void *arg);
