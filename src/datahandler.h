/**
 * @brief: Datahandler administrates saved data; Writes login data and settings to emulated EEPROM / Reads Login data and settings from emulated EEPROM
 */

#include <ESP8266WiFi.h>
#pragma once

typedef struct LoginData
{
    char loginID[32];
    char password[32];
} LoginData;

typedef struct Settings
{
    uint32_t periodicRoutineIntervalMS;
    uint32_t timedRoutineIntervalMS;
    uint8_t deepSleepEnabled;
    uint32_t deepSleepDurationMS;
    uint32_t deepSleepIntervalMS;
    uint8_t logoutPeriodStartHour;
    uint8_t logoutPeriodStartMinute;
    uint8_t logoutPeriodEndHour;
    uint8_t logoutPeriodEndMinute;
    uint32_t loginRoutineInLogoutPeriodIntervalMS;
    char pingAddress[32];
    char ntpPool[32];
} Settings;

class DataHandler
{
  public:
    static DataHandler &instance()
    {
        static DataHandler _instance;
        return _instance;
    }

    LoginData &getLoginData();
    Settings &getSettings();

    void setLoginData(const LoginData &);
    void setSettings(const Settings &);
    bool isDataSaved();

    String log;

  private:
    DataHandler();

    LoginData currentLoginData;
    Settings currentSettings;
    bool dataSaved;
};