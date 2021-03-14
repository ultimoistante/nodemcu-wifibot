#include "SerialHandler.h"

// -----------------------------------------------------------------------------

void SerialHandler::begin(unsigned int baudrate)
    {
    this->reading = 0;
    this->eol_char = '\n';
    //
    Serial.begin(baudrate);
    while (!Serial); // wait for serial port to connect. Needed for native USB port only
    Serial.println();
    }

// -----------------------------------------------------------------------------


// void SerialHandler::logInfo(const char* message)
//     {this->printMessage("[I]", message);}


// -----------------------------------------------------------------------------


void SerialHandler::printMessage(const char* prefix, const char* message)
// void SerialHandler::printMessage(const char* prefix, const char* format, ...)
    {
    // va_list argptr;
    // va_start(argptr, format);

    // Serial.vprintf_P(format, argptr);

    // va_end(argptr);
    Serial.printf("%s %s\n", prefix, message);
    }


// -----------------------------------------------------------------------------


void SerialHandler::doReceive()
    {
    if (Serial.available())
        {
        if (this->reading == 0)
            {
            //this->rxBuffer = "";
            this->rxBufferPtr = 0;
            this->rxBuffer[this->rxBufferPtr] = '\0';
            }
        //
        this->reading = 1;
        char inByte = Serial.read();
        //
        if (this->reading) // still waiting for end of message
            {
            if (inByte == this->eol_char) // received end of message character
                {
                this->reading = 0;
                this->processLine();
                //onboardSerialProcessMessage();
                }
            else
                {
                //this->rxBuffer += inByte; // appends message character at end of buffer
                this->rxBuffer[this->rxBufferPtr] = inByte;
                this->rxBufferPtr++;
                this->rxBuffer[this->rxBufferPtr] = '\0';
                }
            }
        }
    }


// -----------------------------------------------------------------------------


void SerialHandler::processLine()
    {
    // splits message in tokens
    //int tokenCount = Utils::stringSplit(this->rxBuffer, this->separator_char, this->bufferTokens, 8);
    int tokenCount = Utils::charStringSplit(this->rxBuffer, this->separator_chars, this->bufferTokens);
    // then calls message handler callback
    this->onMessageCallback(this->bufferTokens, tokenCount);
    }


// -----------------------------------------------------------------------------
