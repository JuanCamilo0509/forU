#include "global_memory.h" // I can change this to the nvs_manager instead
#include "gpio_manager.h"
#include "gyroscope.h"
#include "nvs_manager.h"
#include "provisioning.h"
#include "wifi.h"

i2c_master_bus_handle_t global_i2c_bus_handle;
QueueHandle_t screenQueue;
configuration_variables g_config;
int8_t passed;

static void gestureDetectionTask(void *arg) { gestureDetection(); }

void init_shared_i2c_bus(void) {
  i2c_master_bus_config_t bus_config = {
      .i2c_port = I2C_MASTER_NUM,
      .sda_io_num = I2C_MASTER_SDA_IO,
      .scl_io_num = I2C_MASTER_SCL_IO,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .flags.enable_internal_pullup = true,
  };
  ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &global_i2c_bus_handle));
}

void app_main(void) {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  // External interruption for the variable hard reset.
  // Interruption for the wake up pin.
  initGpios();

  init_shared_i2c_bus();
  screenInit();
  screenQueue = xQueueCreate(10, sizeof(screen_event_t));

  passed = read_passed_from_nvs();
  int configured = load_internal_variables();

  if (!configured) {
    save_passed_to_nvs(0);
    configScreen();
    startCaptivePortal();
    return;
  }
  if (passed == 0) {
    configScreen();
    testProtocol();
  } else {
    wifi_init_sta();
    mqtt_app_start(0);
    init_sntp();
    iconScreen();
    if (bmi160Init()) {
      xTaskCreate(gestureDetectionTask, "gesture_task", 4096, NULL, 5, NULL);
    } else {
      ESP_LOGE("MAIN", "Something went wrong");
    }
  }
}
