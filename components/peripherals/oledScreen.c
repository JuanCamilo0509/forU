#include "oledScreen.h"

static i2c_master_dev_handle_t display_dev_handle = NULL;
/*!< Display device handle */
extern i2c_master_bus_handle_t global_i2c_bus_handle;
extern esp_mqtt_client_handle_t client;
extern configuration_variables g_config;
extern QueueHandle_t screenQueue;
static u8g2_t u8g2;

uint8_t u8x8_byte_i2c_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int,
                         void *arg_ptr) {
  static uint8_t buffer[132];
  static uint8_t buf_idx;

  switch (msg) {
  case U8X8_MSG_BYTE_INIT:
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = I2C_DISPLAY_ADDRESS,
        .scl_speed_hz = I2C_FREQ_HZ,
        .scl_wait_us = 0,
        .flags.disable_ack_check = false,
    };

    esp_err_t ret = i2c_master_bus_add_device(global_i2c_bus_handle,
                                              &dev_config, &display_dev_handle);
    if (ret != ESP_OK) {
      ESP_LOGE(U8G2, "I2C master driver initialized failed");
      return 0;
    }

    ESP_LOGI(U8G2, "I2C master driver initialized successfully");
    break;

  case U8X8_MSG_BYTE_START_TRANSFER:
    buf_idx = 0;
    break;

  case U8X8_MSG_BYTE_SET_DC:
    break;

  case U8X8_MSG_BYTE_SEND:
    for (size_t i = 0; i < arg_int; ++i) {
      buffer[buf_idx++] = *((uint8_t *)arg_ptr + i);
    }
    break;

  case U8X8_MSG_BYTE_END_TRANSFER:
    if (buf_idx > 0 && display_dev_handle != NULL) {
      esp_err_t ret = i2c_master_transmit(display_dev_handle, buffer, buf_idx,
                                          I2C_TIMEOUT_MS);
      if (ret != ESP_OK) {
        ESP_LOGE(U8G2, "I2C master transmission failed");
        return 0;
      }

      ESP_LOGD(U8G2, "Sent %d bytes to 0x%02X: control_byte=0x%02X", buf_idx,
               I2C_DISPLAY_ADDRESS, buffer[0]);
    }
    break;

  default:
    return 0;
  }
  return 1;
}

uint8_t u8x8_gpio_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int,
                           void *arg_ptr) {
  switch (msg) {
  case U8X8_MSG_GPIO_AND_DELAY_INIT:
    ESP_LOGI(U8G2, "GPIO and delay initialization completed");
    break;

  case U8X8_MSG_DELAY_MILLI:
    /* Millisecond delay */
    vTaskDelay(pdMS_TO_TICKS(arg_int));
    break;

  case U8X8_MSG_DELAY_10MICRO:
    /* 10 microsecond delay */
    esp_rom_delay_us(arg_int * 10);
    break;

  case U8X8_MSG_DELAY_100NANO:
    /* 100 nanosecond delay - use minimal delay on ESP32 */
    __asm__ __volatile__("nop");
    break;

  case U8X8_MSG_DELAY_I2C:
    /* I2C timing delay: 5us for 100KHz, 1.25us for 400KHz */
    esp_rom_delay_us(5 / arg_int);
    break;

  case U8X8_MSG_GPIO_RESET:
    /* GPIO reset control (optional for most display controllers) */
    break;

  default:
    /* Other GPIO messages not handled */
    return 0;
  }
  return 1;
}

void u8g2_ClearSection(u8g2_t *u8g2, int x, int y, int w, int h) {
  u8g2_SetDrawColor(u8g2, 0);
  u8g2_DrawBox(u8g2, x, y, w, h);
  u8g2_SetDrawColor(u8g2, 1);
}

void configurationTask(void *pvParameters) {
  screen_event_t event;
  while (1) {
    if (xQueueReceive(screenQueue, &event, portMAX_DELAY) == pdTRUE) {
      switch (event.type) {
      case OK_WIFI:
        fillCheckBox(&u8g2, event.TAG, CENTER_LEFT);
        u8g2_SendBuffer(&u8g2);
        break;
      case OK_MQTT:
        fillCheckBox(&u8g2, event.TAG, BOTTOM_LEFT);
        u8g2_SendBuffer(&u8g2);
        break;
      case ERROR:
        u8g2_ClearBuffer(&u8g2);
        printStrDoubleLine(&u8g2, event.TAG, event.text, CENTER_CENTER);
        u8g2_SendBuffer(&u8g2);
        break;
      default:
        break;
      }
      u8g2_SendBuffer(&u8g2);
    }
  }
}

void iconScreenTask(void *pvParameters) {
  screen_event_t event;
  int current = 0;
  while (1) {
    if (xQueueReceive(screenQueue, &event, portMAX_DELAY) == pdTRUE) {
      ESP_LOGI("U8G2", "Current Index: %d", current);
      // TODO: Create a global variable for the icons quantities.
      // BUG: There is an error with this when you get to the first index.
      int next_index = (current + 1) % 13;
      int prev_index = (current - 1 < 0) ? 13 : current - 1;

      switch (event.type) {
      case SELECT:
        char indexToServer[5];
        snprintf(indexToServer, sizeof(indexToServer), "%d", current);
        esp_mqtt_client_publish(client, g_config.mqtt_user, indexToServer, 0, 0,
                                1);
        ESP_LOGI(U8G2, "Message send");
        break;
      case UP:
        u8g2_ClearSection(&u8g2, 0, 0, 24, 32);
        printIcon(&u8g2, next_index, CENTER_LEFT);
        current = next_index;
        ESP_LOGI(U8G2, "UP");
        break;
      case DOWN:
        u8g2_ClearSection(&u8g2, 0, 0, 24, 32);
        printIcon(&u8g2, prev_index, CENTER_LEFT);
        current = prev_index;
        ESP_LOGI(U8G2, "DOWN");
        break;
      case USR1:
        if (event.text[0] != '\0') {
          current = atoi(event.text);
          ESP_LOGI("Screen", "Update by MQTT to: %d", current);
          u8g2_ClearSection(&u8g2, 0, 0, 24, 32);
          printIcon(&u8g2, current, CENTER_LEFT);
        }
        break;
      case USR2:
        if (event.text[0] != '\0') {
          current = atoi(event.text);
          ESP_LOGI("Screen", "Update by MQTT to: %d", current);
          u8g2_ClearSection(&u8g2, 104, 0, 128, 32);
          printIcon(&u8g2, current, CENTER_RIGHT);
        }
        break;
      case DAY_CHANGE:
        u8g2_ClearSection(&u8g2, 25, 0, 80, 32);
        printStrDoubleLine(&u8g2, event.text, "dias", CENTER_CENTER);
        break;
      default:
        u8g2_ClearBuffer(&u8g2);
        printStrDoubleLine(&u8g2, "Wtf", "Como llegamos aquí", CENTER_CENTER);
        break;
      }
      u8g2_SendBuffer(&u8g2);
    }
  }
}

void screenInit(void) {
  u8g2_Setup_ssd1306_i2c_128x32_univision_f(&u8g2, U8G2_R0, u8x8_byte_i2c_cb,
                                            u8x8_gpio_delay_cb);
  u8g2_InitDisplay(&u8g2);
  u8g2_SetPowerSave(&u8g2, 0);
}

void iconScreen(void) {
  u8g2_ClearBuffer(&u8g2);
  printIconByName(&u8g2, loadL, CENTER_RIGHT);
  printIconByName(&u8g2, loadR, CENTER_CENTER);
  printIconByName(&u8g2, loadL, CENTER_LEFT);

  u8g2_SendBuffer(&u8g2);

  xTaskCreate(iconScreenTask, "screen Task", 4096, NULL, 5, NULL);
}

void configScreen(void) {
  u8g2_ClearBuffer(&u8g2);
  u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
  drawStr(&u8g2, "Configuration", TOP_LEFT);
  u8g2_SetFont(&u8g2, u8g2_font_ncenR08_tr);
  checkbox(&u8g2, "WIFI", CENTER_LEFT);
  checkbox(&u8g2, "SERVER", BOTTOM_LEFT);
  printIconByName(&u8g2, web, CENTER_RIGHT);
  u8g2_SendBuffer(&u8g2);

  xTaskCreate(configurationTask, "screen Task", 4096, NULL, 5, NULL);
}
