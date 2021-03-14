#ifndef _POWERMANAGER_WEBSERVICEHANDLER_H
#define _POWERMANAGER_WEBSERVICEHANDLER_H

#include <ESP8266WiFi.h>

// -----------------------------------------------------------------------------


class WebServiceHandler
	{
	public:
		// WebServiceHandler() { this->wifiServer = wifiServer; }
		//
		void begin(WiFiServer* wifiServer, unsigned long timeout);
		//
		void handleWebRequest();
		//
		// void update();
		// //
		// void setOnExpiredCallback(void (* onExpiredCallback)()) { this->onExpiredCallback = onExpiredCallback; }
		//
	private:
        WiFiServer* wifiServer;
		//
		unsigned long currentTime;
		unsigned long previousTime;
		unsigned long timeoutMsec;
		//
		// Variable to store the HTTP request
		String header;

		// unsigned long milliseconds;
		// unsigned long remaining;
		// //
		// void (* onExpiredCallback)();
	};


// -----------------------------------------------------------------------------


#endif
