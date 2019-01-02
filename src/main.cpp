#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiClientSecure.h>
#include <NTPtimeESP.h>
#include <ESP8266Ping.h>
#include <Ticker.h>

#include "datahandler.h"
#include "config.h"
#include "site.h"
#include "callbacks.h"
#include "loginagent.h"
#include "logger.h"
#include "time.h"

/**
 * @brief Login if there is no internet connection present
 * @return true if internet connection is present (even if no login was performed), false if login failed
 */
bool loginRoutine(bool onlineCheck);

/**
 * @brief Function has to be called periodically
 * 
 * 1. Gets current time from the NTP server and checks if the next logout will be perfomed within a configured time span (logout period)
 * In that case the deep sleep timer (if deep sleep is enabled) gets disabled and the fast login routine timer starts. When a configured number of minutes since the logout have passed, the function stops the fast login routine timer and starts the deep sleep timer (if deep sleep is enabled)
 *
 * 2. Calls loginRoutine(false). 
 */
void timedRoutine();

WiFiClientSecure client;
WiFiManager wifiManager;
ESP8266WebServer server(80);

Site site = Site();
LoginAgent loginAgent(client);
DataHandler dataHandler = DataHandler::instance();
Settings &settings = dataHandler.getSettings();
LoginData &loginData = dataHandler.getLoginData();

NTPtime NTPch(settings.ntpPool);
strDateTime dateTime;

/**
 * @brief Timer calls timedRoutine() every settings.loginRoutineInLogoutPeriodIntervalMS milliseconds
 */
Ticker periodicRoutineTimer(timedRoutine, settings.periodicRoutineIntervalMS, 0, MILLIS);

/**
 * @brief Timer calls loginRoutine() every settings.loginRoutineInLogoutPeriodIntervalMS milliseconds
 */
Ticker loginRoutineInLogoutPeriodTimer([]() -> void { loginRoutine(true); }, settings.loginRoutineInLogoutPeriodIntervalMS, 0, MILLIS); // Lambda expression because callback has to be a 'void function'

/**
 * @brief Lets ESP (deep) sleep for settings.deepSleepDurationMS milliseconds
 */
void deepsleep()
{
  ESP.deepSleep(settings.deepSleepDurationMS * 1000);
}

/**
 * @brief Timer calls deepsleep() settings.deepSleepIntervalMS milliseconds
 */
Ticker deepSleepTimer(deepsleep, settings.deepSleepIntervalMS, 0, MILLIS);

void setup()
{
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY, 1);

  wifiManager.autoConnect(CONFIG_ACCESSPOINT_SSID);

  MDNS.begin(CONFIG_MDNS_HOSTNAME);
  MDNS.addService("http", "tcp", 80);

  initWebServerCallbacks(server);
  server.begin();

  NTPch.setSendInterval(10);

  if (settings.deepSleepEnabled && dataHandler.isDataSaved())
  {
    deepSleepTimer.start();
  }

  periodicRoutineTimer.start();

  timedRoutine();
}

void loop()
{
  server.handleClient();
  periodicRoutineTimer.update();
  loginRoutineInLogoutPeriodTimer.update();
  deepSleepTimer.update();
}

bool loginRoutine(bool onlineCheck)
{
  if(onlineCheck && Ping.ping(settings.pingAddress))
  {
    return true;
  }

  LoginData loginData = DataHandler::instance().getLoginData();
  return loginAgent.login(loginData) == LoginAgent::LOGIN_SUCCESSFUL;
}

void timedRoutine()
{
  // first parameter: Time zone; second parameter: 1 for European summer time; 2 for US daylight saving time; 0 for no DST adjustment;
  dateTime = NTPch.getNTPtime(1.0, 1);

  // check if dateTime is valid before using the returned time; otherwise send a new request; if after 3 requests dataTime is still not valid then login without checking for logout period
  for (int i = 0; i < 3 && !dateTime.valid; i++)
  {
    dateTime = NTPch.getNTPtime(1.0, 1);
    delay(100);
  }

  if (dateTime.valid)
  {
    // dateTime.epochTime is already adjusted! (summer time)
    time_t currentTimeEpoch = dateTime.epochTime;
   
    struct tm currentTime;
    gmtime_r(&currentTimeEpoch, &currentTime);

    struct tm startLogoutPeriodTime = currentTime;
    startLogoutPeriodTime.tm_hour = settings.logoutPeriodStartHour;
    startLogoutPeriodTime.tm_min = settings.logoutPeriodStartMinute;
    startLogoutPeriodTime.tm_sec = 0;

    struct tm endLogoutPeriodTime = currentTime;
    endLogoutPeriodTime.tm_hour = settings.logoutPeriodEndHour;
    endLogoutPeriodTime.tm_min = settings.logoutPeriodEndMinute;
    endLogoutPeriodTime.tm_sec = 0;

    time_t startLogoutPeriodTimeEpoch = mktime(&startLogoutPeriodTime);
    time_t endLogoutPeriodTimeEpoch = mktime(&endLogoutPeriodTime);
    
    if(loginRoutineInLogoutPeriodTimer.state() != RUNNING && currentTimeEpoch >= startLogoutPeriodTimeEpoch && currentTimeEpoch < endLogoutPeriodTimeEpoch)
    {
      loginRoutineInLogoutPeriodTimer.start();

      if (settings.deepSleepEnabled)
      {
        deepSleepTimer.stop();
      }
    }
    else if(loginRoutineInLogoutPeriodTimer.state() == RUNNING && currentTimeEpoch >= endLogoutPeriodTimeEpoch)
    {
       loginRoutineInLogoutPeriodTimer.stop();

      if (settings.deepSleepEnabled)
      {
        deepSleepTimer.start();
      }
    }
  }
  else
  {
    Logger::instance().writeToLog("Cannot connect to time server. Trying to perform login.");
  }

  loginRoutine(true);
}
