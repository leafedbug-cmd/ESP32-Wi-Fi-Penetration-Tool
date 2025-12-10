idf_component_register(SRCS "attack.c" "attack_dos.c" "attack_handshake.c" "attack_method.c" "attack_pmkid.c"
                    INCLUDE_DIRS "interface"
                    REQUIRES esp_wifi esp_timer esp_event
                    PRIV_REQUIRES frame_analyzer wifi_controller wsl_bypasser hccapx_serializer pcap_serializer led_strip_ws2812)
