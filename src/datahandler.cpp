#include "datahandler.h"
#include "config.h"
#include <EEPROM.h>
#include "logger.h"

#define EEPROM_VALID_BYTE 12
#define EEPROM_ADDR_LOGIN_DATA 0x0
#define EEPROM_ADDR_SETTINGS 0x100

DataHandler::DataHandler()
{
    EEPROM.begin(512);
    uint8_t valid = EEPROM.read(EEPROM_ADDR_LOGIN_DATA);
    dataSaved = false;

    if (valid == EEPROM_VALID_BYTE)
    {
        EEPROM.get(EEPROM_ADDR_LOGIN_DATA + 1, currentLoginData);
        dataSaved = true;
    }
    else
    {
        /* Default settings */
        strncpy(currentLoginData.loginID, "", 32);
        strncpy(currentLoginData.password, "", 32);
    }

    valid = EEPROM.read(EEPROM_ADDR_SETTINGS);

    if (valid == EEPROM_VALID_BYTE)
    {
        EEPROM.get(EEPROM_ADDR_SETTINGS + 1, currentSettings);
        dataSaved = false;
    }
    else
    {
        /* Default settings */
        currentSettings.logoutPeriodStartHour = CONFIG_LOGOUT_PERIOD_START_HOUR;
        currentSettings.logoutPeriodStartMinute = CONFIG_LOGOUT_PERIOD_START_MINUTE;
        currentSettings.logoutPeriodEndHour = CONFIG_LOGOUT_PERIOD_END_HOUR;
        currentSettings.logoutPeriodEndMinute = CONFIG_LOGOUT_PERIOD_END_MINUTE;
        currentSettings.loginRoutineInLogoutPeriodIntervalMS = CONFIG_LOGIN_ROUTINE_IN_LOGOUT_PERIOD_INTERVAL_MS;
        currentSettings.periodicRoutineIntervalMS = CONFIG_PERIODIC_ROUTINE_INTERVAL_MS;
        currentSettings.deepSleepEnabled = CONFIG_DEEPSLEEP;
        currentSettings.deepSleepDurationMS = CONFIG_DEEPSLEEP_DURATION_MS;
        currentSettings.deepSleepIntervalMS = CONFIG_DEEPSLEEP_INTERVAL_MS;
        strncpy(currentSettings.ntpPool, CONFIG_NTP_POOL, 32);
        strncpy(currentSettings.pingAddress, CONFIG_PING_ADDRESS, 32);
    }

    EEPROM.end();
}

LoginData &DataHandler::getLoginData()
{
    return currentLoginData;
}

Settings &DataHandler::getSettings()
{
    return currentSettings;
}

void DataHandler::setLoginData(const LoginData &loginData)
{
    EEPROM.begin(512);
    EEPROM.write(EEPROM_ADDR_LOGIN_DATA, EEPROM_VALID_BYTE);
    EEPROM.put(EEPROM_ADDR_LOGIN_DATA + 1, loginData);
    EEPROM.commit();
    EEPROM.end();

    currentLoginData = loginData;
}

void DataHandler::setSettings(const Settings &settings)
{
    EEPROM.begin(512);
    EEPROM.write(EEPROM_ADDR_SETTINGS, EEPROM_VALID_BYTE);
    EEPROM.put(EEPROM_ADDR_SETTINGS + 1, settings);
    EEPROM.commit();
    EEPROM.end();

    currentSettings = settings;
}

bool DataHandler::isDataSaved()
{
    return dataSaved;
}