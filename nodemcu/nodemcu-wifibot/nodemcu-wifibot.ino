/*
 * Mert Arduino and Tech - YouTube
 * WiFi Robot controlled by Phone (WiFibot/Android/IoT/ESP8266)
 * NodeMCU ESP8266 Tutorial 03
 * Please Subscribe for Support - https://goo.gl/RLT6DT
 */

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


// libraries
#include <ESP8266WiFi.h>
#include "LedBlinker.h"

// ----------------------------------------------------------------------------

// ---- pin definitions
//
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
#define BUTTON_LONG_PRESS_TIME  2000

// ----------------------------------------------------------------------------

uint8_t buttonLongPressed = 0;
unsigned long buttonPressedStartTime = 0;

uint8_t wifiConnected = 0;

/* define port */
// WiFiClient client;
// WiFiServer server(80);

/* WIFI settings */
// const char* ssid = "UNKNOWN";
// const char* password = "the.cat.is.on.the.tablet";

/* data received from application */
String data = "";

LedBlinker eyesLeds;


// ----------------------------------------------------------------------------

// millisecond timer object
os_timer_t msecTimer;
// uint8_t timerTickCounter = 0;

// ----------------------------------------------------------------------------

// Current time
// unsigned long currentTime = millis();
// Previous time
// unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
// const long timeoutTime = 2000;

// ----------------------------------------------------------------------------


void msecTimerCallback(void* pArg)
    {
    unsigned long now = millis();
    //
    // checks for button long press (2 s)
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
    eyesLeds.update(now);
    // statusLedHandler.update();
    // deviceStatusLed.Update();
    // rpiStatusLed.Update();
    //
    // powerButton.check();
    // wpsButton.check();
    }


// ----------------------------------------------------------------------------

void tryWiFiConnection()
    {
    wifiConnected = 0;
    eyesLeds.blink(20, 200);
    //
    WiFi.mode(WIFI_STA);
    // WiFi.begin (ssid, password);
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

void onWiFiEvent(WiFiEvent_t event)
    {
    // instance->serialHandler->logDebug("[WiFi-event] event: ");
    // 
    switch (event)
        {
        case WIFI_EVENT_MODE_CHANGE:
            Serial.println("WIFI_EVENT_MODE_CHANGE");
            // instance->statusLedHandler->setDeviceStatus(StatusLedHandler::DEVICE_STATUS_LED_MODE_NOT_CONNECTED);
            /*
            // WPS button I/O setup
            // pinMode(0,OUTPUT);         // Use GPIO0
            // digitalWrite(0,LOW);       // for hardware safe operation.
            pinMode(WPS_BUTTON_PIN, INPUT_PULLUP);  // Push Button for GPIO2 active LOW
            // Serial.printf("\nCould not connect to WiFi. state='%d'\n", status);
            Serial.printf("\nCould not connect to WiFi.\n");
            Serial.println("Please press WPS button on your router, until mode is indicated.");
            Serial.println("next press the ESP module WPS button, router WPS timeout = 2 minutes");
            //
            while(digitalRead(WPS_BUTTON_PIN) == HIGH)  // wait for WPS Button active
                yield(); // do nothing, allow background work (WiFi) in while loops
            //
            setDeviceStatusLed(DEVICE_STATUS_LED_MODE_TRYING_WPS);
            Serial.println("WPS button pressed");

            if(!tryWPSConnection())
                {
                Serial.println("Failed to connect with WPS :-(");
                }
            else
                {
                WiFi.begin(WiFi.SSID().c_str(),WiFi.psk().c_str()); // reading data from EPROM,
                while (WiFi.status() == WL_DISCONNECTED)
                    {          // last saved credentials
                    delay(500);
                    Serial.print("."); // show wait for connect to AP
                    }
                //   pinMode(0,INPUT);    // GPIO0, LED OFF, show WPS & connect OK
                }*/
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
            // eyesLeds.blink(20, 1000);
            eyesLeds.off();

            // instance->statusLedHandler->setDeviceStatus(StatusLedHandler::DEVICE_STATUS_LED_MODE_CONNECTED);
            // setDeviceStatusLed(DEVICE_STATUS_LED_MODE_CONNECTED);
            // digitalWrite (ONBOARDLED, LOW); // Turn on LED
            // wifiFirstConnection = true;
            break;
        case WIFI_EVENT_STAMODE_DISCONNECTED:
            Serial.println("WIFI_EVENT_STAMODE_DISCONNECTED");
            Serial.printf("Disconnected from SSID: %s\n", WiFi.BSSIDstr().c_str());
            // setDeviceStatusLed(DEVICE_STATUS_LED_MODE_CONNECTING);
            //Serial.printf ("Reason: %d\n", info.disconnected.reason);
            // digitalWrite (ONBOARDLED, HIGH); // Turn off LED
            //NTP.stop(); // NTP sync can be disabled to avoid sync errors
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
    // from https://gist.github.com/copa2/fcc718c6549721c210d614a325271389
    // wpstest.ino
    // this->statusLedHandler->setDeviceStatus(StatusLedHandler::DEVICE_STATUS_LED_MODE_TRYING_WPS);
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
            {
            wpsSuccess = false;
            }
        }
    return wpsSuccess;
    }


// -----------------------------------------------------------------------------


void setup()
    {
    // initializes all motor control/enable pins as output
    pinMode(LEFT_MOTOR_FORWARD, OUTPUT);
    pinMode(RIGHT_MOTOR_FORWARD, OUTPUT);
    pinMode(LEFT_MOTOR_BACKWARD, OUTPUT);
    pinMode(RIGHT_MOTOR_BACKWARD, OUTPUT);
    pinMode(LEFT_MOTOR_ENABLE, OUTPUT);
    pinMode(RIGHT_MOTOR_ENABLE, OUTPUT);
    // initializes button pin as input
    pinMode(BUTTON_PIN, INPUT);
    // initializes eyes leds pin as output
    pinMode(EYES_LEDS, OUTPUT);
    digitalWrite(EYES_LEDS, LOW);
    // initializes led blinker
    eyesLeds.begin(EYES_LEDS);
    //
    // starts millisecond timer
    os_timer_setfn(&msecTimer, msecTimerCallback, NULL);
    os_timer_arm(&msecTimer, 1, true);
    //
    Serial.begin(115200);
    delay(10);
    Serial.println('\n');


    tryWiFiConnection();
    // WiFi.begin(ssid, password);             // Connect to the network
    // Serial.print("Connecting to ");
    // Serial.print(ssid); Serial.println(" ...");

    // int i = 0;
    // while (WiFi.status() != WL_CONNECTED)
    //     { // Wait for the Wi-Fi to connect
    //     digitalWrite(EYES_LEDS, HIGH);
    //     delay(50);
    //     digitalWrite(EYES_LEDS, LOW);
    //     delay(950);
    //     Serial.print(++i); Serial.print(' ');
    //     }

    // Serial.println('\n');
    // digitalWrite(EYES_LEDS, HIGH);
    // Serial.println("Connection established!");
    // Serial.print("IP address:\t");
    // Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
    //
    // start server communication
    // server.begin();
    }


// ----------------------------------------------------------------------------


void loop()
    {
    if (buttonLongPressed)
        {
        Serial.println("BUTTON!");
        buttonLongPressed = 0;
        //
        WiFi.disconnect();
        tryWPSConnection();
        }
    // Serial.print("button: ");
    // Serial.println(digitalRead(BUTTON_PIN));
    // delay(10);

    /*
    // If the server available, run the "checkClient" function
    client = server.available();
    if (!client) return;
    data = checkClient();

    Serial.print("Data: ");
    Serial.println(data);

    // Run function according to incoming data from application

    // If the incoming data is "forward", run the "MotorForward" function
    if (data == "forward")
        {
        //server.send(200, "text/plain", "OK");
        client.println("ok");
        MotorForward();
        }
    // If the incoming data is "backward", run the "MotorBackward" function
    else if (data == "backward")
        {
        //server.send(200, "text/plain", "OK");
        client.println("ok");
        MotorBackward();
        }
    // If the incoming data is "left", run the "TurnLeft" function
    else if (data == "left")
        {
        //server.send(200, "text/plain", "OK");
        client.println("ok");
        TurnLeft();
        }
    // If the incoming data is "right", run the "TurnRight" function
    else if (data == "right")
        {
        //server.send(200, "text/plain", "OK");
        client.println("ok");
        TurnRight();
        }
    // If the incoming data is "stop", run the "MotorStop" function
    else if (data == "stop")
        {
        //server.send(200, "text/plain", "OK");
        client.println("ok");
        MotorStop();
        }
    // If the incoming data is "whoareyou", returns "nodemcuwifibot"
    else if (data == "whoareyou")
        {
        // server.send(200, "text/plain", "nodemcuwifibot");
        client.println("HTTP/1.1 200 OK");
        client.println("Content-type:text/plain");
        client.println("Connection: close");
        client.println();
        client.println("nodemcuwifibot");
        }*/
    }


// ----------------------------------------------------------------------------


void MotorForward(void)
    {
    digitalWrite(LEFT_MOTOR_ENABLE, HIGH);
    digitalWrite(RIGHT_MOTOR_ENABLE, HIGH);
    digitalWrite(LEFT_MOTOR_FORWARD, HIGH);
    digitalWrite(RIGHT_MOTOR_FORWARD, HIGH);
    digitalWrite(LEFT_MOTOR_BACKWARD, LOW);
    digitalWrite(RIGHT_MOTOR_BACKWARD, LOW);
    }

/********************************************* BACKWARD *****************************************************/
void MotorBackward(void)
    {
    digitalWrite(LEFT_MOTOR_ENABLE, HIGH);
    digitalWrite(RIGHT_MOTOR_ENABLE, HIGH);
    digitalWrite(LEFT_MOTOR_BACKWARD, HIGH);
    digitalWrite(RIGHT_MOTOR_BACKWARD, HIGH);
    digitalWrite(LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(RIGHT_MOTOR_FORWARD, LOW);
    }

/********************************************* TURN LEFT *****************************************************/
void TurnLeft(void)
    {
    digitalWrite(LEFT_MOTOR_ENABLE, HIGH);
    digitalWrite(RIGHT_MOTOR_ENABLE, HIGH);
    digitalWrite(LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(RIGHT_MOTOR_FORWARD, HIGH);
    digitalWrite(RIGHT_MOTOR_BACKWARD, LOW);
    digitalWrite(LEFT_MOTOR_BACKWARD, HIGH);
    }

/********************************************* TURN RIGHT *****************************************************/
void TurnRight(void)
    {
    digitalWrite(LEFT_MOTOR_ENABLE, HIGH);
    digitalWrite(RIGHT_MOTOR_ENABLE, HIGH);
    digitalWrite(LEFT_MOTOR_FORWARD, HIGH);
    digitalWrite(RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(RIGHT_MOTOR_BACKWARD, HIGH);
    digitalWrite(LEFT_MOTOR_BACKWARD, LOW);
    }

/********************************************* STOP *****************************************************/
void MotorStop(void)
    {
    digitalWrite(LEFT_MOTOR_ENABLE, LOW);
    digitalWrite(RIGHT_MOTOR_ENABLE, LOW);
    digitalWrite(LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(LEFT_MOTOR_BACKWARD, LOW);
    digitalWrite(RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(RIGHT_MOTOR_BACKWARD, LOW);
    }

/********************************** RECEIVE DATA FROM the APP ******************************************/
/*String checkClient(void)
    {
    while (!client.available())
        delay(1);
    String request = client.readStringUntil('\r');
    Serial.print("request: ");
    Serial.println(request);
    request.remove(0, 5);
    request.remove(request.length() - 9, 9);
    return request;
    }*/
