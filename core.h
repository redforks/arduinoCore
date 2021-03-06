#ifndef Core_h
#define Core_h

#if ARDUINO < 100
#include <WProgram.h>
#include <pins_arduino.h>  // fix for broken pre 1.0 version - TODO TEST
#else
#include <Arduino.h>
#endif

#ifndef DIGITAL_VALUES
#error "define DIGITAL_VALUES to your count of digital values"
#endif

#ifndef ANALOG_VALUES
#error "define DIGITAL_VALUES to your count of analog values"
#endif

namespace core {
// type of digital and analog ids.
typedef byte idType;

// callback function pointer
typedef void (*callback)(void);

// compareULong compares two ulong, return 1 if a > b, -1 if a < b, 0 if a ==
// b.
//
// If keep increase uint32_t, it will overflow eventually, compareULong
// try to do comparation relaibally. It is done by compare to flag value,
// instead of compare with 0.
int compareULong(uint32_t a, uint32_t b, uint32_t flag);

// Store of my arduino framework.
//
// Store manages two set of values, ditial and analog. These values are not
// mapped direcly to arduino I/O pins, they are mapped to logical concept of
// each application. Some are really a hardware pin, like buttons, some are
// one pin map to several values, such as temperature and humidity got from one
// sensor, some are not related to hardware at all, used to communicate between
// control loops.
//
// If value changes, Store will notify registered callbacks.
//
// To save memory and simplify, Store preallocate digital and analog values
// array,
// define DIGITAL_VALUES, ANALOG_VALUES in your application.
namespace store {
// Current value of digitals. Only read digitals, use setDigital() to
// change the value.
extern byte digitals[DIGITAL_VALUES];

// Current value of the analogs. Only read analogs, use setAnalog() to
// change the value.
extern uint16_t analogs[ANALOG_VALUES];

// defineDigital defines a new digital value, returns id of the digital value.
// Support max DIGITAL_VALUES digital values.
idType defineDigital();

// defineAnalog defines an analog value, returns id of the analog value.
// Support max ANALOG_VALUES analog values.
idType defineAnalog();

// monitor digital value changes, if one of specific value changes, the
// function get called. nIds is count of ids.
// monitor identified by its address, it is OK to call monitorDigitals() and
// monitorAnalogs() several times on a callback.
void monitorDigitals(callback f, byte nIds, ...);

// monitor analog value changes, if one of specific value changes, the
// function get called. nIds is count of ids.
// monitor identified by its address, it is OK to call monitorDigitals() and
// monitorAnalogs() several times on a callback.
void monitorAnalogs(callback f, byte nIds, ...);

// setDigital set digital value, value should be either LOW or HIGH.
void setDigital(idType id, byte val);

// setAnalog set analog value.
void setAnalog(idType id, uint16_t val);
}

// clock call functions at specific delay or interval.
namespace clock {
// interval calls the function every mills. Not calling the function
// immediately, call it by yourself.
// Returns clock id, can remove the internal by calling removeInterval().
void* interval(uint32_t mills, callback f);

// removeInterval remove the interval, ignored if id is invalid.
void removeInterval(void* id);

// delays wait for mills, then call the function.
void* delay(uint32_t mills, callback f);

// removeDelay drop the delay, ignored if the id invalid or the function has
// been called.
void removeDelay(void* id);

// check time, call delay and interval functions.
void check();
}
}

#endif
