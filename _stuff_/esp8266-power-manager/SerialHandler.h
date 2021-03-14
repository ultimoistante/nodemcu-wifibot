#ifndef _POWERMANAGER_SERIALHANDLER_H
#define _POWERMANAGER_SERIALHANDLER_H

// -----------------------------------------------------------------------------

#include <Arduino.h>
#include "utils.h"


// -----------------------------------------------------------------------------

class SerialHandler
    {
    public:
        void begin(unsigned int baudrate);
        //
        // void printMessage(const char* prefix, const char* format, ...);
        void printMessage(const char* prefix, const char* message);
        //
        void logInfo(const char* message) { this->printMessage("[I]", message); }
        void logWarning(const char* message) { this->printMessage("[W]", message); }
        void logDebug(const char* message) { this->printMessage("[D]", message); }
        //
        void doReceive();
        //void setTokenCharSeparator(char separator) { this->separator_char = separator; }
        void setRxTokenCharSeparators(char* separators) { strcpy(this->separator_chars, separators); }
        void setRxEndOfLineChar(char terminator) { this->eol_char = terminator; }
        //
        //void setMessageHandler(void (* onMessageCallback)(String*, int)) { this->onMessageCallback = onMessageCallback; }
        void setMessageHandler(void (*onMessageCallback)(char**, int)) { this->onMessageCallback = onMessageCallback; }
        //
    private:
        // char printBuffer[256];
        char            eol_char;
        //char            separator_char;
        char            separator_chars[3];
        //String			rxBuffer;
        char			rxBuffer[120];
        unsigned char   rxBufferPtr;
        byte			reading;
        //String			bufferTokens[8];
        char* bufferTokens[16];
        //unsigned char	bufferTokensLength;
        String			response;
        //
        void processLine();
        //void (* onMessageCallback)(String*, int);
        void (*onMessageCallback)(char**, int);
    };

// -----------------------------------------------------------------------------

#endif
