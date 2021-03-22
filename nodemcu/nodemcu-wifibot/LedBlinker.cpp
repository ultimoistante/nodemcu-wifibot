#include "LedBlinker.h"

// -----------------------------------------------------------------------------
//
// Basic Led Blinker class
// Written by Salvatore Carotenuto of OpenMakersItaly
//
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------


void LedBlinker::begin(int ledPin)
    {
    this->ledPin = ledPin;
    this->nextState = NULL;
    this->totalStates = 0;
    //
    pinMode(this->ledPin, OUTPUT);
    }


// -----------------------------------------------------------------------------


void LedBlinker::blink(uint16_t onMsecs, uint16_t offMsecs, uint8_t repeats)
    {
    this->states[0].state = 1;
    this->states[0].startTime = 0;
    this->states[0].durationMsecs = onMsecs;
    this->states[1].state = 0;
    this->states[1].startTime = 0;
    this->states[1].durationMsecs = offMsecs;
    //    
    this->repeats = repeats;
    this->loop = (this->repeats == 0);
    //
    this->nextState = &(this->states[0]);
    this->nextStateIndex = 0;
    this->totalStates = 2;
    }


// -----------------------------------------------------------------------------


void LedBlinker::on()
    {
    this->nextState = NULL;
    digitalWrite(this->ledPin, 1);
    }


// -----------------------------------------------------------------------------


void LedBlinker::off()
    {
    this->nextState = NULL;
    digitalWrite(this->ledPin, 0);
    }


// -----------------------------------------------------------------------------


void LedBlinker::update(uint32_t currentTimeMsecs)
    {
    if (this->nextState)
        {
        if (currentTimeMsecs >= this->nextState->startTime)
            {
            digitalWrite(this->ledPin, this->nextState->state);
            uint16_t currentDuration = this->nextState->durationMsecs;
            // advances on next state
            bool stop = false;
            if (this->nextStateIndex < this->totalStates - 1)
                this->nextStateIndex++;
            else
                {
                if (this->repeats)
                    this->repeats--;
                if (!this->loop && this->repeats == 0)
                    stop = true;
                // 
                this->nextStateIndex = 0;
                }
            if (!stop)
                {
                this->nextState = &(this->states[this->nextStateIndex]);
                this->nextState->startTime = currentTimeMsecs + currentDuration;
                }
            else
                this->off();
            }
        }
    }


// -----------------------------------------------------------------------------
