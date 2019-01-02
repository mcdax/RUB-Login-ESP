#include "datahandler.h"
#pragma once

class Logger
{
  public:
    static Logger &instance()
    {
        static Logger _instance;
        return _instance;
    }

    void writeToLog(String msg);

  private:
    Logger();
    void writeToSerial(String &msg);
    void writeToDatabase(String &msg);
};