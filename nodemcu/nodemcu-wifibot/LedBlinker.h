#ifndef _LEDBLINKER_H
#define _LEDBLINKER_H

// -----------------------------------------------------------------------------
//
// Basic Led Blinker class
// Written by Salvatore Carotenuto of OpenMakersItaly
//
// -----------------------------------------------------------------------------

#include <Arduino.h>

// -----------------------------------------------------------------------------

typedef struct led_state_t
    {
    uint8_t state;
    uint32_t startTime;
    uint16_t durationMsecs;
    } led_state;

// -----------------------------------------------------------------------------


class LedBlinker
    {
    public:
        void begin(int ledPin);
        //
        void blink(uint16_t onMsecs, uint16_t offMsecs, uint8_t repeats = 0);
        void on();
        void off();
        //
        void update(uint32_t currentTimeMsecs);
    private:
        int ledPin;
        //
        led_state* nextState;
        uint8_t nextStateIndex;
        uint8_t totalStates;
        uint8_t repeats;
        bool loop;
        //
        led_state states[2];
    };


// -----------------------------------------------------------------------------

#endif

