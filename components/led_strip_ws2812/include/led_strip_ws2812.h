/**
 * @file led_strip_ws2812.h
 * @brief WS2812 addressable LED control for ESP32-S3-DevKitC-1
 * 
 * Controls the built-in WS2812 LED on GPIO 48 of ESP32-S3-DevKitC-1
 */

#ifndef LED_STRIP_WS2812_H
#define LED_STRIP_WS2812_H

#include <stdint.h>

/**
 * @brief Initialize WS2812 LED strip on GPIO 48
 * 
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
int led_strip_init(void);

/**
 * @brief Set LED color and refresh
 * 
 * @param red Red component (0-255)
 * @param green Green component (0-255)
 * @param blue Blue component (0-255)
 * @return ESP_OK on success
 */
int led_strip_set_color(uint8_t red, uint8_t green, uint8_t blue);

/**
 * @brief Pulse LED with specified color
 * 
 * @param red Red component (0-255)
 * @param green Green component (0-255)
 * @param blue Blue component (0-255)
 * @param pulse_period_ms Period in milliseconds for pulse cycle
 */
void led_strip_pulse(uint8_t red, uint8_t green, uint8_t blue, uint32_t pulse_period_ms);

/**
 * @brief Stop pulsing and turn off LED
 */
void led_strip_off(void);

#endif // LED_STRIP_WS2812_H
