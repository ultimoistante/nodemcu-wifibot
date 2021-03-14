#ifndef _POWERMANAGER_WIFIHANDLER_H
#define _POWERMANAGER_WIFIHANDLER_H

#include <ESP8266WiFi.h>
#include "StatusLedHandler.h"
#include "SerialHandler.h"


// -----------------------------------------------------------------------------

class WiFiHandler
	{
	public:
		static const uint8_t STATUS_STAMODE_NOT_CONNECTED;
		//
		static WiFiHandler* instance;
		//
		void begin(StatusLedHandler* statusLedHandler, SerialHandler* serialHandler);
		//
		bool tryWPSConnection();
		//
		uint8_t getCurrentStatus() { return this->currentStatus; }
		//
		// void handleWebRequest();
		//
		// void update();
		// //
		// void setOnExpiredCallback(void (* onExpiredCallback)()) { this->onExpiredCallback = onExpiredCallback; }
		//
	private:
		StatusLedHandler* statusLedHandler;
		SerialHandler* serialHandler;
		//
		uint8_t currentStatus;
		// //
		// unsigned long currentTime;
		// unsigned long previousTime;
		// unsigned long timeoutMsec;
		// //
		// // Variable to store the HTTP request
		// String header;

		// unsigned long milliseconds;
		// unsigned long remaining;
		// //
		// void (* onExpiredCallback)();
		WiFiEvent_t lastEvent;

		static void onEvent(WiFiEvent_t event);
	};

// -----------------------------------------------------------------------------

#endif
