/**
 * @file led_strip_ws2812.c
 * @brief WS2812 addressable LED control implementation
 */

#include <stdio.h>
#include <string.h>
#include "driver/gpio.h"
#include "driver/rmt.h"
#include "esp_log.h"
#include "led_strip_ws2812.h"

static const char* TAG = "led_ws2812";

// GPIO pin for WS2812 LED on ESP32-S3-DevKitC-1
#define LED_STRIP_GPIO 48

// RMT channel for LED control
#define LED_STRIP_RMT_CH RMT_CHANNEL_0

// LED strip configuration
#define LED_STRIP_NUM_LEDS 1

// WS2812 timing constants (in RMT clock ticks)
#define T0H_TICKS 14  // ~0.35us
#define T0L_TICKS 41  // ~1.05us  
#define T1H_TICKS 39  // ~1.00us
#define T1L_TICKS 16  // ~0.40us

static uint8_t led_red = 0;
static uint8_t led_green = 0;
static uint8_t led_blue = 0;
static bool pulse_active = false;
static uint32_t pulse_period = 1000;

/**
 * @brief Convert RGB to RMT format for WS2812
 */
static void rgb_to_rmt(uint8_t r, uint8_t g, uint8_t b, uint32_t* rmt_data, int pos) {
    // WS2812 uses GRB format
    uint8_t colors[] = {g, r, b};
    
    for (int i = 0; i < 3; i++) {
        uint8_t byte = colors[i];
        for (int j = 7; j >= 0; j--) {
            if ((byte & (1 << j)) == 0) {
                // Send 0 bit
                rmt_data[pos * 24 + (7 - j)] = (T0H_TICKS << 16) | T0L_TICKS;
            } else {
                // Send 1 bit
                rmt_data[pos * 24 + (7 - j)] = (T1H_TICKS << 16) | T1L_TICKS;
            }
        }
    }
}

int led_strip_init(void) {
    ESP_LOGI(TAG, "Initializing WS2812 LED on GPIO %d", LED_STRIP_GPIO);
    
    // Configure GPIO
    gpio_pad_select_gpio(LED_STRIP_GPIO);
    gpio_set_direction(LED_STRIP_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_STRIP_GPIO, 0);
    
    // Configure RMT
    rmt_config_t config = RMT_DEFAULT_CONFIG_OUTPUT(LED_STRIP_GPIO, LED_STRIP_RMT_CH);
    config.clk_div = 80; // 1 MHz clock (80 MHz / 80)
    config.tx_config.carrier_en = false;
    
    if (rmt_config(&config) != ESP_OK) {
        ESP_LOGE(TAG, "RMT config failed");
        return 1;
    }
    
    if (rmt_driver_install(config.channel, 0, 0) != ESP_OK) {
        ESP_LOGE(TAG, "RMT driver install failed");
        return 1;
    }
    
    ESP_LOGI(TAG, "WS2812 LED initialized successfully");
    return 0;
}

int led_strip_set_color(uint8_t red, uint8_t green, uint8_t blue) {
    pulse_active = false;
    led_red = red;
    led_green = green;
    led_blue = blue;
    
    // Prepare RMT data
    uint32_t rmt_data[LED_STRIP_NUM_LEDS * 24 + 1];
    
    // Convert RGB to RMT format
    rgb_to_rmt(red, green, blue, rmt_data, 0);
    
    // Add reset code (low for >50us)
    rmt_data[LED_STRIP_NUM_LEDS * 24] = 0;
    
    // Send to LED
    if (rmt_write_items(LED_STRIP_RMT_CH, (rmt_item32_t*)rmt_data, 
                        LED_STRIP_NUM_LEDS * 24 + 1, true) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write LED data");
        return 1;
    }
    
    ESP_LOGD(TAG, "Set LED color R:%d G:%d B:%d", red, green, blue);
    return 0;
}

void led_strip_pulse(uint8_t red, uint8_t green, uint8_t blue, uint32_t pulse_period_ms) {
    led_red = red;
    led_green = green;
    led_blue = blue;
    pulse_period = pulse_period_ms;
    pulse_active = true;
    
    // Start pulsing (would need a timer task in real implementation)
    // For now, just set the color
    led_strip_set_color(red, green, blue);
    ESP_LOGI(TAG, "LED pulse started: R:%d G:%d B:%d Period:%d ms", 
             red, green, blue, pulse_period_ms);
}

void led_strip_off(void) {
    pulse_active = false;
    led_strip_set_color(0, 0, 0);
    ESP_LOGI(TAG, "LED turned off");
}
