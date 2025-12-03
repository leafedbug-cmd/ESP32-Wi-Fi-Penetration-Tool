# WS2812 Status LED Component

Simple wrapper around the ESP-IDF RMT driver that drives the single WS2812 LED
available on the ESP32-S3-DevKitC-1 as well as the Flipper Zero Wi-Fi Dev Board.

## Features
- Initializes the WS2812 on GPIO 48 using the RMT peripheral
- Sets a static RGB color with `led_strip_set_color`
- Convenience helpers for starting a single-color "pulse" indicator and
  switching the LED off

## Usage
Include `led_strip_ws2812.h` and call `led_strip_init()` once during boot. After
that you can set a static color or start a pulse. The `app_main` entry point
shows a minimal example that paints the LED blue once the firmware is running.
If you need more elaborate blinking patterns, extend the component with a timer
task and re-use the provided RGB-to-RMT conversion helper.

## Configuration
- `LED_STRIP_GPIO` (default `48`) controls which pin drives the LED.
- `LED_STRIP_NUM_LEDS` (default `1`) can be increased if you chain more pixels.

Tune timings via `T0H/T0L/T1H/T1L` constants if you are driving a strip with a
different clock or voltage requirement.
