/**
 * @file main.c
 * @author 0x1381 (nullx1381@gmail.com)
 * @date 2021-04-02
 * @copyright Copyright (c) 2021
 * 
 * @brief Main file used to setup ESP32 into initial state
 * 
 * Starts management AP and webserver  
 */

#include <stdio.h>

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "attack.h"
#include "wifi_controller.h"
#include "webserver.h"
#include "led_strip_ws2812.h"

static const char* TAG = "main";

void app_main(void)
{
    ESP_LOGD(TAG, "app_main started");
    
    // Initialize NVS (required for WiFi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize LED on GPIO 48 (WS2812 built-in LED)
    ESP_LOGI(TAG, "Initializing WS2812 LED");
    if (led_strip_init() == 0) {
        ESP_LOGI(TAG, "LED initialized successfully");
        // LED pulse: blue color while running
        led_strip_pulse(0, 0, 255, 1000);  // Blue pulse at 1Hz
    } else {
        ESP_LOGW(TAG, "LED initialization failed");
    }
    
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifictl_mgmt_ap_start();
    attack_init();
    webserver_run();
}
