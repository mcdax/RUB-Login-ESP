#include "logger.h"

Logger::Logger()
{
}

void Logger::writeToLog(String msg)
{
  writeToSerial(msg);
  writeToDatabase(msg);
}

void Logger::writeToSerial(String &msg)
{
  Serial.println(msg);
}

void Logger::writeToDatabase(String &msg)
{
  DataHandler::instance().log = DataHandler::instance().log + msg + "\n";
}