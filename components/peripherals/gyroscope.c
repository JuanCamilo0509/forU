// TODO: Change the inertial measurement unit reference.

#include "gyroscope.h"
#include "oledScreen.h"

extern i2c_master_bus_handle_t global_i2c_bus_handle;

esp_err_t bmi160_register_read(i2c_master_dev_handle_t dev_handle,
                               uint8_t reg_addr, uint8_t *data, size_t len) {
  return i2c_master_transmit_receive(dev_handle, &reg_addr, 1, data, len,
                                     I2C_MASTER_TIMEOUT_MS);
}

esp_err_t bmi160_register_write_byte(i2c_master_dev_handle_t dev_handle,
                                     uint8_t reg_addr, uint8_t data) {
  uint8_t write_buf[2] = {reg_addr, data};
  return i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf),
                             I2C_MASTER_TIMEOUT_MS);
}

void i2c_master_init(i2c_master_bus_handle_t *bus_handle,
                     i2c_master_dev_handle_t *dev_handle) {
  i2c_device_config_t dev_config = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = MPU9250_SENSOR_ADDR,
      .scl_speed_hz = I2C_MASTER_FREQ_HZ,
  };
  ESP_ERROR_CHECK(
      i2c_master_bus_add_device(*bus_handle, &dev_config, dev_handle));
}

void gestureDetection() {
  screen_event_t screenEvent;
  int16_t x = 0, y = 0, z = 0;
  while (1) {
    readFromBMI160(&x, &y, &z);
    int eventDetected = 0;
    // ESP_LOGI("MAIN", "X: %d | Y: %d | Z: %d", x, y, z);
    if (z > 5000) {
      screenEvent.type = UP;
      eventDetected = 1;
      ESP_LOGI(TAG, "UP");
    } else if (z < -5000) {
      screenEvent.type = DOWN;
      eventDetected = 1;
      ESP_LOGI(TAG, "DOWN");
    } else if (y > 1000) {
      screenEvent.type = SELECT;
      eventDetected = 1;
      ESP_LOGI(TAG, "SELECTED");
    }

    if (eventDetected) {
      if (xQueueSend(screenQueue, &screenEvent, pdMS_TO_TICKS(10)) != pdPASS) {
        ESP_LOGW(TAG, "Screen queue full, event dropped!");
      }
      vTaskDelay(pdMS_TO_TICKS(300));
    }
    vTaskDelay(pdMS_TO_TICKS(30));
  }
}

int bmi160Init() {
  uint8_t data[1];

  i2c_master_init(&global_i2c_bus_handle, &dev_handle);
  ESP_LOGI(TAG, "I2C initialized successfully");

  ESP_ERROR_CHECK(
      bmi160_register_read(dev_handle, BMI160_CHIPID_REG_ADDR, data, 1));
  ESP_LOGI(TAG, "BMI160 CHIP_ID = %X", data[0]);

  ESP_ERROR_CHECK(
      bmi160_register_write_byte(dev_handle, BMI160_CMD_REG_ADDR, 0x11));

  ESP_ERROR_CHECK(bmi160_register_write_byte(dev_handle, 0x5F, 20));

  ESP_ERROR_CHECK(bmi160_register_write_byte(dev_handle, 0x5F, 20));

  ESP_ERROR_CHECK(bmi160_register_write_byte(dev_handle, 0x5E, 0x1C));

  ESP_ERROR_CHECK(bmi160_register_write_byte(dev_handle, 0x50, 0x04));

  ESP_ERROR_CHECK(bmi160_register_write_byte(dev_handle, 0x53, 0x0A));

  ESP_LOGI(TAG, "I2C de-initialized successfully");
  vTaskDelay(pdMS_TO_TICKS(100));
  return 1;
}

void readFromBMI160(int16_t *gyro_x, int16_t *gyro_y, int16_t *gyro_z) {
  uint8_t raw_buffer[6];
  esp_err_t err =
      bmi160_register_read(dev_handle, BMI160_GYRO_DATA_ADDR, raw_buffer, 6);
  if (err == ESP_OK) {
    *gyro_x = (int16_t)((raw_buffer[1] << 8) | raw_buffer[0]);
    *gyro_y = (int16_t)((raw_buffer[3] << 8) | raw_buffer[2]);
    *gyro_z = (int16_t)((raw_buffer[5] << 8) | raw_buffer[4]);
  } else {
    ESP_LOGE(TAG, "Error leyendo los datos de los ejes");
  }
}
