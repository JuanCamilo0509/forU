/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* i2c - Simple Example

   Simple I2C example that shows how to initialize I2C
   as well as reading and writing from and to registers for a sensor connected
   over I2C.

   The sensor used in this example is a MPU9250 inertial measurement unit.
*/

#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include <stdio.h>

#define BMI160_GYRO_DATA_ADDR 0x0C
#define BMI160_SENSOR_ADDR 0x68
#define BMI160_CHIPID_REG_ADDR 0x00
#define BMI160_CMD_REG_ADDR 0x7E
#define BMI160_CMD_GYRO_NORMAL 0x15

#define I2C_MASTER_SCL_IO CONFIG_I2C_MASTER_SCL
#define I2C_MASTER_SDA_IO CONFIG_I2C_MASTER_SDA
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ CONFIG_I2C_MASTER_FREQUENCY
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0
#define I2C_MASTER_TIMEOUT_MS 1000

#define MPU9250_SENSOR_ADDR 0x68 /*!< Address of the MPU9250 sensor */
#define MPU9250_WHO_AM_I_REG_ADDR                                              \
  0x75 /*!< Register addresses of the "who am I" register */
#define MPU9250_PWR_MGMT_1_REG_ADDR                                            \
  0x6B /*!< Register addresses of the power management register */
#define MPU9250_RESET_BIT 7

static i2c_master_dev_handle_t dev_handle;
static const char *TAG = "BMI160";
extern QueueHandle_t screenQueue;

/**
 * @brief Read a sequence of bytes from a MPU9250 sensor registers
 */
esp_err_t bmi160_register_read(i2c_master_dev_handle_t dev_handle,
                               uint8_t reg_addr, uint8_t *data, size_t len);

/**
 * @brief Write a byte to a MPU9250 sensor register
 */
esp_err_t bmi160_register_write_byte(i2c_master_dev_handle_t dev_handle,
                                     uint8_t reg_addr, uint8_t data);

/**
 * @brief i2c master initialization
 */
void i2c_master_init(i2c_master_bus_handle_t *bus_handle,
                     i2c_master_dev_handle_t *dev_handle);

void readFromBMI160(int16_t *gyro_x, int16_t *gyro_y, int16_t *gyro_z,
                    int16_t *acc_x, int16_t *acc_y, int16_t *acc_z);

int bmi160Init(void);

void gestureDetection(void);
