#ifndef __GlobalDefinitions__H
#define __GlobalDefinitions__H

#pragma once

// CConfigurationManager Definitions
#define ESP8266_LED 2

const char WiFiAPPID[] = "ESP8266 ConfigWifi";
const char WiFiAPPSK[] = "123456789";

// CConfigurationFile Definitions
#define CONFIG_FILE_NAME "/config.txt"
#define CONFIG_FILE_SEPAREATOR '&'
#define CONFIG_FILE_EQUALS '='
#define CONFIG_FILE_END '#'

const char stationID[] = "ESP8266_OUTSIDE_LEDs_Switch";
const char outside_switch_topic[] = "outside/led_switch";
const char outside_switch_state_topic[] = "outside/led_switch_state";

#define ESP_MIN_OPERATE_VOLTAGE 2.8

// CWorkingStation Definitions
#define SWITCH_PIN 0
#define BUILD_IN_LED 2
#define BUILD_IN_LED_ON LOW
#define BUILD_IN_LED_OFF HIGH

#define CURRENT_SENSOR_LOCATION LIVING_ROOM_SENSOR

// Real Global Definitions
#define PRINT_LINES 1

#define SERIAL_SPEED 115200 // 74880

#if (PRINT_LINES == 1)
#define Print(x) Serial.print(x)
#define Println(x) Serial.println(x)
#define SERIAL_CONFIGURE Serial.begin(SERIAL_SPEED, SERIAL_8N1, SERIAL_TX_ONLY)
#define SERIAL_END Serial.flush(); \
                   Serial.end()
#else
#define Print(x) ((void)0)
#define Println(x) ((void)0)
#define SERIAL_CONFIGURE ((void)0)
#define SERIAL_END ((void)0)
#endif // DEBUG

#endif // !__GlobalDefinitions__H
