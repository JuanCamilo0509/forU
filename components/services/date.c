#include "date.h"
#include "oledScreen.h"

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 48
#endif

/* Variable holding number of times ESP32 restarted since first boot.
 * It is placed into RTC memory using RTC_DATA_ATTR and
 * maintains its value when ESP32 wakes from deep sleep.
 */

extern QueueHandle_t screenQueue;

static void sntp_event_handler(void *arg, esp_event_base_t base, int32_t id,
                               void *data) {
  time_t now;
  time(&now);

  nvs_handle_t nvs_handle;
  char time[16];
  size_t timeSize = sizeof(time);
  esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);

  if (err == ESP_OK) {
    err = nvs_get_str(nvs_handle, "anniversary", time, &timeSize);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
      ESP_LOGW("NVS_LOAD", "DATE NOT FOUND");
    } else if (err != ESP_OK) {
      ESP_LOGE("NVS_LOAD", "Error getting anniversary", esp_err_to_name(err));
    }
    nvs_close(nvs_handle);
  } else {
    ESP_LOGE("NVS_LOAD", "Can't open the NVS namespace: %s",
             esp_err_to_name(err));
  }

  ESP_LOGD("DATE", "%s", time);
  struct tm *timeinfo = localtime(&now);
  time_t local_now = mktime(timeinfo);

  struct tm anniversary = string2tm(time);
  ESP_LOGD("DATE", "%d-%d-%d", anniversary.tm_year, anniversary.tm_mon,
           anniversary.tm_mday);

  time_t aniversary_time = mktime((struct tm *)&anniversary);

  double seg_dif = difftime(local_now, aniversary_time);
  int days = abs((int)(seg_dif / 86400));

  screen_event_t screenEvent;
  screenEvent.type = DAY_CHANGE;
  snprintf(screenEvent.text, sizeof(screenEvent.text), "%d", days);
  xQueueSend(screenQueue, &screenEvent, 0);
}

void init_sntp(void) {
  ESP_ERROR_CHECK(esp_event_handler_register(
      NETIF_SNTP_EVENT, NETIF_SNTP_TIME_SYNC, &sntp_event_handler, NULL));

  esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");

  setenv("TZ", "COT5", 1);
  tzset();

  esp_netif_sntp_init(&config);
}

static struct tm string2tm(char *str) {
  // format is year-month-day
  struct tm time = {0};
  int year = 0, month = 0, day = 0;
  if (sscanf(str, "%d-%d-%d", &year, &month, &day) == 3) {
    time.tm_year = year - 1900;
    time.tm_mon = month - 1;
    time.tm_mday = day;
  } else {
    printf("Bad formating");
  }

  return time;
}
