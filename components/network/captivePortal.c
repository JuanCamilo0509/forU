#include "captivePortal.h"

extern QueueHandle_t screenQueue;
extern int testing;

#ifdef CONFIG_ESP_ENABLE_DHCP_CAPTIVEPORTAL
static void dhcp_set_captiveportal_url(void) {
  // get the IP of the access point to redirect to
  esp_netif_ip_info_t ip_info;
  esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"),
                        &ip_info);

  char ip_addr[16];
  inet_ntoa_r(ip_info.ip.addr, ip_addr, 16);
  ESP_LOGI(TAG, "Set up softAP with IP: %s", ip_addr);

  // turn the IP into a URI
  char *captiveportal_uri = (char *)malloc(32 * sizeof(char));
  assert(captiveportal_uri && "Failed to allocate captiveportal_uri");
  strcpy(captiveportal_uri, "http://");
  strcat(captiveportal_uri, ip_addr);

  // get a handle to configure DHCP with
  esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");

  // set the DHCP option 114
  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_stop(netif));
  ESP_ERROR_CHECK(esp_netif_dhcps_option(
      netif, ESP_NETIF_OP_SET, ESP_NETIF_CAPTIVEPORTAL_URI, captiveportal_uri,
      strlen(captiveportal_uri)));
  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_start(netif));
}
#endif // CONFIG_ESP_ENABLE_DHCP_CAPTIVEPORTAL

void url_decode(char *dst, const char *src) {
  char a, b;
  while (*src) {
    if ((*src == '%') && ((a = src[1]) && (b = src[2])) &&
        (isxdigit((unsigned char)a) && isxdigit((unsigned char)b))) {
      if (a >= 'a')
        a -= 'a' - 'A';
      if (a >= 'A')
        a -= 'A' - 10;
      else
        a -= '0';
      if (b >= 'a')
        b -= 'a' - 'A';
      if (b >= 'A')
        b -= 'A' - 10;
      else
        b -= '0';
      *dst++ = 16 * a + b;
      src += 3;
    } else if (*src == '+') {
      *dst++ = ' ';
      src++;
    } else {
      *dst++ = *src++;
    }
  }
  *dst = '\0';
}

esp_err_t root_get_handler(httpd_req_t *req) {
  const uint32_t root_len = root_end - root_start;

  ESP_LOGI(captivePortalTAG, "Serve root");
  httpd_resp_set_type(req, "text/html");
  httpd_resp_send(req, root_start, root_len);

  return ESP_OK;
}

esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err) {
  httpd_resp_set_status(req, "303 See Other");
  httpd_resp_set_hdr(req, "Location", "/");
  httpd_resp_send(req, "Redirect to the captive portal", HTTPD_RESP_USE_STRLEN);

  ESP_LOGI(captivePortalTAG, "Redirecting to root");
  return ESP_OK;
}

httpd_handle_t start_webserver(void) {
  httpd_handle_t server = NULL;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_open_sockets = 7;
  config.lru_purge_enable = true;

  // Start the httpd server
  ESP_LOGI(captivePortalTAG, "Starting server on port: '%d'",
           config.server_port);
  if (httpd_start(&server, &config) == ESP_OK) {
    // Set URI handlers
    ESP_LOGI(captivePortalTAG, "Registering URI handlers");

    httpd_register_uri_handler(server, &root);
    httpd_register_uri_handler(server, &forms);

    httpd_register_err_handler(server, HTTPD_404_NOT_FOUND,
                               http_404_error_handler);
  }
  return server;
}

esp_err_t root_post_handler(httpd_req_t *req) {
  char buf[1024];

  int ret, remaining = req->content_len;

  ret = httpd_req_recv(req, buf, remaining);
  if (ret <= 0) {
    if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
      httpd_resp_send_408(req);
    }
    return ESP_FAIL;
  }
  buf[ret] = '\0';

  ESP_LOGI(captivePortalTAG, "Datos crudos recibidos: %s", buf);

  char ssid[32] = {0};
  char password[64] = {0};
  char mqtt_url[64] = {0};
  char mqtt_user[32] = {0};
  char mqtt_password[64] = {0};
  char topic[32] = {0};
  char anniversary[16] = {0};

  httpd_query_key_value(buf, "ssid", ssid, sizeof(ssid));
  httpd_query_key_value(buf, "password", password, sizeof(password));
  httpd_query_key_value(buf, "mqtt_url", mqtt_url, sizeof(mqtt_url));
  httpd_query_key_value(buf, "mqtt_user", mqtt_user, sizeof(mqtt_user));
  httpd_query_key_value(buf, "mqtt_password", mqtt_password,
                        sizeof(mqtt_password));
  httpd_query_key_value(buf, "topic", topic, sizeof(topic));
  httpd_query_key_value(buf, "anniversary", anniversary, sizeof(anniversary));

  char decoded_ssid[32] = {0};
  char decoded_password[64] = {0};
  char decoded_mqtt_url[64] = {0};
  char decoded_mqtt_user[32] = {0};
  char decoded_mqtt_password[64] = {0};
  char decoded_topic[32] = {0};
  char decoded_anniversary[16] = {0};

  url_decode(decoded_ssid, ssid);
  url_decode(decoded_password, password);
  url_decode(decoded_mqtt_url, mqtt_url);
  url_decode(decoded_mqtt_user, mqtt_user);
  url_decode(decoded_mqtt_password, mqtt_password);
  url_decode(decoded_topic, topic);
  url_decode(decoded_anniversary, anniversary);

  if (strlen(decoded_ssid)) {
    ESP_LOGI(captivePortalTAG, "SSID: %s", decoded_ssid);
    ESP_LOGI(captivePortalTAG, "Password: %s", decoded_password);
    ESP_LOGI(captivePortalTAG, "MQTT URL: %s", decoded_mqtt_url);
    ESP_LOGI(captivePortalTAG, "mqtt_user: %s", decoded_mqtt_user);
    ESP_LOGI(captivePortalTAG, "mqtt_password: %s", decoded_mqtt_password);
    ESP_LOGI(captivePortalTAG, "Topic: %s", decoded_topic);
    ESP_LOGI(captivePortalTAG, "Anniversary: %s", decoded_anniversary);

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err == ESP_OK) {

      nvs_set_str(nvs_handle, "wifi_ssid", decoded_ssid);
      nvs_set_str(nvs_handle, "wifi_pass", decoded_password);
      nvs_set_str(nvs_handle, "mqtt_url", decoded_mqtt_url);
      nvs_set_str(nvs_handle, "mqtt_user", decoded_mqtt_user);
      nvs_set_str(nvs_handle, "mqtt_pass", decoded_mqtt_password);
      nvs_set_str(nvs_handle, "topic", decoded_topic);
      nvs_set_str(nvs_handle, "anniversary", decoded_anniversary);

      nvs_commit(nvs_handle);
      nvs_close(nvs_handle);

      ESP_LOGI(captivePortalTAG, "Data saved on NVS", esp_err_to_name(err));

    } else {
      ESP_LOGE(captivePortalTAG, "Error opening the nvs: %s",
               esp_err_to_name(err));
    }

    const char *resp_str = "<html><body><h1>Configuracion "
                           "Guardada!</h1><p>El ESP32 se reiniciara "
                           "para conectarse.</p></body></html>";

    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

    vTaskDelay(pdMS_TO_TICKS(2000));
    esp_restart();
  } else {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Formulario invalido");
  }

  return ESP_OK;
}

void captivePortal(void) {
  esp_log_level_set("httpd_uri", ESP_LOG_ERROR);
  esp_log_level_set("httpd_txrx", ESP_LOG_ERROR);
  esp_log_level_set("httpd_parse", ESP_LOG_ERROR);

#ifdef CONFIG_ESP_ENABLE_DHCP_CAPTIVEPORTAL
  dhcp_set_captiveportal_url();
#endif

  start_webserver();

  dns_server_config_t config = DNS_SERVER_CONFIG_SINGLE(
      "*" /* all A queries */, "WIFI_AP_DEF" /* softAP netif ID */);
  start_dns_server(&config);
}
