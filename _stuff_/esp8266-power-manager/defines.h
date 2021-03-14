#ifndef DEFINES_H
#define DEFINES_H

// ----------------------------------------------------------------------------

#define DEVICE_STATUS_LED_PIN   13  // D7
#define RPI_STATUS_LED_PIN      16  // D0

#define POWER_BUTTON_PIN        14  // D5  
#define WPS_BUTTON_PIN          12  // D6

// #define MOSFET_CONTROL_PIN       2  // D4
#define MOSFET_CONTROL_PIN      15  // D8

// ----------------------------------------------------------------------------

#define SERIAL_BAUDRATE         115200

// ----------------------------------------------------------------------------

// #define DEVICE_STATUS_LED_MODE_NOT_CONNECTED 0
// #define DEVICE_STATUS_LED_MODE_TRYING_WPS    1
// #define DEVICE_STATUS_LED_MODE_CONNECTING    2
// #define DEVICE_STATUS_LED_MODE_CONNECTED     3

// ----------------------------------------------------------------------------

#define NTP_SERVER          "europe.pool.ntp.org"
#define NTP_TIMEOUT         1500
#define NTP_PRINT_TIME_INTERVAL   1000
// #define NTP_SERVER_POOL         "europe.pool.ntp.org"
// #define NTP_TIME_OFFSET         3600
// #define NTP_UPDATE_INTERVAL     60000

// ----------------------------------------------------------------------------

#define WEBSERVICE_PORT             80
#define WEBSERVICE_TIMEOUT_MSEC     2000

// ----------------------------------------------------------------------------

#endif