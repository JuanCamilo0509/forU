#pragma once
#include "date.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "global_memory.h"
#include "mqtt.h"
#include "sdkconfig.h"
#include "u8g2.h"
#include "u8g2_demo.h"

/* I2C Configuration (configurable via menuconfig) */
#define I2C_MASTER_NUM I2C_NUM_0 /*!< I2C master port number */
#define I2C_MASTER_SDA_IO                                                      \
  CONFIG_I2C_MASTER_SDA /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_SCL_IO                                                      \
  CONFIG_I2C_MASTER_SCL /*!< GPIO number used for I2C master clock */
#define I2C_FREQ_HZ                                                            \
  CONFIG_I2C_MASTER_FREQUENCY /*!< I2C master clock frequency */
#define I2C_TIMEOUT_MS 1000   /*!< I2C master timeout */

/* Display Configuration (configurable via menuconfig) */
#define I2C_DISPLAY_ADDRESS                                                    \
  CONFIG_I2C_DISPLAY_ADDRESS /*!< Display I2C address */

static const char *U8G2 = "U8G2";

typedef enum {
  UP,
  DOWN,
  USR1,
  USR2,
  SELECT,
  DAY_CHANGE,
  ERROR,
  OK_WIFI,
  OK_MQTT,
  UPDATE
} event_type;
extern time_t timestamp_global;

typedef struct {
  char TAG[16];
  event_type type;
  char text[128];
} screen_event_t;

uint8_t u8x8_byte_i2c_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int,
                         void *arg_ptr);

uint8_t u8x8_gpio_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int,
                           void *arg_ptr);

void iconScreenTask(void *pvParameters);

void screenInit(void);
void screenPowerSave(void);

void u8g2_ClearSection(u8g2_t *u8g2, int x, int y, int w, int h);

void iconScreen(void);
void configScreen(void);
