#pragma once

#include "wifi.h"
#include <sys/param.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"

#include "esp_netif.h"
#include "esp_wifi.h"
#include "lwip/inet.h"
#include "nvs_flash.h"
#include "oledScreen.h"

#include "captivePortal.h"
#include "dns_server.h"
#include "esp_http_server.h"

#define EXAMPLE_ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_MAX_STA_CONN CONFIG_ESP_MAX_STA_CONN

extern const char root_start[] asm("_binary_root_html_start");
extern const char root_end[] asm("_binary_root_html_end");

static const char *captivePortalTAG = "captiveaPortal";

#ifdef CONFIG_ESP_ENABLE_DHCP_CAPTIVEPORTAL
static void dhcp_set_captiveportal_url(void);
#endif // CONFIG_ESP_ENABLE_DHCP_CAPTIVEPORTAL

// HTTP GET Handler
esp_err_t root_get_handler(httpd_req_t *req);

// HTTP POST Handler
esp_err_t root_post_handler(httpd_req_t *req);

static const httpd_uri_t root = {
    .uri = "/", .method = HTTP_GET, .handler = root_get_handler};

static const httpd_uri_t forms = {
    .uri = "/form", .method = HTTP_POST, .handler = root_post_handler};

// HTTP Error (404) Handler - Redirects all requests to the root page
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err);

httpd_handle_t start_webserver(void);

void captivePortal();

void url_decode(char *dst, const char *src);
