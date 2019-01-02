#include "datahandler.h"
#include <WiFiClientSecure.h>

class LoginAgent
{
  public:
    enum LoginStatus
    {
        LOGIN_SUCCESSFUL,
        LOGIN_DATA_WRONG,
        LOGIN_FAILED_GETTING_IP
    };

    LoginStatus login(LoginData &loginData);
    LoginAgent(WiFiClientSecure &client);

  private:
    bool performLogin(LoginData &loginData, String &ip);
    bool getIP(String &);
    String urlencode(String str);
    WiFiClientSecure *client;
};