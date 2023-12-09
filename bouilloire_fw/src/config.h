#ifndef CONFIG_H
#define CONFIG_H

#define PIN_RELAY_A 2
#define PIN_RELAY_B 3
#define PIN_SDA 4
#define PIN_SCL 5
#define PIN_TEMPERATURE 26

#define PIN_BTN_ON_OFF 18
#define PIN_BTN_PLUS 19
#define PIN_BTN_MINUS 20
#define PIN_BTN_PROG 21

#define PIN_LEDS 16

#define ABSOLUTE_MIN_TEMPERATURE 20
#define ABSOLUTE_MAX_TEMPERATURE 100

//coefficients of the equation temp = a * x + b
#define TEMP_CONVERSION_A 0.0341456166419019
#define TEMP_CONVERSION_B (-39.8855869242199 + 5)

//Value in degrees. If the measured value is lower than this, the relays are forced to off
#define MINIMUM_VALID_TEMPERATURE 0

#define TEMPERATURE_HYSTERESIS 5

#define READ_TEMP_INTERVAL_US 1000000

// 5 minutes without touching anything switches off the system
#define AUTO_OFF_TIMEOUT_US (5*60*1000000) 
//#define AUTO_OFF_TIMEOUT_US 10000000


#endif //CONFIG_H