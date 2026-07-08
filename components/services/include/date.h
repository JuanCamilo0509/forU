#pragma once
#include "esp_attr.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif_sntp.h"
#include "esp_sleep.h"
#include "esp_sntp.h"
#include "esp_system.h"
#include "lwip/ip_addr.h"
#include "nvs_flash.h"
#include <string.h>
#include <sys/time.h>
#include <time.h>

static const char *TAG_SNTP = "sntp";

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 48
#endif

static time_t timestamp_global = 0;

/* Variable holding number of times ESP32 restarted since first boot.
 * It is placed into RTC memory using RTC_DATA_ATTR and
 * maintains its value when ESP32 wakes from deep sleep.
 */

RTC_DATA_ATTR static int boot_count = 0;

static void sntp_event_handler(void *arg, esp_event_base_t base, int32_t id,
                               void *data);
static struct tm string2tm(char *str);

void init_sntp(void);
