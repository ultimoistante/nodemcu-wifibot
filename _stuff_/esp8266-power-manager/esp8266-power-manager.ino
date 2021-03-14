
/*
 * DataVault esp8266-power-manager firmware
 *
 * Written by Salvatore Carotenuto (ultimoistante)
 *
 * Changelog:
 *    2021-01-26:
 *       .added handling of raspberryPi status led to StatusLedHandler
 *       .started adding logging and receive methods to SerialHandler
 *    2020-12-30:
 *       .rewritten WiFi connection code in event-driven mode
 *       .replaced NTP client code, adding from: https://github.com/gmag11/ESPNtpClient
 *    2020-12-27:
 *       .changed pin assignments
 *       .added status led patterns
 *       .added wifi connection
 *       .added basic webserver
 *       .started adding mDNS support
 *    2020-12-16:
 *       .added jLed library
 *       .started writing
*/

// TODO: fix mDNS: https://tttapa.github.io/ESP8266/Chap08%20-%20mDNS.html

// ----------------------------------------------------------------------------
// Hardware connections:
//
//    +-------------------+
//    |A0          D0 [16]| - rPi power status LED
//    |            D1 [05]| - I2C SDA (unused)
//    |            D2 [04]| - I2C SCL (unused)
//    |            D3 [00]| - __RESERVED__
//    |            D4 [02]| - (LED_BUILTIN)
//    |                3v3| - 
//    |                GND| - 
//    |            D5 [14]| - Power button
//    |            D6 [12]| - WPS request button
//    |            D7 [13]| - Power Manager (this device) status LED
//    |            D8 [15]| - MOSFET module on/off
//    |       D9 / RX [03]| - to RaspberryPi serial TX
//    |      D10 / TX [01]| - to RaspberryPi serial RX
//    |                GND| - 
//    |                3v3| - 
//    +-------------------+
//
// ----------------------------------------------------------------------------


#include <ESP8266WiFi.h>
// #include <WiFiUdp.h>
#include <ESP8266mDNS.h>        // Include the mDNS library
#include <jled.h>
#include <AceButton.h>
using namespace ace_button;
// #include "NTPClient.h"
#include "src/ESPNtpClient/ESPNtpClient.h"
#include "defines.h"
#include "SerialHandler.h"
#include "WiFiHandler.h"
#include "WebServiceHandler.h"
#include "StatusLedHandler.h"

// ----------------------------------------------------------------------------

// user buttons
AceButton powerButton(POWER_BUTTON_PIN);
AceButton wpsButton(WPS_BUTTON_PIN);

// status leds
// auto deviceStatusLed = JLed(DEVICE_STATUS_LED_PIN).Breathe(2000).Forever();
auto deviceStatusLed = JLed(DEVICE_STATUS_LED_PIN);
auto rpiStatusLed = JLed(RPI_STATUS_LED_PIN);

bool rpiPowered = false;

// ----------------------------------------------------------------------------

SerialHandler serialHandler;

// handler for wifi connection
WiFiHandler wifiHandler;

// Set web server port number to 80
WiFiServer wifiServer(80);
WebServiceHandler webServiceHandler;

StatusLedHandler statusLedHandler;

// WiFiUDP ntpUDP;

// By default 'pool.ntp.org' is used with 60 seconds update interval and
// no offset
// NTPClient timeClient(ntpUDP, NTP_SERVER_POOL, NTP_TIME_OFFSET, NTP_UPDATE_INTERVAL);

NTPEvent_t ntpEvent; // Last triggered event
boolean ntpEventTriggered = false; // True if a time even has been triggered
double offset;
double timedelay;
bool wifiFirstConnection = false;
unsigned long nextPrintTime = 0;

// ----------------------------------------------------------------------------

// millisecond timer object
os_timer_t msecTimer;
uint8_t timerTickCounter = 0;

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
    statusLedHandler.update();
    // deviceStatusLed.Update();
    // rpiStatusLed.Update();
    //
    powerButton.check();
    wpsButton.check();
    }


// -----------------------------------------------------------------------------


void processNtpEvent(NTPEvent_t ntpEvent)
    {
    switch (ntpEvent.event)
        {
        case timeSyncd:
        case partlySync:
        case syncNotNeeded:
        case accuracyError:
            Serial.printf("[NTP-event] %s\n", NTP.ntpEvent2str(ntpEvent));
            break;
        default:
            break;
        }
    }

// -----------------------------------------------------------------------------


void handleButtonEvent(AceButton* button, uint8_t eventType, uint8_t /*buttonState*/)
    {
    uint8_t pin = button->getPin();
    //
    if (pin == POWER_BUTTON_PIN)
        {
        switch (eventType)
            {
            // Interpret a Released event as a Pressed event, to distiguish it
            // from a LongPressed event.
            case AceButton::kEventReleased:
                if (!rpiPowered)
                    {
                    setMosfetStatus(true);
                    statusLedHandler.setRaspberryPiStatus(StatusLedHandler::RPI_STATUS_BOOTING);
                    // rpiStatusLed.Breathe(500).Forever();
                    rpiPowered = true;
                    }
                break;

                // LongPressed goes in and out of edit mode.
            case AceButton::kEventLongPressed:
                if (rpiPowered)
                    {
                    rpiPowered = false;
                    setMosfetStatus(false);
                    statusLedHandler.setRaspberryPiStatus(StatusLedHandler::RPI_STATUS_BOOTING);
                    rpiStatusLed.FadeOff(500).Repeat(1);
                    }
                break;
            }
        }
    //
    else if (pin == WPS_BUTTON_PIN && eventType == AceButton::kEventReleased)
        {
        if (wifiHandler.getCurrentStatus() == WiFiHandler::STATUS_STAMODE_NOT_CONNECTED)
            {
            wifiHandler.tryWPSConnection();
            }
        }
    }


// -----------------------------------------------------------------------------


void serialReceiveMessageHandler(char** tokens, int tokenCount)
    {
    unsigned char config_changed = 0;
    String msg = "";
    //
    Serial.print("[outboardSerialMessageHandler] Token count: ");
    Serial.println(tokenCount);
    //
    // prints the extracted parameters
    for (int i = 0; i < tokenCount; i++)
        {
        Serial.print("'");
        Serial.print(tokens[i]);
        Serial.println("'");
        }
    }


// -----------------------------------------------------------------------------


void setMosfetStatus(bool status)
    {
    digitalWrite(MOSFET_CONTROL_PIN, status ? HIGH : LOW);
    }


// -----------------------------------------------------------------------------


/*void wifiConnect()
    {
    // Connect to Wi-Fi network with SSID and password
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
        {
        delay(500);
        Serial.print(".");
        }
    // Print local IP address and start web server
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    server.begin();
    // starts ntp client
    timeClient.begin();
    //
    if (!MDNS.begin("esp8266")) {             // Start the mDNS responder for esp8266.local
        Serial.println("Error setting up MDNS responder!");
    }
    Serial.println("mDNS responder started");
    }
*/


// -----------------------------------------------------------------------------


// the setup function runs once when you press reset or power the board
void setup()
    {
    //
    // Button uses the built-in pull up register.
    pinMode(POWER_BUTTON_PIN, INPUT_PULLUP);
    pinMode(WPS_BUTTON_PIN, INPUT_PULLUP);
    // sets mosfet control pin mode
    pinMode(MOSFET_CONTROL_PIN, OUTPUT);
    //
    setMosfetStatus(false);
    //
    // initialize digital pin LED_BUILTIN as an output.
    // pinMode(LED_BUILTIN, OUTPUT);
    //
    serialHandler.begin(SERIAL_BAUDRATE);
    serialHandler.setRxTokenCharSeparators("|,");
    serialHandler.setMessageHandler(serialReceiveMessageHandler);
    delay(250);
    serialHandler.logInfo("Ultimoistante's DataVault Power Manager");
    //
    // starts millisecond timer
    os_timer_setfn(&msecTimer, msecTimerCallback, NULL);
    os_timer_arm(&msecTimer, 1, true);

    // Configure the ButtonConfig with the event handler.
    ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
    buttonConfig->setEventHandler(handleButtonEvent);
    buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
    buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);
    buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);
    //
    statusLedHandler.begin(&deviceStatusLed, &rpiStatusLed);
    //
    // starts wifi connection
    // setDeviceStatusLed(DEVICE_STATUS_LED_MODE_NOT_CONNECTED);
    //delay(2000);

    wifiHandler.begin(&statusLedHandler, &serialHandler);
    delay(1000);

    // //wifiConnect();
    // WiFi.mode(WIFI_STA);
    // // WiFi.begin (ssid, password);
    // WiFi.begin(WiFi.SSID().c_str(),WiFi.psk().c_str()); // reading data from EPROM, 
    // WiFi.onEvent(onWifiEvent);
    // while (WiFi.status() == WL_DISCONNECTED) {          // last saved credentials
    //     delay(500);
    //     Serial.print(".");
    // }

    /*
    wl_status_t status = WiFi.status();
    if(status == WL_CONNECTED)
        {
        setDeviceStatusLed(DEVICE_STATUS_LED_MODE_CONNECTED);
        Serial.printf("\nConnected successful to SSID '%s'\n", WiFi.SSID().c_str());
        }
    else
        {
        // WPS button I/O setup
        // pinMode(0,OUTPUT);         // Use GPIO0
        // digitalWrite(0,LOW);       // for hardware safe operation.
        pinMode(WPS_BUTTON_PIN, INPUT_PULLUP);  // Push Button for GPIO2 active LOW
        Serial.printf("\nCould not connect to WiFi. state='%d'\n", status);
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
            }
        }
        */

        /*NTP.onNTPSyncEvent ([] (NTPEvent_t event) {
            ntpEvent = event;
            ntpEventTriggered = true;
        });


        webServiceHandler.begin(&wifiServer, WEBSERVICE_TIMEOUT_MSEC);*/
    }


// ----------------------------------------------------------------------------

// the loop function runs over and over again forever
void loop()
    {
    unsigned long now = millis();
    //
    // serial receive
    if (WiFi.isConnected())
        serialHandler.doReceive();


    /*
    //
    //   digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    //   delay(1000);                       // wait for a second
    //   digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    //   delay(1000);                       // wait for a second
    webServiceHandler.handleWebRequest();

    if (wifiFirstConnection) {
        wifiFirstConnection = false;
        NTP.setTimeZone (TZ_Europe_Rome);
        NTP.setInterval (600);
        NTP.setNTPTimeout (NTP_TIMEOUT);
        // NTP.setMinSyncAccuracy (750);
        // NTP.settimeSyncThreshold (500);
        NTP.begin (NTP_SERVER);
    }

    if (ntpEventTriggered) {
        ntpEventTriggered = false;
        processNtpEvent(ntpEvent);
    }

    if (now >= nextPrintTime)
        {
        nextPrintTime = now + NTP_PRINT_TIME_INTERVAL;
        //
        // last = millis ();
        // Serial.print (i);
        // Serial.print (" ");
        Serial.print (NTP.getTimeDateStringUs()); Serial.print (" ");
        Serial.print ("WiFi is ");
        Serial.print (WiFi.isConnected() ? "connected" : "not connected"); Serial.print (". ");
        Serial.print ("Uptime: ");
        Serial.print (NTP.getUptimeString ()); Serial.print (" since ");
        Serial.println (NTP.getTimeDateString (NTP.getFirstSyncUs ()));
        Serial.printf ("Free heap: %u\n\n", ESP.getFreeHeap ());
        // i++;
    }
    delay (0);
    // timeClient.update();
    // Serial.println(timeClient.getFormattedTime());
    // delay(1000);
    */
    }