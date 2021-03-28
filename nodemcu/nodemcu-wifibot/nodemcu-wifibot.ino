// nodemcu-wifibot
//
// WiFi-enabled robot (based on NodeMCU - ESP8266) controlled by Android App
// Written by Salvatore Carotenuto (mailto: ultimoistante@gmail.com)
//
// based on original work by Mert Arduino and Tech (YouTube link: https://goo.gl/RLT6DT - github: https://github.com/MertArduino)
//
// hardware connections:
//
//                              NodeMCU
//                      +--------------------+
//  battery monitor --- | A0         D0 [16] | ---
//                      |            D1 [05] | --- Left motor backward
//                      |            D2 [04] | --- Left motor forward  
//                      |            D3 [00] |
//                      |            D4 [02] | --- Left motor enable
//                      |                3v3 |
//                      |                GND |
//                      |            D5 [14] | --- Right motor backward 
//                      |            D6 [12] | --- Right motor forward 
//                      |            D7 [13] | --- Right motor enable
//                      |            D8 [15] | --- Button
//                      |         D9/RX [03] |
//                      |        D10/TX [01] |
//                      |                GND |
//                      |                3v3 |
//                      +--------------------+
//

// includes
#include <ESP8266WiFi.h>
#include "LedBlinker.h"

// ----------------------------------------------------------------------------

#define _VERSION_   "1.0.0"

// ----------------------------------------------------------------------------

// ---- pin definitions
//
// eyes leds pin
#define EYES_LEDS             16 // D0
// motor control pins
#define LEFT_MOTOR_BACKWARD    5 // D1
#define LEFT_MOTOR_FORWARD     4 // D2
#define RIGHT_MOTOR_BACKWARD  14 // D5
#define RIGHT_MOTOR_FORWARD   12 // D6
#define LEFT_MOTOR_ENABLE      2 // D4
#define RIGHT_MOTOR_ENABLE    13 // D7
// button pin
#define BUTTON_PIN            15 // D8

// ----------------------------------------------------------------------------

// defines used by motor functions
#define LEFT_MOTOR              1
#define RIGHT_MOTOR             2
#define DIRECTION_FORWARD       4
#define DIRECTION_BACKWARD      8

// time after button "long press" is detected (in milliseconds)
#define BUTTON_LONG_PRESS_TIME  2000

// ----------------------------------------------------------------------------


uint8_t buttonLongPressed = 0;
unsigned long buttonPressedStartTime = 0;

uint8_t wifiConnected = 0;

WiFiClient client;
WiFiServer server(80);

// millisecond timer object
os_timer_t msecTimer;

// led blinker class instance
LedBlinker eyesLeds;


// ----------------------------------------------------------------------------


// this function is attached to interrupt and called automatically once a millisecond
void msecTimerCallback(void* pArg)
    {
    unsigned long now = millis();
    //
    // checks for button long press (2 seconds)
    if (buttonPressedStartTime != 0)
        {
        if ((now - buttonPressedStartTime) > BUTTON_LONG_PRESS_TIME)
            {
            buttonPressedStartTime = 0;
            buttonLongPressed = 1;
            }
        }
    else
        {
        buttonPressedStartTime = (digitalRead(BUTTON_PIN) == 1) ? now : 0;
        }
    //
    // updates eyes leds
    eyesLeds.update(now);
    }


// ----------------------------------------------------------------------------


// tries wifi connection with last stored credentials
void tryWiFiConnection()
    {
    wifiConnected = 0;
    //
    WiFi.mode(WIFI_STA);
    WiFi.begin(WiFi.SSID().c_str(), WiFi.psk().c_str()); // tries connection with data stored in EEPROM (from last succesful connection)
    WiFi.onEvent(onWiFiEvent);
    Serial.println("trying connection with last stored credentials...");
    uint8_t retries = 10;
    while (WiFi.status() == WL_DISCONNECTED && retries > 0)
        {          // last saved credentials
        delay(500);
        retries--;
        Serial.print(".");
        }
    //
    if (WiFi.status() == WL_DISCONNECTED)
        Serial.println("connection timeout!");
    }


// ----------------------------------------------------------------------------


// callback function called by WiFi manager on WiFi events
void onWiFiEvent(WiFiEvent_t event)
    {
    switch (event)
        {
        case WIFI_EVENT_MODE_CHANGE:
            Serial.println("WIFI_EVENT_MODE_CHANGE");
            break;
        case WIFI_EVENT_STAMODE_CONNECTED:
            Serial.println("WIFI_EVENT_STAMODE_CONNECTED");
            Serial.printf("Connected to %s. Asking for IP address.\r\n", WiFi.BSSIDstr().c_str());
            break;
        case WIFI_EVENT_STAMODE_GOT_IP:
            Serial.println("WIFI_EVENT_STAMODE_GOT_IP");
            Serial.printf("Got IP: %s\r\n", WiFi.localIP().toString().c_str());
            wifiConnected = WiFi.status() == WL_CONNECTED ? 1 : 0;
            Serial.printf("Connected: %s\r\n", wifiConnected == 1 ? "yes" : "no");
            //
            // does rapid blinking on eyes leds (10 blinks)
            eyesLeds.blink(20, 100, 10);
            //
            // starts local HTTP server
            server.begin();
            break;
        case WIFI_EVENT_STAMODE_DISCONNECTED:
            Serial.println("WIFI_EVENT_STAMODE_DISCONNECTED");
            Serial.printf("Disconnected from SSID: %s\n", WiFi.BSSIDstr().c_str());
            WiFi.reconnect();
            break;
        default:
            Serial.print(event);
            Serial.println(" (unhandled)");
            break;
        }
    }


// -----------------------------------------------------------------------------


bool tryWPSConnection()
    {
    // taken from https://gist.github.com/copa2/fcc718c6549721c210d614a325271389
    //
    Serial.println("WPS config start");
    bool wpsSuccess = WiFi.beginWPSConfig();
    if (wpsSuccess)
        {
        // Well this means not always success :-/ in case of a timeout we have an empty ssid
        String newSSID = WiFi.SSID();
        if (newSSID.length() > 0)
            {
            // WPSConfig has already connected in STA mode successfully to the new station. 
            Serial.printf("WPS finished. Connected successfull to SSID '%s'\n", newSSID.c_str());
            }
        else
            wpsSuccess = false;
        }
    return wpsSuccess;
    }


// -----------------------------------------------------------------------------


void setup()
    {
    // initializes all motor control/enable pins as output
    pinMode(LEFT_MOTOR_FORWARD, OUTPUT);
    pinMode(LEFT_MOTOR_BACKWARD, OUTPUT);
    pinMode(LEFT_MOTOR_ENABLE, OUTPUT);
    pinMode(RIGHT_MOTOR_FORWARD, OUTPUT);
    pinMode(RIGHT_MOTOR_BACKWARD, OUTPUT);
    pinMode(RIGHT_MOTOR_ENABLE, OUTPUT);
    // initializes button pin as input
    pinMode(BUTTON_PIN, INPUT);
    // initializes eyes leds pin as output
    pinMode(EYES_LEDS, OUTPUT);
    digitalWrite(EYES_LEDS, LOW);
    //
    // initializes led blinker
    eyesLeds.begin(EYES_LEDS);
    //
    // starts millisecond timer and attaches msecTimerCallback function to it
    os_timer_setfn(&msecTimer, msecTimerCallback, NULL);
    os_timer_arm(&msecTimer, 1, true);
    //
    // starts serial
    Serial.begin(115200);
    delay(50);
    Serial.println("\n\n");
    Serial.println("\nNodeMCU WiFi Bot - version " _VERSION_ " started");
    delay(10);
    //
    // tries wifi connection with stored credentials
    eyesLeds.blink(20, 250);
    tryWiFiConnection();
    //
    }


// ----------------------------------------------------------------------------


void loop()
    {
    // checks if we have "long pressed" the button
    if (buttonLongPressed)
        {
        // in this case, erases stored credentials an tries WPS connection
        buttonLongPressed = 0;
        //
        WiFi.disconnect();
        eyesLeds.blink(20, 500);
        tryWPSConnection();
        }
    //
    // checks for incoming connections
    client = server.available();
    if (client)
        {
        bool validRequest = true;
        // waits for client to become available
        while (!client.available()) delay(1);
        // reads request path from HTTP request
        String request = client.readStringUntil('\r');
        Serial.print("request: ");
        Serial.println(request);
        request.remove(0, 5);
        request.remove(request.length() - 9, 9);
        //
        // if request is "forward", calls "goForward" function
        if (request == "forward")
            {
            sendHttpOkResponse(&client, NULL);
            goForward();
            }
        // if request is "backward", calls "goBackward" function
        else if (request == "backward")
            {
            sendHttpOkResponse(&client, NULL);
            goBackward();
            }
        // if request is "left", calls "turnLeft" function
        else if (request == "left")
            {
            sendHttpOkResponse(&client, NULL);
            turnLeft();
            }
        // if request is "right", calls "turnRight" function
        else if (request == "right")
            {
            sendHttpOkResponse(&client, NULL);
            turnRight();
            }
        // if request is "stop", calls "stop" function
        else if (request == "stop")
            {
            sendHttpOkResponse(&client, NULL);
            stop();
            }
        // if request is "whoareyou", returns "nodemcuwifibot"
        // NOTE: this request is used by app to find motor in the network
        else if (request == "whoareyou")
            {
            sendHttpOkResponse(&client, "nodemcuwifibot");
            }
        else
            {
            validRequest = false;
            }
        //
        if (validRequest)
            // does rapid blinking on eyes leds (3 blinks)
            eyesLeds.blink(20, 100, 3);
        }
    }


// ----------------------------------------------------------------------------


void sendHttpOkResponse(WiFiClient* client, const char* data)
    {
    if (client)
        {
        client->println("HTTP/1.1 200 OK");
        client->println("Content-type:text/plain");
        client->println("Connection: close");
        client->println();
        if (data)
            client->println(data);
        }
    }


// ----------------------------------------------------------------------------


void setMotorDirection(uint8_t motor, uint8_t direction)
    {
    int forward_pin = 0;
    int backward_pin = 0;
    if (motor == LEFT_MOTOR)
        {
        forward_pin = LEFT_MOTOR_FORWARD;
        backward_pin = LEFT_MOTOR_BACKWARD;
        }
    else if (motor == RIGHT_MOTOR)
        {
        forward_pin = RIGHT_MOTOR_FORWARD;
        backward_pin = RIGHT_MOTOR_BACKWARD;
        }
    if (forward_pin && backward_pin)
        {
        digitalWrite(forward_pin, (direction == DIRECTION_FORWARD) ? HIGH : LOW);
        digitalWrite(backward_pin, (direction == DIRECTION_FORWARD) ? LOW : HIGH);
        }
    }


// ----------------------------------------------------------------------------


void goForward(void)
    {
    // sets both motors direction
    setMotorDirection(LEFT_MOTOR, DIRECTION_FORWARD);
    setMotorDirection(RIGHT_MOTOR, DIRECTION_FORWARD);
    // then enables both motors
    digitalWrite(LEFT_MOTOR_ENABLE, HIGH);
    digitalWrite(RIGHT_MOTOR_ENABLE, HIGH);
    }


// ----------------------------------------------------------------------------


void goBackward(void)
    {
    // sets both motors direction
    setMotorDirection(LEFT_MOTOR, DIRECTION_BACKWARD);
    setMotorDirection(RIGHT_MOTOR, DIRECTION_BACKWARD);
    // then enables both motors
    digitalWrite(LEFT_MOTOR_ENABLE, HIGH);
    digitalWrite(RIGHT_MOTOR_ENABLE, HIGH);
    }


// ----------------------------------------------------------------------------


void turnLeft(void)
    {
    // sets both motors direction
    setMotorDirection(LEFT_MOTOR, DIRECTION_BACKWARD);  // left wheel to forward
    setMotorDirection(RIGHT_MOTOR, DIRECTION_FORWARD);  // right wheel to backward
    // then enables both motors
    digitalWrite(LEFT_MOTOR_ENABLE, HIGH);
    digitalWrite(RIGHT_MOTOR_ENABLE, HIGH);
    }


// ----------------------------------------------------------------------------


void turnRight(void)
    {
    // sets both motors direction
    setMotorDirection(LEFT_MOTOR, DIRECTION_FORWARD);  // left wheel to forward
    setMotorDirection(RIGHT_MOTOR, DIRECTION_BACKWARD);  // right wheel to backward
    // then enables both motors
    digitalWrite(LEFT_MOTOR_ENABLE, HIGH);
    digitalWrite(RIGHT_MOTOR_ENABLE, HIGH);
    }


// ----------------------------------------------------------------------------


void stop(void)
    {
    digitalWrite(LEFT_MOTOR_ENABLE, LOW);
    digitalWrite(RIGHT_MOTOR_ENABLE, LOW);
    digitalWrite(LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(LEFT_MOTOR_BACKWARD, LOW);
    digitalWrite(RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(RIGHT_MOTOR_BACKWARD, LOW);
    }


// ----------------------------------------------------------------------------
