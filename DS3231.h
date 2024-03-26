#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "esp_log.h"
#include "esp_err.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c.h"

// #define DS3231_SINGLE_DEVICE 1

#define DS3231_ADDR_I2C 0x68

#define DS3231_TIMEOUT_TIME_MS 3000
#define DS3231_TIMEOUT_TIME_TICKS pdMS_TO_TICKS(DS3231_TIMEOUT_TIME_MS)

#define DS3231_TIME_DELAY_POLLING_MS 1
#define DS3231_TIME_DELAY_POLLING_TICKS pdMS_TO_TICKS(DS3231_TIME_DELAY_POLLING_MS)


enum DS3231_ADDR_REG
{
    DS3231_ADDR_REG_CURRENT_SECOND = 0x00,
    DS3231_ADDR_REG_CURRENT_MINUTE = 0x01,
    DS3231_ADDR_REG_CURRENT_HOUR = 0x02,
    DS3231_ADDR_REG_CURRENT_DAY = 0x03,
    DS3231_ADDR_REG_CURRENT_DATE = 0x04,
    DS3231_ADDR_REG_CURRENT_MONTH_CENTURY = 0x05,
    DS3231_ADDR_REG_CURRENT_YEAR = 0x06,
    
    DS3231_ADDR_REG_ALARM1_SECOND = 0x07,
    DS3231_ADDR_REG_ALARM1_MINUTE = 0x08,
    DS3231_ADDR_REG_ALARM1_HOUR = 0x09,
    DS3231_ADDR_REG_ALARM1_DAY_DATE = 0x0A,
    
    DS3231_ADDR_REG_ALARM2_MINUTE = 0x0B,
    DS3231_ADDR_REG_ALARM2_HOUR = 0x0C,
    DS3231_ADDR_REG_ALARM2_DAY_DATE = 0x0D,

    DS3231_ADDR_REG_CONTROL = 0x0E,
    DS3231_ADDR_REG_STATUS = 0x0F,

    DS3231_ADDR_REG_AGING_OFFSET = 0x10,
    
    DS3231_ADDR_REG_TEMPERATURE_MSB = 0x11,
    DS3231_ADDR_REG_TEMPERATURE_LSB = 0x12,
};

// Current Time
enum DS3231_REG_MASK_CURRENT_SECOND
{
    DS3231_REG_MASK_CURRENT_SECOND_PLACE_TENS = 0b10001111,
    DS3231_REG_MASK_CURRENT_SECOND_PLACE_ONES = 0b11110000,
};
enum DS3231_REG_SHIFT_CURRENT_SECOND
{
    DS3231_REG_SHIFT_CURRENT_SECOND_PLACE_TENS = 4,
    DS3231_REG_SHIFT_CURRENT_SECOND_PLACE_ONES = 0,
};
enum DS3231_REG_MASK_CURRENT_MINUTE
{
    DS3231_REG_MASK_CURRENT_MINUTE_PLACE_TENS = 0b10001111,
    DS3231_REG_MASK_CURRENT_MINUTE_PLACE_ONES = 0b11110000,
};
enum DS3231_REG_SHIFT_CURRENT_MINUTE
{
    DS3231_REG_SHIFT_CURRENT_MINUTE_PLACE_TENS = 4,
    DS3231_REG_SHIFT_CURRENT_MINUTE_PLACE_ONES = 0,
};
enum DS3231_REG_MASK_CURRENT_HOUR
{
    DS3231_REG_MASK_CURRENT_HOUR_12_24_HOUR = 0b10111111,
    DS3231_REG_MASK_CURRENT_HOUR_AM_PM_PLACE_TWENTIES = 0b11011111,
    DS3231_REG_MASK_CURRENT_HOUR_PLACE_TENS = 0b11101111,
    DS3231_REG_MASK_CURRENT_HOUR_PLACE_ONES = 0b11110000,
};
enum DS3231_REG_SHIFT_CURRENT_HOUR
{
    DS3231_REG_SHIFT_CURRENT_HOUR_12_24_HOUR = 6,
    DS3231_REG_SHIFT_CURRENT_HOUR_AM_PM_PLACE_TWENTIES = 5,
    DS3231_REG_SHIFT_CURRENT_HOUR_PLACE_TENS = 4,
    DS3231_REG_SHIFT_CURRENT_HOUR_PLACE_ONES = 0,
};
enum DS3231_REG_MASK_CURRENT_DAY
{
    DS3231_REG_MASK_CURRENT_DAY_PLACE_ONES = 0b11111000,
};
enum DS3231_REG_SHIFT_CURRENT_DAY
{
    DS3231_REG_SHIFT_CURRENT_DAY_PLACE_ONES = 0,
};
enum DS3231_REG_MASK_CURRENT_DATE
{
    DS3231_REG_MASK_CURRENT_DATE_PLACE_TENS = 0b11001111,
    DS3231_REG_MASK_CURRENT_DATE_PLACE_ONES = 0b11110000,
};
enum DS3231_REG_SHIFT_CURRENT_DATE
{
    DS3231_REG_SHIFT_CURRENT_DATE_PLACE_TENS = 4,
    DS3231_REG_SHIFT_CURRENT_DATE_PLACE_ONES = 0,
};
enum DS3231_REG_MASK_CURRENT_MONTH_CENTURY
{
    DS3231_REG_MASK_CURRENT_CENTURY = 0b01111111,
    DS3231_REG_MASK_CURRENT_MONTH_PLACE_TENS = 0b11101111,
    DS3231_REG_MASK_CURRENT_MONTH_PLACE_ONES = 0b11110000,
};
enum DS3231_REG_SHIFT_CURRENT_MONTH_CENTURY
{
    DS3231_REG_SHIFT_CURRENT_CENTURY = 7,
    DS3231_REG_SHIFT_CURRENT_MONTH_PLACE_TENS = 4,
    DS3231_REG_SHIFT_CURRENT_MONTH_PLACE_ONES = 0,
};
enum DS3231_REG_MASK_CURRENT_YEAR
{
    DS3231_REG_MASK_CURRENT_YEAR_PLACE_TENS = 0b00001111,
    DS3231_REG_MASK_CURRENT_YEAR_PLACE_ONES = 0b11110000,
};
enum DS3231_REG_SHIFT_CURRENT_YEAR
{
    DS3231_REG_SHIFT_CURRENT_YEAR_PLACE_TENS = 4,
    DS3231_REG_SHIFT_CURRENT_YEAR_PLACE_ONES = 0,
};

// Alarm 1 / 2
/*
    TODO
*/

// Control
enum DS3231_REG_MASK_CONTROL
{
    DS3231_REG_MASK_CONTROL_ENABLE_OSCILLATOR = 0b01111111,
    DS3231_REG_MASK_CONTROL_BATTERY_BACKED_SQUARE_WAVE_OUTPUT = 0b10111111,
    DS3231_REG_MASK_CONTROL_CONVERT_TEMPERATURE = 0b11011111,
    DS3231_REG_MASK_CONTROL_RATE_SELECT_2 = 0b11101111,
    DS3231_REG_MASK_CONTROL_RATE_SELECT_1 = 0b11110111,
    DS3231_REG_MASK_CONTROL_RATE_SELECT = 0b11100111,
    DS3231_REG_MASK_CONTROL_INTERRUPT_CONTROL = 0b11111011,
    DS3231_REG_MASK_CONTROL_INTERRUPT_ALARM_ENABLE_2 = 0b11111101,
    DS3231_REG_MASK_CONTROL_INTERRUPT_ALARM_ENABLE_1 = 0b11111110,
};
enum DS3231_REG_SHIFT_CONTROL
{
    DS3231_REG_SHIFT_CONTROL_ENABLE_OSCILLATOR = 7,
    DS3231_REG_SHIFT_CONTROL_BATTERY_BACKED_SQUARE_WAVE_OUTPUT = 6,
    DS3231_REG_SHIFT_CONTROL_CONVERT_TEMPERATURE = 5,
    DS3231_REG_SHIFT_CONTROL_RATE_SELECT_2 = 4,
    DS3231_REG_SHIFT_CONTROL_RATE_SELECT_1 = 3,
    DS3231_REG_SHIFT_CONTROL_RATE_SELECT = DS3231_REG_SHIFT_CONTROL_RATE_SELECT_1,
    DS3231_REG_SHIFT_CONTROL_INTERRUPT_CONTROL = 2,
    DS3231_REG_SHIFT_CONTROL_INTERRUPT_ALARM_ENABLE_2 = 1,
    DS3231_REG_SHIFT_CONTROL_INTERRUPT_ALARM_ENABLE_1 = 0,
};

enum DS3231_SQUARE_WAVE_FREQUENCY
{                                              // RS2 RS1
    DS3231_SQUARE_WAVE_FREQUENCY_1HZ = 0,      //  0   0
    DS3231_SQUARE_WAVE_FREQUENCY_1_024KHZ = 1, //  0   1
    DS3231_SQUARE_WAVE_FREQUENCY_4_096KHZ = 2, //  1   0
    DS3231_SQUARE_WAVE_FREQUENCY_8_192KHZ = 3, //  1   1
    DS3231_SQUARE_WAVE_FREQUENCY_MIN = DS3231_SQUARE_WAVE_FREQUENCY_1HZ,
    DS3231_SQUARE_WAVE_FREQUENCY_MAX = DS3231_SQUARE_WAVE_FREQUENCY_8_192KHZ,
    DS3231_SQUARE_WAVE_FREUQENCY_DEFAULT = DS3231_SQUARE_WAVE_FREQUENCY_1HZ,
};

// Status
enum DS3231_REG_MASK_STATUS
{
    DS3231_REG_MASK_STATUS_OSCILLATOR_STOP_FLAG = 0b01111111,
    DS3231_REG_MASK_STATUS_ENABLE_32KHZ_OUTPUT = 0b11110111,
    DS3231_REG_MASK_STATUS_BUSY = 0b11111011,
    DS3231_REG_MASK_STATUS_ALARM_FLAG_2 = 0b11111101,
    DS3231_REG_MASK_STATUS_ALARM_FLAG_1 = 0b11111110,
};
enum DS3231_REG_SHIFT_STATUS
{
    DS3231_REG_SHIFT_STATUS_OSCILLATOR_STOP_FLAG = 7,
    DS3231_REG_SHIFT_STATUS_ENABLE_32KHZ_OUTPUT = 3,
    DS3231_REG_SHIFT_STATUS_BUSY = 2,
    DS3231_REG_SHIFT_STATUS_ALARM_FLAG_2 = 1,
    DS3231_REG_SHIFT_STATUS_ALARM_FLAG_1 = 0,
};

// Temperature
#define DS3231_TEMPERATURE_FRACTION_MULTIPLIER 0.25
enum DS3231_REG_MASK_TEMPERATURE
{
    DS3231_REG_MASK_TEMPERATURE_WHOLE = 0b00000000,
    DS3231_REG_MASK_TEMPERATURE_FRACTION = 0b00111111,
};
enum DS3231_REG_SHIFT_TEMPERATURE
{
    DS3231_REG_SHIFT_TEMPERATURE_WHOLE = 0,
    DS3231_REG_SHIFT_TEMPERATURE_FRACTION = 6,
};
#define DS3231_TEMPERATURE_RESOLUTION 0.25

// Time Constants
#define DS3231_I2C_WAIT_TIME_MS 1000
#define DS3231_I2C_WAIT_TIME_TICKS pdMS_TO_TICKS(DS3231_I2C_WAIT_TIME_MS)

// 'Object' variables
typedef struct DS3231_Control
{
    bool enableOscillator;
    bool batteryBackedSquareWaveEnable;
    bool convertTemperature;                  // NOT USED: write only 
	uint8_t rateSelect;
    bool interruptControl;
    bool alarm1InterruptEnable;
    bool alarm2InterruptEnable;
} DS3231_Control_t;
typedef struct DS3231_Status
{
    bool oscillatorStopFlag;
    bool enable32kHzOutput;
    bool busy;                               // NOT USED: read only
	bool alarmFlag2;
    bool alarmFlag1;
} DS3231_Status_t;
typedef struct DS3231_TimeCurrent
{
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint16_t year;
    // uint8_t century;
} DS3231_TimeCurrent_t;

typedef struct DS3231
{
	i2c_port_t port;

    DS3231_TimeCurrent_t currentTime;

    // Alarm 1
    // Alarm 2
    
	DS3231_Control_t control;
    DS3231_Status_t status;
    
	int8_t agingOffset;
    float temperature;
    bool clockIntegrityFlag;
} DS3231_t;

#ifdef DS3231_SINGLE_DEVICE
extern DS3231_t DS3231;
#endif

// Helper Functions
char const *DS3231_dayToName(uint8_t day);
uint8_t DS3231_getCompilerTime_Month(char const *string);
DS3231_TimeCurrent_t DS3231_getCompilerTime();
void DS3231_formatCurrentTimeString(DS3231_TimeCurrent_t time, char *buffer, int bufferSize);

// Register Functions (Should be public???)
bool DS3231_readRegister(DS3231_t *ds3231, uint8_t addr, uint8_t *val);
bool DS3231_writeRegister(DS3231_t *ds3231, uint8_t addr, uint8_t val);

// Current time
bool DS3231_readCurrentSecond(DS3231_t *ds3231);
bool DS3231_writeCurrentSecond(DS3231_t *ds3231, uint8_t second);
bool DS3231_readCurrentMinute(DS3231_t *ds3231);
bool DS3231_writeCurrentMinute(DS3231_t *ds3231, uint8_t minute);
bool DS3231_readCurrentHour(DS3231_t *ds3231);
bool DS3231_writeCurrentHour(DS3231_t *ds3231, uint8_t hour);
bool DS3231_readCurrentDay(DS3231_t *ds3231);
bool DS3231_writeCurrentDay(DS3231_t *ds3231, uint8_t day);
bool DS3231_readCurrentDate(DS3231_t *ds3231);
bool DS3231_writeCurrentDate(DS3231_t *ds3231, uint8_t date);
bool DS3231_readCurrentMonthAndCentury(DS3231_t *ds3231);
bool DS3231_writeCurrentMonth(DS3231_t *ds3231, uint8_t month);
bool DS3231_readCurrentYear(DS3231_t *ds3231);
bool DS3231_writeCurrentYear(DS3231_t *ds3231, uint16_t year);
bool DS3231_readCurrentTime(DS3231_t *ds3231);
bool DS3231_writeCurrentTime(DS3231_t *ds3231, DS3231_TimeCurrent_t time);
bool DS3231_writeCurrentTime_Compiler(DS3231_t *ds3231);
DS3231_TimeCurrent_t DS3231_getCurrentTime(DS3231_t *ds3231);
void DS3231_printCurrentTime(DS3231_t *ds3231);

bool DS3231_readTime(DS3231_t *ds3231, struct tm *timeStruct);
bool DS3231_writeTime(DS3231_t *ds3231, struct tm *timeStruct);
bool DS3231_checkBattery(DS3231_t *ds3231);
bool DS3231_setTimeFromSystem(DS3231_t *ds3231);

// Alarm 1

// Alarm 2

// Control
bool DS3231_readControl(DS3231_t *ds3231);
bool DS3231_writeControl(DS3231_t *ds3231);
bool DS3231_getOscillatorEnable(DS3231_t *ds3231);
bool DS3231_setOscillatorEnable(DS3231_t *ds3231, bool enable);
bool DS3231_getBatteryBackedSquareWaveEnable(DS3231_t *ds3231);
bool DS3231_setBatteryBackedSquareWaveEnable(DS3231_t *ds3231, bool enable);
bool DS3231_convertTemperature(DS3231_t *ds3231);
uint8_t DS3231_getRateSelect(DS3231_t *ds3231);
bool DS3231_setRateSelect(DS3231_t *ds3231, uint8_t rateSelect);
bool DS3231_getInterruptControl(DS3231_t *ds3231);
bool DS3231_setInterruptControl(DS3231_t *ds3231, bool enable);
bool DS3231_getAlarm1InterruptEnable(DS3231_t *ds3231);
bool DS3231_setAlarm1InterruptEnable(DS3231_t *ds3231, bool enable);
bool DS3231_getAlarm2InterruptEnable(DS3231_t *ds3231);
bool DS3231_setAlarm2InterruptEnable(DS3231_t *ds3231, bool enable);

// Status
bool DS3231_readStatus(DS3231_t *ds3231);
bool DS3231_writeStatus(DS3231_t *ds3231);
bool DS3231_getOscillatorStopFlag(DS3231_t *ds3231);
bool DS3231_clearOscillatorStopFlag(DS3231_t *ds3231);
bool DS3231_getEnable32kHzOutput(DS3231_t *ds3231);
bool DS3231_setEnable32kHzOutput(DS3231_t *ds3231, bool enable);
uint8_t DS3231_getBusy(DS3231_t *ds3231);
bool DS3231_waitUntilStatusReady(DS3231_t *ds3231);
bool DS3231_getAlarmFlag2(DS3231_t *ds3231);
bool DS3231_clearAlarmFlag2(DS3231_t *ds3231);
bool DS3231_getAlarmFlag1(DS3231_t *ds3231);
bool DS3231_clearAlarmFlag1(DS3231_t *ds3231);

// Aging Offset
bool DS3231_readAgingOffset(DS3231_t *ds3231);
bool DS3231_writeAgingOffset(DS3231_t *ds3231);
int8_t DS3231_getAgingOffset(DS3231_t *ds3231);
bool DS3231_setAgingOffset(DS3231_t *ds3231, int8_t offset);

// Temperature
bool DS3231_readTemperature(DS3231_t *ds3231);
void DS3231_printTemperature(DS3231_t *ds3231);

bool DS3231_readAllRegisters(DS3231_t *ds3231);

bool DS3231_detect(DS3231_t *ds3231);

i2c_port_t DS3231_getPort(DS3231_t *ds3231);
void DS3231_setPort(DS3231_t *ds3231, i2c_port_t port);

bool DS3231_deinit(DS3231_t *ds3231);
bool DS3231_init(DS3231_t *ds3231, i2c_port_t port);