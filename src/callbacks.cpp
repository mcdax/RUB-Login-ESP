#include "callbacks.h"
#include "config.h"
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#include "logger.h"

extern Ticker deepSleepTimer;
extern Ticker loginRoutineInLogoutPeriodTimer;

bool loginRoutine(bool onlineCheck);

void initWebServerCallbacks(ESP8266WebServer &server)
{
  server.on("/", HTTP_GET, [&server]() {
    site.build_site();

    if (loginRoutineInLogoutPeriodTimer.state() != RUNNING && DataHandler::instance().getSettings().deepSleepEnabled)
    {
      deepSleepTimer.stop();
      deepSleepTimer.start();
    }

    server.sendHeader("Connection", "close");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/html", site.site);
  });
  server.on("/update", HTTP_POST, [&server]() {

     if(loginRoutineInLogoutPeriodTimer.state() != RUNNING &&  DataHandler::instance().getSettings().deepSleepEnabled)
      {
        deepSleepTimer.stop();
      }

      server.sendHeader("Connection", "close");
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
      ESP.restart(); }, [&server]() {
      HTTPUpload& upload = server.upload();
      if(upload.status == UPLOAD_FILE_START){
        Serial.setDebugOutput(true);
        WiFiUDP::stopAll();
        Serial.printf("Update: %s\n", upload.filename.c_str());
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if(!Update.begin(maxSketchSpace)){//start with max available size
          server.send(200, "text/html", "Update Failed...");
          Update.printError(Serial);
        }
      } else if(upload.status == UPLOAD_FILE_WRITE){
        if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
          Update.printError(Serial);
          server.send(200, "text/html", "Update Failed...");
        }
      } else if(upload.status == UPLOAD_FILE_END){
        if(Update.end(true)){ //true to set the size to the current progress
          server.send(200, "text/html", "Update Success: Rebooting...");
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          server.send(200, "text/html", "Update Failed...");
          Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
      }
      yield(); });

  server.on("/savelogin", HTTP_POST, [&server]() {
    if (loginRoutineInLogoutPeriodTimer.state() != RUNNING && DataHandler::instance().getSettings().deepSleepEnabled)
    {
      deepSleepTimer.stop();
    }

    LoginData login;

    strncpy(login.loginID, server.arg("loginid").c_str(), 32);

    if (strcmp("*********", server.arg("password").c_str()) != 0)
    {
      strncpy(login.password, server.arg("password").c_str(), 32);
    }
    else
    {
      strncpy(login.password, DataHandler::instance().getLoginData().password, 32);
    }

    DataHandler::instance().setLoginData(login);

    if (loginRoutine(false))
    {
      server.send(200, "text/html", "Verbindung hergestellt");
      if (loginRoutineInLogoutPeriodTimer.state() != RUNNING && DataHandler::instance().getSettings().deepSleepEnabled)
      {
        deepSleepTimer.start();
      }
    }
    else
    {
      server.send(200, "text/html", "Fehler");
    }
  });

  server.on("/settings", HTTP_POST, [&server]() {
    if (loginRoutineInLogoutPeriodTimer.state() != RUNNING && DataHandler::instance().getSettings().deepSleepEnabled)
    {
      deepSleepTimer.stop();
    }

    if (server.arg("action").equals("savesettings"))
    {
      bool valid = true;
      String errorMsg;

      if (server.arg("logoutperiodstartminute").toInt() >= 60 || server.arg("logoutperiodstopminute").toInt() >= 60 || server.arg("logoutperiodstarthour").toInt() >= 24 || server.arg("logoutperiodstophour").toInt() >= 24)
      {
        valid = false;
        errorMsg = "Invalid time";
      }

      if (server.arg("loginroutineinlogoutperiodinterval").toInt() < 15)
      {
        valid = false;
        errorMsg = "Invalid login interval while in logout period";
      }

      if (server.arg("deepsleepduration").toInt() < 30 || server.arg("deepsleepduration").toInt() > 4260)
      {
        valid = false;
        errorMsg = "Invalid Deep Sleep duration";
      }

      if (server.arg("deepsleepinterval").toInt() < 40)
      {
        valid = false;
        errorMsg = "Invalid time before falling into Deep Sleep";
      }

      if (server.arg("periodicdroutineinterval").toInt() < 30)
      {
        valid = false;
        errorMsg = "Invalid perdiodic routine interval";
      }

      if (server.arg("ntppool").length() < 3)
      {
        valid = false;
        errorMsg = "Invalid NTP Pool";
      }

      if (server.arg("pingaddress").length() < 3)
      {
        valid = false;
        errorMsg = "Invalid ping address";
      }

      if (valid)
      {

        // Object for new Settings
        Settings settings;
        settings.logoutPeriodStartMinute = server.arg("logoutperiodstartminute").toInt();
        settings.logoutPeriodStartHour = server.arg("logoutperiodstarthour").toInt();
        settings.logoutPeriodEndMinute = server.arg("logoutperiodstopminute").toInt();
        settings.logoutPeriodEndHour = server.arg("logoutperiodstophour").toInt();
        settings.loginRoutineInLogoutPeriodIntervalMS = server.arg("loginroutineinlogoutperiodinterval").toInt() * 1000;
        settings.deepSleepEnabled = server.arg("deepsleepenabled").toInt();
        settings.deepSleepDurationMS = server.arg("deepsleepduration").toInt() * 1000;
        settings.deepSleepIntervalMS = server.arg("deepsleepinterval").toInt() * 1000;
        settings.periodicRoutineIntervalMS = server.arg("periodicdroutineinterval").toInt() * 1000;
        strncpy(settings.ntpPool, server.arg("ntppool").c_str(), 32);
        strncpy(settings.pingAddress, server.arg("pingaddress").c_str(), 32);

        DataHandler::instance().setSettings(settings);

        server.send(200, "text/html", "Settings saved");
      }
      else
      {
        server.send(200, "text/html", errorMsg);
      }
    }
    else if (server.arg("action").equals("resetsettings"))
    {
      Settings settings;

      settings.logoutPeriodStartHour = CONFIG_LOGOUT_PERIOD_START_HOUR;
      settings.logoutPeriodStartMinute = CONFIG_LOGOUT_PERIOD_START_MINUTE;
      settings.logoutPeriodEndHour = CONFIG_LOGOUT_PERIOD_END_HOUR;
      settings.logoutPeriodEndMinute = CONFIG_LOGOUT_PERIOD_END_MINUTE;
      settings.loginRoutineInLogoutPeriodIntervalMS = CONFIG_LOGIN_ROUTINE_IN_LOGOUT_PERIOD_INTERVAL_MS;
      settings.periodicRoutineIntervalMS = CONFIG_PERIODIC_ROUTINE_INTERVAL_MS;
      settings.deepSleepEnabled = CONFIG_DEEPSLEEP;
      settings.deepSleepDurationMS = CONFIG_DEEPSLEEP_DURATION_MS;
      settings.deepSleepIntervalMS = CONFIG_DEEPSLEEP_INTERVAL_MS;
      strncpy(settings.ntpPool, CONFIG_NTP_POOL, 32);
      strncpy(settings.pingAddress, CONFIG_PING_ADDRESS, 32);

      DataHandler::instance().setSettings(settings);

      server.send(200, "text/html", "Settings reseted");
    }

    if (loginRoutineInLogoutPeriodTimer.state() != RUNNING &&  DataHandler::instance().getSettings().deepSleepEnabled)
    {
      deepSleepTimer.start();
    }
  });
}