#include "DS3231.h"

static char const *TAG = "DS3231";

#ifdef DS3231_SINGLE_DEVICE
DS3231_t DS3231 =
{

};
#endif

// Helper Functions
char const *DS3231_dayToName(uint8_t day)
{
    // Can be user-defined. Following is the standard set out in PCF8563
    switch (day)
    {
        case 0:
            return "Sunday";
        case 1:
            return "Monday";
        case 2:
            return "Tuesday";
        case 3:
            return "Wednesday";
        case 4:
            return "Thursday";
        case 5:
            return "Friday";
        case 6:
            return "Saturday";
        default:
            return "INVALID";
    }
}
uint8_t DS3231_getCompilerTime_Month(char const *string)
{
    if (strcmp(string, "Jan") == 0)
        return 1;
    else if (strcmp(string, "Feb") == 0)
        return 2;
    else if (strcmp(string, "Mar") == 0)
        return 3;
    else if (strcmp(string, "Apr") == 0)
        return 4;
    else if (strcmp(string, "May") == 0)
        return 5;
    else if (strcmp(string, "Jun") == 0)
        return 6;
    else if (strcmp(string, "Jul") == 0)
        return 7;
    else if (strcmp(string, "Aug") == 0)
        return 8;
    else if (strcmp(string, "Sep") == 0)
        return 9;
    else if (strcmp(string, "Oct") == 0)
        return 10;
    else if (strcmp(string, "Nov") == 0)
        return 11;
    else if (strcmp(string, "Dev") == 0)
        return 12;
    else
    {
        ESP_LOGE(TAG, "Unknown month from compiler string: '%s'", string);

        return 0;
    }
}
DS3231_TimeCurrent_t DS3231_getCompilerTime()
{
    // __DATE__:        Nov 29 2023
    // __TIME__:        03:40:48
    DS3231_TimeCurrent_t t = { 0 };

    uint8_t second = 0;
    uint8_t minute = 0;
    uint8_t hour = 0;
    uint8_t date = 0;
    uint8_t month = 0;
    uint16_t year = 0;
    char monthString[4] = { 0 };

    // Can we directly pass date / year from struct?
    sscanf(__DATE__, "%3s %hhu %hu", monthString, &date, &year);
    sscanf(__TIME__, "%hhu:%hhu:%hhu", &hour, &minute, &second);

    month = DS3231_getCompilerTime_Month(monthString);
    
    t.second = second;
    t.minute = minute;
    t.hour = hour;
    t.date = date;
    t.month = month;
    t.year = year;
    
    return t;
}
void DS3231_formatCurrentTimeString(DS3231_TimeCurrent_t time, char *buffer, int bufferSize)
{
	snprintf(buffer, bufferSize, "%02d/%02d/%04d  %02d:%02d:%02d", time.date, time.month, time.year, time.hour, time.minute, time.second);
}


// Register Functions
bool DS3231_readRegister(DS3231_t *ds3231, uint8_t addr, uint8_t *val)
{
    esp_err_t ret = ESP_OK;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DS3231_ADDR_I2C << 1) | I2C_MASTER_WRITE, I2C_MASTER_NACK);
    i2c_master_write_byte(cmd, addr, I2C_MASTER_NACK);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DS3231_ADDR_I2C << 1) | I2C_MASTER_READ, I2C_MASTER_NACK);
    i2c_master_read_byte(cmd, val, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
	
    ret = i2c_master_cmd_begin(ds3231->port, cmd, DS3231_I2C_WAIT_TIME_TICKS);

    i2c_cmd_link_delete(cmd);
	
    if (ret == ESP_OK)
    {
        return true;
    }
    else
    {
        ESP_LOGE(TAG, "Failed to read from I2C register: %s", esp_err_to_name(ret));

        return false;
    }
}
bool DS3231_writeRegister(DS3231_t *ds3231, uint8_t addr, uint8_t val)
{
    //printf("TODO: DS3231 check busy flag\n");
    
    esp_err_t ret = ESP_OK;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DS3231_ADDR_I2C << 1) | I2C_MASTER_WRITE, I2C_MASTER_NACK);
    i2c_master_write_byte(cmd, addr, I2C_MASTER_NACK);
    i2c_master_write_byte(cmd, val, I2C_MASTER_NACK);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(ds3231->port, cmd, DS3231_I2C_WAIT_TIME_TICKS);

    i2c_cmd_link_delete(cmd);

    if (ret == ESP_OK)
    {
        return true;
    }
    else
    {
        ESP_LOGE(TAG, "Failed to write to I2C register: %s", esp_err_to_name(ret));

        return false;
    }
}

// Current Time
bool DS3231_readCurrentSecond(DS3231_t *ds3231)
{
    uint8_t val = 0, tens = 0, ones = 0;

    if (!DS3231_readRegister(ds3231, DS3231_ADDR_REG_CURRENT_SECOND, &val))
        return false;
    
    tens = ((val & ~DS3231_REG_MASK_CURRENT_SECOND_PLACE_TENS) >> DS3231_REG_SHIFT_CURRENT_SECOND_PLACE_TENS);
    ones = ((val & ~DS3231_REG_MASK_CURRENT_SECOND_PLACE_ONES) >> DS3231_REG_SHIFT_CURRENT_SECOND_PLACE_ONES);

    ds3231->currentTime.second = (10 * tens) + ones;

    return true;
}
bool DS3231_writeCurrentSecond(DS3231_t *ds3231, uint8_t second)
{
    if (second > 60)
    {
        ESP_LOGE(TAG, "Invalid second (0-59)");

        return false;
    }
    else if (second == 60)
        second = 0;

    uint8_t tens = second / 10;
    uint8_t ones = second - (10 * tens);

    uint8_t val = (tens << DS3231_REG_SHIFT_CURRENT_SECOND_PLACE_TENS) |
        (ones << DS3231_REG_SHIFT_CURRENT_SECOND_PLACE_ONES);
    
    if (!DS3231_writeRegister(ds3231, DS3231_ADDR_REG_CURRENT_SECOND, val))
    {
        ESP_LOGE(TAG, "Failed to set current time (second)");

        return false;
    }

    return true;
}
bool DS3231_readCurrentMinute(DS3231_t *ds3231)
{
    uint val = 0, tens = 0, ones = 0;

    if (!DS3231_readRegister(ds3231, DS3231_ADDR_REG_CURRENT_MINUTE, &val))
        return false;
    
    tens = ((val & ~DS3231_REG_MASK_CURRENT_MINUTE_PLACE_TENS) >> DS3231_REG_SHIFT_CURRENT_MINUTE_PLACE_TENS);
    ones = ((val & ~DS3231_REG_MASK_CURRENT_MINUTE_PLACE_ONES) >> DS3231_REG_SHIFT_CURRENT_MINUTE_PLACE_ONES);

    ds3231->currentTime.minute = (10 * tens) + ones;

    return true;
}
bool DS3231_writeCurrentMinute(DS3231_t *ds3231, uint8_t minute)
{
    if (minute > 60)
    {
        ESP_LOGE(TAG, "Invalid minute (0-59)");

        return false;
    }
    else if (minute == 60)
        minute = 0;

    uint8_t tens = minute / 10;
    uint8_t ones = minute - (10 * tens);

    uint8_t val = (tens << DS3231_REG_SHIFT_CURRENT_MINUTE_PLACE_TENS) |
        (ones << DS3231_REG_SHIFT_CURRENT_MINUTE_PLACE_ONES);
    
    if (!DS3231_writeRegister(ds3231, DS3231_ADDR_REG_CURRENT_MINUTE, val))
    {
        ESP_LOGE(TAG, "Failed to set current time (minute)");

        return false;
    }

    return true;
}
bool DS3231_readCurrentHour(DS3231_t *ds3231)
{
    uint8_t val = 0, mode_12_24_hour = 0, am_pm_twenties = 0, tens = 0, ones = 0;

    if (!DS3231_readRegister(ds3231, DS3231_ADDR_REG_CURRENT_HOUR, &val))
        return false;
    
    // Always use 24-hour mode for this library
    // When (mode_12_24_hour == 1), RTC is running in 12-hour mode: (am_pm_twenties == 1) means PM (hour + 12)
    mode_12_24_hour = ((val & ~DS3231_REG_MASK_CURRENT_HOUR_12_24_HOUR) >> DS3231_REG_SHIFT_CURRENT_HOUR_12_24_HOUR);
    am_pm_twenties = ((val & ~DS3231_REG_MASK_CURRENT_HOUR_AM_PM_PLACE_TWENTIES) >> DS3231_REG_SHIFT_CURRENT_HOUR_AM_PM_PLACE_TWENTIES);
    tens = ((val & ~DS3231_REG_MASK_CURRENT_HOUR_PLACE_TENS) >> DS3231_REG_SHIFT_CURRENT_HOUR_PLACE_TENS);
    ones = ((val & ~DS3231_REG_MASK_CURRENT_HOUR_PLACE_ONES) >> DS3231_REG_SHIFT_CURRENT_HOUR_PLACE_ONES);
    
    uint8_t h = 0;

    if (mode_12_24_hour)
    {
        // 12-Hour mode
        h = (10 * tens) + ones; // + 1???
    }
    else
    {
        // 24-Hour mode
        h = (20 * am_pm_twenties) + (10 * tens) + ones;
    }


    /*
    uint8_t h = (10 * tens) + ones;

    if (mode_12_24_hour)    // 12-Hour mode
        h += 12;
    }
    else                    // 24-Hour mode
        h += (20 * am_pm_twenties);
    */

    ds3231->currentTime.hour = h;
    
    return true;
}
bool DS3231_writeCurrentHour(DS3231_t *ds3231, uint8_t hour)
{
    // Always use 24-hour mode for this library
    if (hour > 24)
    {
        ESP_LOGE(TAG, "Invalid hour (0-23)");

        return false;
    }
    else if (hour == 24)
        hour = 0;


    uint8_t twenties = hour / 20;
    uint8_t tens = hour / 10;
    uint8_t ones = 0;

    if (hour >= 20)
        ones = hour - (20 * twenties);
    else
        ones = hour - (10 * tens);
    
    // Assume bit 6 (12/24-hour mode) always LOW (24-hour mode)
    uint8_t val = (twenties << DS3231_REG_SHIFT_CURRENT_HOUR_AM_PM_PLACE_TWENTIES) |
        (tens << DS3231_REG_SHIFT_CURRENT_HOUR_PLACE_TENS) |
        (ones << DS3231_REG_SHIFT_CURRENT_HOUR_PLACE_ONES);
    
    if (!DS3231_writeRegister(ds3231, DS3231_ADDR_REG_CURRENT_HOUR, val))
    {
        ESP_LOGE(TAG, "Failed to set current time (hour)");

        return false;
    }

    return true;
}
bool DS3231_readCurrentDay(DS3231_t *ds3231)
{
    uint8_t val = 0, ones = 0;

    if (!DS3231_readRegister(ds3231, DS3231_ADDR_REG_CURRENT_DAY, &val))
        return false;
    
    ones = ((val & ~DS3231_REG_MASK_CURRENT_DAY_PLACE_ONES) >> DS3231_REG_SHIFT_CURRENT_DAY_PLACE_ONES);

    ds3231->currentTime.day = ones;

    return true;
}
bool DS3231_writeCurrentDay(DS3231_t *ds3231, uint8_t day)
{
    if (day < 1 || day > 7)
    {
        ESP_LOGE(TAG, "Invalid day (1-7)");

        return false;
    }

    uint8_t ones = day;

    uint8_t val = (ones << DS3231_REG_SHIFT_CURRENT_DAY_PLACE_ONES);

    if (!DS3231_writeRegister(ds3231, DS3231_ADDR_REG_CURRENT_DAY, val))
    {
        ESP_LOGE(TAG, "Failed to set current time (day)");

        return false;
    }

    return true;
}
bool DS3231_readCurrentDate(DS3231_t *ds3231)
{
    uint8_t val = 0, tens = 0, ones = 0;

    if (!DS3231_readRegister(ds3231, DS3231_ADDR_REG_CURRENT_DATE, &val))
        return false;

    tens = ((val & ~DS3231_REG_MASK_CURRENT_DATE_PLACE_TENS) >> DS3231_REG_SHIFT_CURRENT_DATE_PLACE_TENS);
    ones = ((val & ~DS3231_REG_MASK_CURRENT_DATE_PLACE_ONES) >> DS3231_REG_SHIFT_CURRENT_DATE_PLACE_ONES);

    ds3231->currentTime.date = (10 * tens) + ones;

    return true;
}
bool DS3231_writeCurrentDate(DS3231_t *ds3231, uint8_t date)
{
    if (date > 31)
    {
        ESP_LOGE(TAG, "Invalid month");

        return false;
    }

    uint8_t tens = date / 10;
    uint8_t ones = date - (10 * tens);

    uint8_t val = (tens << DS3231_REG_SHIFT_CURRENT_DATE_PLACE_TENS) |
        (ones << DS3231_REG_SHIFT_CURRENT_DATE_PLACE_ONES);

    if (!DS3231_writeRegister(ds3231, DS3231_ADDR_REG_CURRENT_DATE, val))
    {
        ESP_LOGE(TAG, "Failed to set current time (date)");

        return false;
    }

    return true;
}
bool DS3231_readCurrentMonthAndCentury(DS3231_t *ds3231)
{
    uint8_t val = 0, tens = 0, ones = 0;

    if (!DS3231_readRegister(ds3231, DS3231_ADDR_REG_CURRENT_MONTH_CENTURY, &val))
        return false;
    
    // Ready century and figure out what to do with it later...
    uint8_t century = ((val & ~DS3231_REG_MASK_CURRENT_CENTURY) >> DS3231_REG_SHIFT_CURRENT_CENTURY);

    tens = ((val & ~DS3231_REG_MASK_CURRENT_MONTH_PLACE_TENS) >> DS3231_REG_SHIFT_CURRENT_MONTH_PLACE_TENS);
    ones = ((val & ~DS3231_REG_MASK_CURRENT_MONTH_PLACE_ONES) >> DS3231_REG_SHIFT_CURRENT_MONTH_PLACE_ONES);

    ds3231->currentTime.month = (10 * tens) + ones;

    return true;
}
bool DS3231_writeCurrentMonth(DS3231_t *ds3231, uint8_t month)
{
    if (month == 0 || month > 12)
    {
        ESP_LOGE(TAG, "Invalid month (1-12)");

        return false;
    }

    uint8_t tens = month / 10;
    uint8_t ones = month - (10 * tens);

    // Ignore century bit for now

    uint8_t val = (tens << DS3231_REG_SHIFT_CURRENT_MONTH_PLACE_TENS) |
        (ones << DS3231_REG_SHIFT_CURRENT_MONTH_PLACE_ONES);
    
    if (!DS3231_writeRegister(ds3231, DS3231_ADDR_REG_CURRENT_MONTH_CENTURY, val))
    {
        ESP_LOGE(TAG, "Failed to set current time (month)");

        return false;
    }

    return true;
}
bool DS3231_readCurrentYear(DS3231_t *ds3231)
{
    uint8_t val = 0, tens = 0, ones = 0;

    if (!DS3231_readRegister(ds3231, DS3231_ADDR_REG_CURRENT_YEAR, &val))
        return false;
    
    tens = ((val & ~DS3231_REG_MASK_CURRENT_YEAR_PLACE_TENS) >> DS3231_REG_SHIFT_CURRENT_YEAR_PLACE_TENS);
    ones = ((val & ~DS3231_REG_MASK_CURRENT_YEAR_PLACE_ONES) >> DS3231_REG_SHIFT_CURRENT_YEAR_PLACE_ONES);

    // For now, ignore century bit and assume year is in millenium 2000
    ds3231->currentTime.year = 2000 + (10 * tens) + ones;

    return true;
}
bool DS3231_writeCurrentYear(DS3231_t *ds3231, uint16_t year)
{
    // For now, ignore century bit and assume year is in millenium 2000
    if (year < 2000 || year >= 3000)
    {
        ESP_LOGE(TAG, "Invalid year (millenium 2000 only)");

        return false;
    }
    
    year -= 2000;

    uint8_t tens = year / 10;
    uint8_t ones = year - (10 * tens);

    uint8_t val = (tens << DS3231_REG_SHIFT_CURRENT_YEAR_PLACE_TENS) |
        (ones << DS3231_REG_SHIFT_CURRENT_YEAR_PLACE_ONES);
    
    if (!DS3231_writeRegister(ds3231, DS3231_ADDR_REG_CURRENT_YEAR, val))
    {
        ESP_LOGE(TAG, "Failed to set current time (year)");

        return false;
    }

    return true;
}
bool DS3231_readCurrentTime(DS3231_t *ds3231)
{
    if (!DS3231_readCurrentSecond(ds3231))
        return false;
    
    if (!DS3231_readCurrentMinute(ds3231))
        return false;
    
    if (!DS3231_readCurrentHour(ds3231))
        return false;
    
    if (!DS3231_readCurrentDay(ds3231))
        return false;
    
    if (!DS3231_readCurrentDate(ds3231))
        return false;
    
    if (!DS3231_readCurrentMonthAndCentury(ds3231))
        return false;
    
    if (!DS3231_readCurrentYear(ds3231))
        return false;

    return true;
}
bool DS3231_writeCurrentTime(DS3231_t *ds3231, DS3231_TimeCurrent_t time)
{
    if (!DS3231_writeCurrentSecond(ds3231, time.second))
        return false;
    
    if (!DS3231_writeCurrentMinute(ds3231, time.minute))
        return false;
    
    if (!DS3231_writeCurrentHour(ds3231, time.hour))
        return false;
    
    if (!DS3231_writeCurrentDay(ds3231, time.day))
        return false;
    
    if (!DS3231_writeCurrentDate(ds3231, time.date))
        return false;
    
    if (!DS3231_writeCurrentMonth(ds3231, time.month))
        return false;
    
    if (!DS3231_writeCurrentYear(ds3231, time.year))
        return false;
    
    ds3231->clockIntegrityFlag = true;

    return true;
}
bool DS3231_writeCurrentTime_Compiler(DS3231_t *ds3231)
{
    return DS3231_writeCurrentTime(ds3231, DS3231_getCompilerTime());
}
void DS3231_printCurrentTime(DS3231_t *ds3231)
{
    if (DS3231_readCurrentTime(ds3231))
    {
        ESP_LOGI(TAG, "DS3231 Current Time: %02d:%02d:%02d  (%s) %02d/%02d/%04d",
            ds3231->currentTime.hour, ds3231->currentTime.minute, ds3231->currentTime.second,
            DS3231_dayToName(ds3231->currentTime.day),
            ds3231->currentTime.date, ds3231->currentTime.month, ds3231->currentTime.year);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to read DS3231 Current Time");
    }
}
DS3231_TimeCurrent_t DS3231_getCurrentTime(DS3231_t *ds3231)
{
	return ds3231->currentTime;
}
bool DS3231_readTime(DS3231_t *ds3231, struct tm *timeStruct)
{
    if (DS3231_readCurrentTime(ds3231))
    {
        timeStruct->tm_sec = ds3231->currentTime.second;
        timeStruct->tm_min = ds3231->currentTime.minute;
        timeStruct->tm_hour = ds3231->currentTime.hour;
        timeStruct->tm_mday = ds3231->currentTime.date;
        timeStruct->tm_mon = ds3231->currentTime.month - 1;
        timeStruct->tm_year = ds3231->currentTime.year + 100;
        timeStruct->tm_wday = ds3231->currentTime.day - 1;
        timeStruct->tm_yday = -1;
        timeStruct->tm_isdst = -1;
        
        /*
        ESP_LOGI(TAG, "Read time from DS3231: %d-%d-%d %d:%d:%d",
            timeStruct->tm_year + 1900, timeStruct->tm_mon + 1, timeStruct->tm_mday,
            timeStruct->tm_hour, timeStruct->tm_min, timeStruct->tm_sec);
        */
        
        return true;
    }
    else
    {
        return false;
    }
}
bool DS3231_writeTime(DS3231_t *ds3231, struct tm *timeStruct)
{
    // TODO: pick out members from timeStruct and create DS3231_TimeCurrent_t and call DS3231_writeCurrentTime()

    return false;
}
bool DS3231_checkBattery(DS3231_t *ds3231)
{
    return ds3231->clockIntegrityFlag;
}
bool DS3231_setTimeFromSystem(DS3231_t *ds3231)
{
    return false;
}

// Alarm 1
/*
    TODO
*/

// Alarm 2
/*
    TODO
*/

// Control
uint8_t DS3231_organiseRegisterControl(DS3231_t *ds3231)
{
    uint8_t ordered = 0;

    ordered |= ((uint8_t)ds3231->control.enableOscillator << DS3231_REG_SHIFT_CONTROL_ENABLE_OSCILLATOR);
    ordered |= ((uint8_t)ds3231->control.batteryBackedSquareWaveEnable << DS3231_REG_SHIFT_CONTROL_BATTERY_BACKED_SQUARE_WAVE_OUTPUT);
    ordered |= ((uint8_t)ds3231->control.convertTemperature << DS3231_REG_SHIFT_CONTROL_CONVERT_TEMPERATURE);
    ordered |= ((uint8_t)ds3231->control.rateSelect << DS3231_REG_SHIFT_CONTROL_RATE_SELECT);
    ordered |= ((uint8_t)ds3231->control.interruptControl << DS3231_REG_SHIFT_CONTROL_INTERRUPT_CONTROL);
    ordered |= ((uint8_t)ds3231->control.alarm1InterruptEnable << DS3231_REG_SHIFT_CONTROL_INTERRUPT_ALARM_ENABLE_1);
    ordered |= ((uint8_t)ds3231->control.alarm2InterruptEnable << DS3231_REG_SHIFT_CONTROL_INTERRUPT_ALARM_ENABLE_2);

    return ordered;
}
bool DS3231_readControl(DS3231_t *ds3231)
{
	uint8_t val = 0;

    if (!DS3231_readRegister(ds3231, DS3231_ADDR_REG_CONTROL, &val))
    {
        ESP_LOGE(TAG, "Failed to read Control register");

        return false;
    }

	if (((val & ~DS3231_REG_MASK_CONTROL_ENABLE_OSCILLATOR) >> DS3231_REG_SHIFT_CONTROL_ENABLE_OSCILLATOR) == 1)
		ds3231->control.enableOscillator = true;
	else
		ds3231->control.enableOscillator = false;

	if (((val & ~DS3231_REG_MASK_CONTROL_BATTERY_BACKED_SQUARE_WAVE_OUTPUT) >> DS3231_REG_SHIFT_CONTROL_BATTERY_BACKED_SQUARE_WAVE_OUTPUT) == 1)
		ds3231->control.batteryBackedSquareWaveEnable = true;
	else
		ds3231->control.batteryBackedSquareWaveEnable = false;
	
	if (((val & ~DS3231_REG_MASK_CONTROL_CONVERT_TEMPERATURE) >> DS3231_REG_SHIFT_CONTROL_CONVERT_TEMPERATURE) == 1)
		ds3231->control.convertTemperature = true;
	else
		ds3231->control.convertTemperature = false;
	
    ds3231->control.rateSelect = ((val & ~DS3231_REG_MASK_CONTROL_RATE_SELECT) >> DS3231_REG_SHIFT_CONTROL_RATE_SELECT);
    
	if (((val & ~DS3231_REG_MASK_CONTROL_INTERRUPT_CONTROL) >> DS3231_REG_SHIFT_CONTROL_INTERRUPT_CONTROL) == 1)
		ds3231->control.interruptControl = true;
	else
		ds3231->control.interruptControl = false;
	
	if (((val & ~DS3231_REG_MASK_CONTROL_INTERRUPT_ALARM_ENABLE_1) >> DS3231_REG_SHIFT_CONTROL_INTERRUPT_ALARM_ENABLE_1) == 1)
		ds3231->control.alarm1InterruptEnable = true;
	else
		ds3231->control.alarm1InterruptEnable = false;
	
	if (((val & ~DS3231_REG_MASK_CONTROL_INTERRUPT_ALARM_ENABLE_2) >> DS3231_REG_SHIFT_CONTROL_INTERRUPT_ALARM_ENABLE_2) == 1)
		ds3231->control.alarm2InterruptEnable = true;
	else
		ds3231->control.alarm2InterruptEnable = false;
	
    return true;
}
bool DS3231_writeControl(DS3231_t *ds3231)
{
    bool success = true;

    uint8_t organised = DS3231_organiseRegisterControl(ds3231);

    if (!DS3231_writeRegister(ds3231, DS3231_ADDR_REG_CONTROL, organised))
    {
        ESP_LOGE(TAG, "Failed to write to Control");

        success = false;
    }

    return success;
}
bool DS3231_getEnableOscillator(DS3231_t *ds3231)
{
    return ds3231->control.enableOscillator;
}
bool DS3231_setEnableOscillator(DS3231_t *ds3231, bool enable)
{
    if (enable == ds3231->control.enableOscillator)
        return true;
    
    bool success = true;

    bool enableOld = ds3231->control.enableOscillator;

    ds3231->control.enableOscillator = enable;

    if (!DS3231_writeControl(ds3231))
    {
        ESP_LOGE(TAG, "Failed to set Oscillator enable state");

        ds3231->control.enableOscillator = enableOld;

        success = false;
    }

    return success;
}
bool DS3231_getBatteryBackedSquareWaveEnable(DS3231_t *ds3231)
{
    return ds3231->control.batteryBackedSquareWaveEnable;
}
bool DS3231_setBatteryBackedSquareWaveEnable(DS3231_t *ds3231, bool enable)
{
    if (enable == ds3231->control.batteryBackedSquareWaveEnable)
        return true;

    bool enableOld = ds3231->control.batteryBackedSquareWaveEnable;

    ds3231->control.batteryBackedSquareWaveEnable = enable;

    if (!DS3231_writeControl(ds3231))
    {
        ESP_LOGE(TAG, "Failed to set Battery-Backed Square Wave enable state");

        ds3231->control.batteryBackedSquareWaveEnable = enableOld;

        return false;
    }

    return true;
}
bool DS3231_convertTemperature(DS3231_t *ds3231)
{
    if (!DS3231_waitUntilStatusReady(ds3231))
    {
        return false;
    }

	ds3231->control.convertTemperature = true;

    if (!DS3231_writeControl(ds3231))
    {
        return false;
    }
    
    return true;
}
uint8_t DS3231_getRateSelect(DS3231_t *ds3231)
{
    return ds3231->control.rateSelect;
}
bool DS3231_setRateSelect(DS3231_t *ds3231, uint8_t rate)
{
    if (rate > DS3231_SQUARE_WAVE_FREQUENCY_8_192KHZ)
    {
        ESP_LOGE(TAG, "Invalid rate select (%d-%d)", DS3231_SQUARE_WAVE_FREQUENCY_MIN, DS3231_SQUARE_WAVE_FREQUENCY_MAX);
        
        return false;
    }

    if (rate == ds3231->control.rateSelect)
        return true;

    uint8_t rateOld = ds3231->control.rateSelect;

    ds3231->control.rateSelect = rate;

    if (!DS3231_writeControl(ds3231))
    {
        ESP_LOGE(TAG, "Failed to set Rate Select");

        ds3231->control.rateSelect = rateOld;

        return false;
    }

    return true;
}
bool DS3231_getInterruptControl(DS3231_t *ds3231)
{
    return ds3231->control.interruptControl;
}
bool DS3231_setInterruptControl(DS3231_t *ds3231, bool enable)
{
    if (enable == ds3231->control.interruptControl)
        return true;

    bool enableOld = ds3231->control.interruptControl;

    ds3231->control.interruptControl = enable;

    if (!DS3231_writeControl(ds3231))
    {
        ESP_LOGE(TAG, "Failed to set Interrupt Control enable state");

        ds3231->control.interruptControl = enableOld;

        return false;
    }

    return true;
}
bool DS3231_getAlarm1InterruptEnable(DS3231_t *ds3231)
{
    return ds3231->control.alarm1InterruptEnable;
}
bool DS3231_setAlarm1InterruptEnable(DS3231_t *ds3231, bool enable)
{
    if (enable == ds3231->control.alarm1InterruptEnable)
        return true;

    bool enableOld = ds3231->control.alarm1InterruptEnable;

    ds3231->control.alarm1InterruptEnable = enable;

    if (!DS3231_writeControl(ds3231))
    {
        ESP_LOGE(TAG, "Failed to set Alarm 1 Interrupt enable state");

        ds3231->control.alarm1InterruptEnable = enableOld;

        return false;
    }

    return true;
}
bool DS3231_getAlarm2InterruptEnable(DS3231_t *ds3231)
{
    return ds3231->control.alarm2InterruptEnable;
}
bool DS3231_setAlarm2InterruptEnable(DS3231_t *ds3231, bool enable)
{
    if (enable == ds3231->control.alarm2InterruptEnable)
        return true;

    bool enableOld = ds3231->control.alarm2InterruptEnable;

    ds3231->control.alarm2InterruptEnable = enable;

    if (!DS3231_writeControl(ds3231))
    {
        ESP_LOGE(TAG, "Failed to set Alarm 2 Interrupt enable state");

        ds3231->control.alarm2InterruptEnable = enableOld;

        return false;
    }

    return true;
}

// Status
uint8_t DS3231_organiseRegisterStatus(DS3231_t *ds3231)
{
    uint8_t ordered = 0;

    ordered |= ((uint8_t)ds3231->status.oscillatorStopFlag << DS3231_REG_SHIFT_STATUS_OSCILLATOR_STOP_FLAG);
    ordered |= ((uint8_t)ds3231->status.enable32kHzOutput << DS3231_REG_SHIFT_STATUS_ENABLE_32KHZ_OUTPUT);
    ordered |= ((uint8_t)ds3231->status.busy << DS3231_REG_SHIFT_STATUS_BUSY);
    ordered |= ((uint8_t)ds3231->status.alarmFlag2 << DS3231_REG_SHIFT_STATUS_ALARM_FLAG_2);
    ordered |= ((uint8_t)ds3231->status.alarmFlag1 << DS3231_REG_SHIFT_STATUS_ALARM_FLAG_1);

    return ordered;
}
bool DS3231_readStatus(DS3231_t *ds3231)
{
	uint8_t val = 0;

	if (!DS3231_readRegister(ds3231, DS3231_ADDR_REG_STATUS, &val))
    {
        ESP_LOGE(TAG, "Failed to read Status register");

        return false;
    }

	if (((val & ~DS3231_REG_MASK_STATUS_OSCILLATOR_STOP_FLAG) >> DS3231_REG_SHIFT_STATUS_OSCILLATOR_STOP_FLAG) == 1)
		ds3231->status.oscillatorStopFlag = true;
	else
		ds3231->status.oscillatorStopFlag = false;

	if (((val & ~DS3231_REG_MASK_STATUS_ENABLE_32KHZ_OUTPUT) >> DS3231_REG_SHIFT_STATUS_ENABLE_32KHZ_OUTPUT) == 1)
		ds3231->status.enable32kHzOutput = true;
	else
		ds3231->status.enable32kHzOutput = false;
	
	if (((val & ~DS3231_REG_MASK_STATUS_BUSY) >> DS3231_REG_SHIFT_STATUS_BUSY) == 1)
		ds3231->status.busy = true;
	else
		ds3231->status.busy = false;

	if (((val & ~DS3231_REG_MASK_STATUS_ALARM_FLAG_2) >> DS3231_REG_SHIFT_STATUS_ALARM_FLAG_2) == 1)
		ds3231->status.alarmFlag2 = true;
	else
		ds3231->status.alarmFlag2 = false;
	
	if (((val & ~DS3231_REG_MASK_STATUS_ALARM_FLAG_1) >> DS3231_REG_SHIFT_STATUS_ALARM_FLAG_1) == 1)
		ds3231->status.alarmFlag1 = true;
	else
		ds3231->status.alarmFlag1 = false;
    
	return true;
}
bool DS3231_writeStatus(DS3231_t *ds3231)
{
    bool success = true;

    uint8_t organised = DS3231_organiseRegisterStatus(ds3231);

    if (!DS3231_writeRegister(ds3231, DS3231_ADDR_REG_STATUS, organised))
    {
        ESP_LOGE(TAG, "Failed to write to Status");
        
        success = false;
    }

	return success;
}
bool DS3231_getOscillatorStopFlag(DS3231_t *ds3231)
{
    return ds3231->status.oscillatorStopFlag;
}
bool DS3231_clearOscillatorStopFlag(DS3231_t *ds3231)
{
    // Was going to re-read Status register to ensure latest values are read, however this could cause an inifite loop

    ds3231->status.oscillatorStopFlag = false;

    if (!DS3231_writeStatus(ds3231))
    {
        ESP_LOGE(TAG, "Failed to clear Oscillator Stop Flag");

        return false;
    }

    return true;
}
bool DS3231_getEnable32kHzOutput(DS3231_t *ds3231)
{
    return ds3231->status.enable32kHzOutput;
}
bool DS3231_setEnable32kHzOutput(DS3231_t *ds3231, bool enable)
{
    if (enable == ds3231->status.enable32kHzOutput)
        return true;

    bool enableOld = ds3231->status.enable32kHzOutput;

    ds3231->status.enable32kHzOutput = enable;

    if (!DS3231_writeStatus(ds3231))
    {
        ESP_LOGE(TAG, "Failed to set Enable 32 kHz enable state");

        ds3231->status.enable32kHzOutput = enableOld;

        return false;
    }

    return true;
}
uint8_t DS3231_getBusy(DS3231_t *ds3231)
{
    if (!DS3231_readStatus(ds3231))
        return 0xFF;
    else
        return (uint8_t)ds3231->status.busy;
}
bool DS3231_waitUntilStatusReady(DS3231_t *ds3231)
{
    TickType_t tStart = xTaskGetTickCount();

    for (;;)
    {
        uint8_t statusReady = DS3231_getBusy(ds3231);

        if (statusReady == 0xFF)
        {
            ESP_LOGE(TAG, "Failed to read status");

            return false;
        }
        else if (statusReady == 0)
        {
            break;
        }
        
        if ((xTaskGetTickCount() - tStart) > DS3231_TIMEOUT_TIME_TICKS)
        {
            ESP_LOGE(TAG, "Timeout - waiting for status ready");

            return false;
        }

        vTaskDelay(DS3231_TIME_DELAY_POLLING_TICKS);
    }

    return true;
}
bool DS3231_getAlarmFlag2(DS3231_t *ds3231)
{
    return ds3231->status.alarmFlag2;
}
bool DS3231_clearAlarmFlag2(DS3231_t *ds3231)
{
    // Was going to re-read Status register to ensure latest values are read, however this could cause an inifite loop

    ds3231->status.alarmFlag2 = false;

    if (!DS3231_writeStatus(ds3231))
    {
        ESP_LOGE(TAG, "Failed to clear Alarm Flag 2");

        return false;
    }

    return true;
}
bool DS3231_getAlarmFlag1(DS3231_t *ds3231)
{
    return ds3231->status.alarmFlag1;
}
bool DS3231_clearAlarmFlag1(DS3231_t *ds3231)
{
    // Was going to re-read Status register to ensure latest values are read, however this could cause an inifite loop

    ds3231->status.alarmFlag1 = false;

    if (!DS3231_writeStatus(ds3231))
    {
        ESP_LOGE(TAG, "Failed to clear Alarm Flag 1");

        return false;
    }

    return true;
}

// Aging Offset
bool DS3231_readAgingOffset(DS3231_t *ds3231)
{
    uint8_t val = 0;

    if (!DS3231_readRegister(ds3231, DS3231_ADDR_REG_AGING_OFFSET, &val))
    {
        ESP_LOGE(TAG, "Failed to read Aging Offset register");

        return false;
    }

    ds3231->agingOffset = (int8_t)val;

    return true;
}
bool DS3231_writeAgingOffset(DS3231_t *ds3231)
{
    bool success = true;

    uint8_t val = (uint8_t)ds3231->agingOffset;

    if (!DS3231_writeRegister(ds3231, DS3231_ADDR_REG_AGING_OFFSET, val))
    {
        ESP_LOGE(TAG, "Failed to write to Aging Offset");

        success = false;
    }

    return success;
}
int8_t DS3231_getAgingOffset(DS3231_t *ds3231)
{
    return ds3231->agingOffset;
}
bool DS3231_setAgingOffset(DS3231_t *ds3231, int8_t offset)
{
    if (offset == ds3231->agingOffset)
        return true;

    int8_t offsetOld = ds3231->agingOffset;

    ds3231->agingOffset = offset;

    uint8_t val = (uint8_t)offset;

    if (!DS3231_writeRegister(ds3231, DS3231_ADDR_REG_AGING_OFFSET, val))
    {
        ESP_LOGE(TAG, "Failed to set Aging Offset");

        ds3231->agingOffset = offsetOld;

        return false;
    }

    return true;
}

// Temperature
bool DS3231_readTemperature(DS3231_t *ds3231)
{
    if (!DS3231_waitUntilStatusReady(ds3231))
    {
        ESP_LOGE(TAG, "Failed to wait for status ready");

        return false;
    }

    uint8_t msb = 0, lsb = 0;

    if (!DS3231_readRegister(ds3231, DS3231_ADDR_REG_TEMPERATURE_MSB, &msb))
        return false;

    if (!DS3231_readRegister(ds3231, DS3231_ADDR_REG_TEMPERATURE_LSB, &lsb))
        return false;

    int8_t wholeInt = (int8_t)msb;
    float wholeFloat = (float)wholeInt;

    uint8_t fractionInt = ((lsb & ~DS3231_REG_MASK_TEMPERATURE_FRACTION) >> DS3231_REG_SHIFT_TEMPERATURE_FRACTION);
    float fractionFloat = (float)fractionInt * DS3231_TEMPERATURE_FRACTION_MULTIPLIER;

    float t = wholeFloat + fractionFloat;
    
    ds3231->temperature = t;

    return true;
}
void DS3231_printTemperature(DS3231_t *ds3231)
{
    if (DS3231_readTemperature(ds3231))
    {
        ESP_LOGI(TAG, "DS3231 Temperature: %.2f [C]", ds3231->temperature);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to read DS3231 Temperature\n");
    }
}

bool DS3231_readAllRegisters(DS3231_t *ds3231)
{
    bool success = true;

    if (!DS3231_readCurrentTime(ds3231))
        success = false;
    
    // Alarm 1/2

    if (!DS3231_readControl(ds3231))
        success = false;
    
    if (!DS3231_readStatus(ds3231))
        success = false;

    if (!DS3231_readAgingOffset(ds3231))
        success = false;
    
    if (!DS3231_readTemperature(ds3231))
        success = false;
    
    return success;
}

bool DS3231_detect(DS3231_t *ds3231)
{
	int const numRetries = 3;

    esp_err_t ret = ESP_OK;
    
	i2c_cmd_handle_t cmd;

	for (int i = 0; i < numRetries; i++)
	{
		cmd = i2c_cmd_link_create();
    
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (DS3231_ADDR_I2C << 1) | I2C_MASTER_WRITE, I2C_MASTER_NACK);
		i2c_master_stop(cmd);

		ret = i2c_master_cmd_begin(ds3231->port, cmd, DS3231_I2C_WAIT_TIME_TICKS);
		
		i2c_cmd_link_delete(cmd);

		if (ret == ESP_OK)
        	return true;
		else
			vTaskDelay(DS3231_TIME_DELAY_POLLING_TICKS);
	}
	
	ESP_LOGE(TAG, "Failed to detect: %s", esp_err_to_name(ret));

	return false;
}

i2c_port_t DS3231_getPort(DS3231_t *ds3231)
{
	return ds3231->port;
}
void DS3231_setPort(DS3231_t *ds3231, i2c_port_t port)
{
	if (port > I2C_NUM_MAX)
	{
		ESP_LOGE(TAG, "Invalid port");

		return;
	}
	
	ds3231->port = port;
}

bool DS3231_deinit(DS3231_t *ds3231)
{
    return true;
}
bool DS3231_init(DS3231_t *ds3231, i2c_port_t port)
{
    bool success = true;
	
	DS3231_setPort(ds3231, port);
	
    if (!DS3231_detect(ds3231))
    {
        ESP_LOGE(TAG, "Device not detected on I2C bus");

        success = false;
        // return false;
    }

    if (!DS3231_readAllRegisters(ds3231))
    {
        ESP_LOGE(TAG, "Failed to read one or more registers");

        success = false;
        // return false;
    }

    if (ds3231->status.oscillatorStopFlag)
    {
		ESP_LOGW(TAG, "Oscillator cutout detected - likely battery low voltage or not present");
		
        ds3231->clockIntegrityFlag = false;  // oscillatorStopFlag needs to be reset after every read, however clockIntegrityFlag is only reset once 

        if (!DS3231_clearOscillatorStopFlag(ds3231))
		{
			ESP_LOGE(TAG, "Failed to clear Oscillator Stop Flag");

			success = false;
		}
    }
    else
    {
        ESP_LOGI(TAG, "Clock integrity OK");

        ds3231->clockIntegrityFlag = true;
    }

    if (!DS3231_checkBattery(ds3231))
    {
        ESP_LOGW(TAG, "Battery low voltage or not present");
    }

    if (!DS3231_setEnable32kHzOutput(ds3231, false))
    {
        ESP_LOGE(TAG, "Failed to disable 32kHz output");

        success = false;
        // return false;
    }

    DS3231_printCurrentTime(ds3231);

    DS3231_printTemperature(ds3231);

    if (success)
        ESP_LOGI(TAG, "Successfully initialised");
    else
        ESP_LOGE(TAG, "Failed to initialise");

    return success;
}