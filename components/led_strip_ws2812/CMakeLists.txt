idf_component_register(SRCS "led_strip_ws2812.c"
                       INCLUDE_DIRS "include"
                       REQUIRES driver esp_common esp_rom led_strip)
