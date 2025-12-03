/**
 * @file led_strip_ws2812.c
 * @brief WS2812 addressable LED control implementation for ESP-IDF 5.x
 * Compatible with Lonely Binary ESP32-S3 board (RGB LED on GPIO 48)
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "driver/rmt_tx.h"
#include "esp_log.h"
#include "led_strip_ws2812.h"

static const char* TAG = "led_ws2812";

// GPIO pin for WS2812 LED on Lonely Binary ESP32-S3 board
#define LED_STRIP_GPIO 48

// LED strip configuration
#define LED_STRIP_NUM_LEDS 1

// WS2812 timing (in nanoseconds)
#define WS2812_T0H_NS 350
#define WS2812_T0L_NS 1000
#define WS2812_T1H_NS 1000
#define WS2812_T1L_NS 350
#define WS2812_RESET_US 280

static rmt_channel_handle_t led_channel = NULL;
static rmt_encoder_handle_t led_encoder = NULL;
static uint8_t led_red = 0;
static uint8_t led_green = 0;
static uint8_t led_blue = 0;
static bool pulse_active = false;
static uint32_t pulse_period = 1000;
static bool initialized = false;
static TaskHandle_t pulse_task_handle = NULL;

// Simple bytes encoder for WS2812
typedef struct {
    rmt_encoder_t base;
    rmt_encoder_t *bytes_encoder;
    rmt_encoder_t *copy_encoder;
    int state;
    rmt_symbol_word_t reset_code;
} ws2812_encoder_t;

static size_t ws2812_encode(rmt_encoder_t *encoder, rmt_channel_handle_t channel,
                            const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state)
{
    ws2812_encoder_t *ws2812_encoder = __containerof(encoder, ws2812_encoder_t, base);
    rmt_encode_state_t session_state = RMT_ENCODING_RESET;
    size_t encoded_symbols = 0;
    
    switch (ws2812_encoder->state) {
        case 0: // Send RGB data
            encoded_symbols += ws2812_encoder->bytes_encoder->encode(
                ws2812_encoder->bytes_encoder, channel, primary_data, data_size, &session_state);
            if (session_state & RMT_ENCODING_COMPLETE) {
                ws2812_encoder->state = 1;
            }
            if (session_state & RMT_ENCODING_MEM_FULL) {
                *ret_state = (rmt_encode_state_t)(session_state & (~RMT_ENCODING_COMPLETE));
                return encoded_symbols;
            }
            // Fall through
        case 1: // Send reset code
            encoded_symbols += ws2812_encoder->copy_encoder->encode(
                ws2812_encoder->copy_encoder, channel, &ws2812_encoder->reset_code,
                sizeof(ws2812_encoder->reset_code), &session_state);
            if (session_state & RMT_ENCODING_COMPLETE) {
                ws2812_encoder->state = 0;
                *ret_state = RMT_ENCODING_COMPLETE;
            }
            break;
    }
    return encoded_symbols;
}

static esp_err_t ws2812_encoder_reset(rmt_encoder_t *encoder)
{
    ws2812_encoder_t *ws2812_encoder = __containerof(encoder, ws2812_encoder_t, base);
    ws2812_encoder->bytes_encoder->reset(ws2812_encoder->bytes_encoder);
    ws2812_encoder->copy_encoder->reset(ws2812_encoder->copy_encoder);
    ws2812_encoder->state = 0;
    return ESP_OK;
}

static esp_err_t ws2812_encoder_del(rmt_encoder_t *encoder)
{
    ws2812_encoder_t *ws2812_encoder = __containerof(encoder, ws2812_encoder_t, base);
    rmt_del_encoder(ws2812_encoder->bytes_encoder);
    rmt_del_encoder(ws2812_encoder->copy_encoder);
    free(ws2812_encoder);
    return ESP_OK;
}

int led_strip_init(void) {
    ESP_LOGI(TAG, "Initializing WS2812 LED on GPIO %d", LED_STRIP_GPIO);
    
    if (initialized) {
        ESP_LOGW(TAG, "LED already initialized");
        return 0;
    }
    
    // Configure RMT TX channel
    rmt_tx_channel_config_t tx_config = {
        .gpio_num = LED_STRIP_GPIO,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10000000, // 10MHz = 100ns per tick
        .mem_block_symbols = 64,
        .trans_queue_depth = 4,
    };
    
    if (rmt_new_tx_channel(&tx_config, &led_channel) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create RMT TX channel");
        return 1;
    }
    
    // Create WS2812 encoder
    ws2812_encoder_t *ws2812_enc = calloc(1, sizeof(ws2812_encoder_t));
    if (!ws2812_enc) {
        ESP_LOGE(TAG, "Failed to allocate encoder");
        return 1;
    }
    
    ws2812_enc->base.encode = ws2812_encode;
    ws2812_enc->base.reset = ws2812_encoder_reset;
    ws2812_enc->base.del = ws2812_encoder_del;
    
    // Bytes encoder for RGB data
    rmt_bytes_encoder_config_t bytes_config = {
        .bit0 = {
            .level0 = 1,
            .duration0 = WS2812_T0H_NS / 100, // Convert ns to ticks (100ns per tick)
            .level1 = 0,
            .duration1 = WS2812_T0L_NS / 100,
        },
        .bit1 = {
            .level0 = 1,
            .duration0 = WS2812_T1H_NS / 100,
            .level1 = 0,
            .duration1 = WS2812_T1L_NS / 100,
        },
        .flags.msb_first = 1,
    };
    
    if (rmt_new_bytes_encoder(&bytes_config, &ws2812_enc->bytes_encoder) != ESP_OK) {
        free(ws2812_enc);
        ESP_LOGE(TAG, "Failed to create bytes encoder");
        return 1;
    }
    
    // Copy encoder for reset code
    rmt_copy_encoder_config_t copy_config = {};
    if (rmt_new_copy_encoder(&copy_config, &ws2812_enc->copy_encoder) != ESP_OK) {
        rmt_del_encoder(ws2812_enc->bytes_encoder);
        free(ws2812_enc);
        ESP_LOGE(TAG, "Failed to create copy encoder");
        return 1;
    }
    
    // Reset code (low for >280us)
    ws2812_enc->reset_code = (rmt_symbol_word_t) {
        .level0 = 0,
        .duration0 = WS2812_RESET_US * 10, // 280us in 100ns ticks
        .level1 = 0,
        .duration1 = WS2812_RESET_US * 10,
    };
    
    led_encoder = &ws2812_enc->base;
    
    // Enable RMT channel
    if (rmt_enable(led_channel) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable RMT channel");
        return 1;
    }
    
    initialized = true;
    ESP_LOGI(TAG, "WS2812 LED initialized successfully");
    return 0;
}

int led_strip_set_color(uint8_t red, uint8_t green, uint8_t blue) {
    if (!initialized) {
        ESP_LOGW(TAG, "LED not initialized");
        return 1;
    }
    
    pulse_active = false;
    led_red = red;
    led_green = green;
    led_blue = blue;
    
    // WS2812 uses GRB format
    uint8_t grb[3] = {green, red, blue};
    
    rmt_transmit_config_t tx_config = {
        .loop_count = 0,
    };
    
    if (rmt_transmit(led_channel, led_encoder, grb, sizeof(grb), &tx_config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to transmit LED data");
        return 1;
    }
    
    rmt_tx_wait_all_done(led_channel, portMAX_DELAY);
    
    ESP_LOGD(TAG, "Set LED color R:%d G:%d B:%d", red, green, blue);
    return 0;
}

// Internal function to set LED without affecting pulse state
static int led_strip_set_color_internal(uint8_t red, uint8_t green, uint8_t blue) {
    if (!initialized) {
        return 1;
    }
    
    // WS2812 uses GRB format
    uint8_t grb[3] = {green, red, blue};
    
    rmt_transmit_config_t tx_config = {
        .loop_count = 0,
    };
    
    if (rmt_transmit(led_channel, led_encoder, grb, sizeof(grb), &tx_config) != ESP_OK) {
        return 1;
    }
    
    rmt_tx_wait_all_done(led_channel, portMAX_DELAY);
    return 0;
}

// Pulse task that fades LED in and out
static void pulse_task(void *pvParameters) {
    int brightness = 0;
    int direction = 5; // Increment step
    
    while (1) {
        if (!pulse_active) {
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }
        
        // Calculate faded color
        uint8_t r = (led_red * brightness) / 100;
        uint8_t g = (led_green * brightness) / 100;
        uint8_t b = (led_blue * brightness) / 100;
        
        led_strip_set_color_internal(r, g, b);
        
        // Update brightness
        brightness += direction;
        if (brightness >= 100) {
            brightness = 100;
            direction = -5;
        } else if (brightness <= 0) {
            brightness = 0;
            direction = 5;
        }
        
        // Delay based on pulse period (full cycle = period, so step delay = period / 40)
        vTaskDelay(pdMS_TO_TICKS(pulse_period / 40));
    }
}

void led_strip_pulse(uint8_t red, uint8_t green, uint8_t blue, uint32_t pulse_period_ms) {
    led_red = red;
    led_green = green;
    led_blue = blue;
    pulse_period = pulse_period_ms;
    pulse_active = true;
    
    // Create pulse task if not already running
    if (pulse_task_handle == NULL) {
        xTaskCreate(pulse_task, "led_pulse", 2048, NULL, 5, &pulse_task_handle);
    }
    
    ESP_LOGI(TAG, "LED pulse started: R:%d G:%d B:%d Period:%ldms", 
             red, green, blue, (long)pulse_period_ms);
}

void led_strip_off(void) {
    pulse_active = false;
    led_strip_set_color(0, 0, 0);
    ESP_LOGI(TAG, "LED turned off");
}
