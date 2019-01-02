#include "site.h"
#include "datahandler.h"
#include "config.h"

Site::Site()
{
  build_site();
}

void Site::build_site()
{
  String siteTemplate = R"(<head>
    <style>
    div,fieldset,input,select{padding:5px;font-size:1em;}
    input{width:100%;box-sizing:border-box;-webkit-box-sizing:border-box;-moz-box-sizing:border-box;}
    select{width:100%;}
    textarea{resize:none;width:98%;height:318px;padding:5px;overflow:auto;}
    body{text-align:center;font-family:verdana;}
    td{padding:0px;}
    button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;-webkit-transition-duration:0.4s;transition-duration:0.4s;cursor:pointer;}
    button:hover{background-color:#006cba;}
    a{text-decoration:none;}
    .p{float:left;text-align:left;}
    .q{float:right;text-align:right;}
    </style>
    </head>
    <body>
    <div style='text-align:left;display:inline-block;min-width:340px;'>
    <div style='text-align:center;'>
    <h3>%firmware_name</h3>
    <h2>%firmware_version</h2>
    </div>
    <fieldset>
    <legend><b>&nbsp;Login data&nbsp;</b></legend>
    <form method='POST' action='/savelogin'>
    <b>LoginID</b><br/><input id='loginid' name='loginid' value='%loginid'><br/><br/>
    <b>Password</b><br/><input id='password' name='password' type='password' value='%password'><br/><br/>
    <button type='submit'>Save</button>
    </form>
    </fieldset>
    <br>
    <fieldset>
    <legend><b>&nbsp;Log&nbsp;</b></legend>
    <textarea name="log" form="log">%systemlog</textarea> 
    </fieldset>
    <br> 
    <fieldset>
    <legend><b>&nbsp;Advanced settings&nbsp;</b></legend>
    <form method='POST' action='/settings'>
   
    <b>Logout period start time (HH:mm)</b><br/><input id='logoutperiodstarthour' name='logoutperiodstarthour' value='%logoutperiodstarthour' maxlength="2" style="width: 40px;text-align: center;">:<input id='logoutperiodstartminute' name='logoutperiodstartminute' value='%logoutperiodstartminute' maxlength="2" style="width: 40px;text-align: center;"><br/><br/>
    <b>Logout period stop time (HH:mm)</b><br/><input id='logoutperiodstophour' name='logoutperiodstophour' value='%logoutperiodstophour' maxlength="2" style="width: 40px;text-align: center;">:<input id='logoutperiodstopminute' name='logoutperiodstopminute' value='%logoutperiodstopminute' maxlength="2" style="width: 40px;text-align: center;"><br/><br/>
    <b>Login interval while in logout period (s) (min. 15)</b><br/><input id='loginroutineinlogoutperiodinterval' name='loginroutineinlogoutperiodinterval' value='%loginroutineinlogoutperiodinterval'><br/><br/>

    <b>Deep Sleep (GPIO16 needs to be connected to RST!)</b><input type="checkbox" id="deepsleepenabled" name="deepsleepenabled" value="1" style="width: 30px;" %deepsleepenabled /><br/><br/>
    <b>Deep Sleep duration (s) (min. 30, max. 4260)</b><br/><input id='deepsleepduration' name='deepsleepduration' value='%deepsleepduration'><br/><br/>
    <b>Time before falling into Deep Sleep (s) (min. 40)</b><br/><input id='deepsleepinterval' name='deepsleepinterval' value='%deepsleepinterval'><br/><br/>

    <b>Login interval (if Deep Sleep is disabled)(s) (min. 30)</b><br/><input id='periodicdroutineinterval' name='periodicdroutineinterval' value='%periodicdroutineinterval'><br/><br/>

    <b>Ping address</b><br/><input id='pingaddress' name='pingaddress' value='%pingaddress'><br/><br/>
    <b>Primary NTP Server</b><br/><input id='ntppool' name='ntppool' value='%ntppool'><br/><br/>

    <button type='submit' name="action" value='savesettings'>Save</button>
    <br/><br/>
    <button type='submit' name="action" value='resetsettings'>Reset</button>
    </form>
    </fieldset>
    <br>
    <fieldset>
    <br> 
    <fieldset><legend><b>&nbsp;Firmware Update&nbsp;</b></legend><form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><button type='submit'>Update</button></form></fieldset>
    </div>
    </body>)";

  DataHandler &dataHandler = DataHandler::instance();

  site = siteTemplate;
  site.replace("%loginid", dataHandler.getLoginData().loginID);
  site.replace("%password", "*********");
  site.replace("%systemlog", dataHandler.log);
  site.replace("%firmware_name", CONFIG_FIRMWARE_NAME);
  site.replace("%firmware_version", CONFIG_FIRMWARE_VERSION);

  site.replace("%logoutperiodstartminute", String(dataHandler.getSettings().logoutPeriodStartMinute));
  site.replace("%logoutperiodstarthour", String(dataHandler.getSettings().logoutPeriodStartHour));
  site.replace("%logoutperiodstopminute", String(dataHandler.getSettings().logoutPeriodEndMinute));
  site.replace("%logoutperiodstophour", String(dataHandler.getSettings().logoutPeriodEndHour));
  site.replace("%loginroutineinlogoutperiodinterval", String((int)(dataHandler.getSettings().loginRoutineInLogoutPeriodIntervalMS / 1000)));
  site.replace("%deepsleepenabled", dataHandler.getSettings().deepSleepEnabled ? "checked" : "");
  site.replace("%deepsleepduration", String((int)(dataHandler.getSettings().deepSleepDurationMS / 1000)));
  site.replace("%deepsleepinterval", String((int)(dataHandler.getSettings().deepSleepIntervalMS / 1000)));
  site.replace("%periodicdroutineinterval", String((int)(dataHandler.getSettings().periodicRoutineIntervalMS / 1000)));
  site.replace("%pingaddress", String(dataHandler.getSettings().pingAddress));
  site.replace("%ntppool", String(dataHandler.getSettings().ntpPool));
}