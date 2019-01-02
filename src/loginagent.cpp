#include "loginagent.h"
#include "logger.h"

LoginAgent::LoginAgent(WiFiClientSecure &client)
{
  this->client = &client;
}

LoginAgent::LoginStatus LoginAgent::login(LoginData &loginData)
{
  String ip;

  if (!getIP(ip))
  {
    Logger::instance().writeToLog("Error while getting IP address");

    return LOGIN_FAILED_GETTING_IP;
  }

  if (!performLogin(loginData, ip))
  {
    Logger::instance().writeToLog("Error while performing login");
    return LOGIN_DATA_WRONG;
  }

  return LOGIN_SUCCESSFUL;
}

bool LoginAgent::getIP(String &ip)
{
  const String host = "login.rz.ruhr-uni-bochum.de";

  if (!client->connect(host, 443))
  {
    return false;
  }

  String url = "/cgi-bin/start";
  client->print(String("GET ") + url + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:64.0) Gecko/20100101 Firefox/64.0\r\n" +
                "Connection: close\r\n\r\n");

  while (client->connected())
  {
    String line = client->readStringUntil('\n');
    if (int index = line.indexOf("<input type=\"ipaddr\" name=\"ipaddr\" value=\"") > 0)
    {
      line = line.substring(line.indexOf("value=") + String("value=").length() + 1);
      line = line.substring(0, line.indexOf("\""));
      ip = line;
      Serial.println(line);
      return true;
    }
  }
  return false;
}

bool LoginAgent::performLogin(LoginData &loginData, String &ip)
{
  const String host = "login.rz.ruhr-uni-bochum.de";

  String url = "/cgi-bin/laklogin";

  String data = "code=1&loginid=" + String(loginData.loginID) + "&password=" + urlencode(String(loginData.password)) + "&ipaddr=" + ip + "&action=Login";

  if (client->connect(host, 443))
  {
    client->println("POST " + url + " HTTP/1.1");
    client->println("Host: " + host);
    client->println("User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:60.0) Gecko/20100101 Firefox/60.0");
    client->println("Connection: close");
    client->println("Content-Type: application/x-www-form-urlencoded;");
    client->print("Content-Length: ");
    client->println(data.length());
    client->println();
    client->println(data);
    delay(10);
    String response = client->readString();
    String msg = response.substring(response.indexOf("<font face=\"Helvetica, Arial, sans-serif\"><big><big>") + String("<font face=\"Helvetica, Arial, sans-serif\"><big><big>").length());
    String time = msg.substring(msg.indexOf("</small>") - String("YYYY-MM-DD HH:MM:SS").length(), msg.indexOf("</small>"));
    msg = msg.substring(0, msg.indexOf("<br>"));

    Logger::instance().writeToLog("[" + time + "] " + "IP: " + ip + ", loginID: " + loginData.loginID + " - " + msg);

    bool login_successful = msg.equals("Authentisierung gelungen");

    return login_successful ? LOGIN_SUCCESSFUL : LOGIN_DATA_WRONG;
  }

  Logger::instance().writeToLog("Connection failed");
  return false;
}

String LoginAgent::urlencode(String str)
{
  String encodedString = "";
  char c;
  char code0;
  char code1;
  char code2;
  for (int i = 0; i < str.length(); i++)
  {
    c = str.charAt(i);
    if (c == ' ')
    {
      encodedString += '+';
    }
    else if (isalnum(c))
    {
      encodedString += c;
    }
    else
    {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9)
      {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9)
      {
        code0 = c - 10 + 'A';
      }
      code2 = '\0';
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
    yield();
  }
  return encodedString;
}