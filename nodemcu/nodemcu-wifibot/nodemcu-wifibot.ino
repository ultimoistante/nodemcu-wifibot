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
 //                      |            D8 [15] |
 //                      |         D9/RX [03] |
 //                      |        D10/TX [01] |
 //                      |                GND |
 //                      |                3v3 |
 //                      +--------------------+


 /* include library */
#include <ESP8266WiFi.h>

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


 /* define port */
WiFiClient client;
WiFiServer server(80);

/* WIFI settings */
const char* ssid = "UNKNOWN";
const char* password = "the.cat.is.on.the.tablet";

/* data received from application */
String data = "";


// ----------------------------------------------------------------------------


void setup()
    {
    //
    pinMode(EYES_LEDS, OUTPUT);
    //
    // initializes all motor control/enable pins as output
    pinMode(LEFT_MOTOR_FORWARD, OUTPUT);
    pinMode(RIGHT_MOTOR_FORWARD, OUTPUT);
    pinMode(LEFT_MOTOR_BACKWARD, OUTPUT);
    pinMode(RIGHT_MOTOR_BACKWARD, OUTPUT);
    pinMode(LEFT_MOTOR_ENABLE, OUTPUT);
    pinMode(RIGHT_MOTOR_ENABLE, OUTPUT);

    digitalWrite(EYES_LEDS, LOW);

    Serial.begin(115200);
    delay(10);
    Serial.println('\n');

    WiFi.begin(ssid, password);             // Connect to the network
    Serial.print("Connecting to ");
    Serial.print(ssid); Serial.println(" ...");

    int i = 0;
    while (WiFi.status() != WL_CONNECTED)
        { // Wait for the Wi-Fi to connect
        digitalWrite(EYES_LEDS, HIGH);
        delay(50);
        digitalWrite(EYES_LEDS, LOW);
        delay(950);
        Serial.print(++i); Serial.print(' ');
        }

    Serial.println('\n');
    digitalWrite(EYES_LEDS, HIGH);
    Serial.println("Connection established!");
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
    //
    // start server communication
    server.begin();
    }


// ----------------------------------------------------------------------------


void loop()
    {
    /* If the server available, run the "checkClient" function */
    client = server.available();
    if (!client) return;
    data = checkClient();

    Serial.print("Data: ");
    Serial.println(data);

    /************************ Run function according to incoming data from application *************************/

    /* If the incoming data is "forward", run the "MotorForward" function */
    if (data == "forward")
        {
        //server.send(200, "text/plain", "OK");
        client.println("ok");
        MotorForward();
        }
    /* If the incoming data is "backward", run the "MotorBackward" function */
    else if (data == "backward")
        {
        //server.send(200, "text/plain", "OK");
        client.println("ok");
        MotorBackward();
        }
    /* If the incoming data is "left", run the "TurnLeft" function */
    else if (data == "left")
        {
        //server.send(200, "text/plain", "OK");
        client.println("ok");
        TurnLeft();
        }
    /* If the incoming data is "right", run the "TurnRight" function */
    else if (data == "right")
        {
        //server.send(200, "text/plain", "OK");
        client.println("ok");
        TurnRight();
        }
    /* If the incoming data is "stop", run the "MotorStop" function */
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
        }
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
String checkClient(void)
    {
    while (!client.available())
        delay(1);
    String request = client.readStringUntil('\r');
    Serial.print("request: ");
    Serial.println(request);
    request.remove(0, 5);
    request.remove(request.length() - 9, 9);
    return request;
    }
