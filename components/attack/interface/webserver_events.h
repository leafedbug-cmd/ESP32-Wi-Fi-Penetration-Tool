/**
 * @file webserver_events.h
 * @brief Shared event declarations for webserver communication
 * 
 * This header breaks the circular dependency between attack and webserver components
 */
#ifndef WEBSERVER_EVENTS_H
#define WEBSERVER_EVENTS_H

#include "esp_event.h"

ESP_EVENT_DECLARE_BASE(WEBSERVER_EVENTS);
enum {
    WEBSERVER_EVENT_ATTACK_REQUEST,
    WEBSERVER_EVENT_ATTACK_RESET
};

/**
 * @brief Struct to deserialize attack request parameters 
 */
typedef struct {
    uint8_t ap_record_id;   //< ID of chosen AP. It can be used to access ap_records array from wifi_controller - ap_scanner
    uint8_t type;           //< Chosen type of attack
    uint8_t method;         //< Chosen method of attack
    uint8_t timeout;        //< Attack timeout in seconds
} attack_request_t;

#endif
