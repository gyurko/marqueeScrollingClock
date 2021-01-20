/** The MIT License (MIT)

  Copyright (c) 2018 David Payne

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

/**********************************************
  Edit Settings.h for personalization
***********************************************/
#include <Arduino.h>
#include <Wire.h>

#include "Settings.h"

#define VERSION "1.0"

#define HOSTNAME "GY-CLOCK-"
#define CONFIG "/conf.txt"
#define BUZZER_PIN D2

/* Useful Constants */
//#define SECS_PER_MIN (60UL)
//#define SECS_PER_HOUR (3600UL)
//#define SECS_PER_DAY (SECS_PER_HOUR * 24L)

/* Useful Macros for getting elapsed time */
//#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)
//#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN)
//#define numberOfHours(_time_) ((_time_ % SECS_PER_DAY) / SECS_PER_HOUR)
//#define elapsedDays(_time_) (_time_ / SECS_PER_DAY)

// declairing prototypes
void configModeCallback(WiFiManager *myWiFiManager);
int8_t getWifiQuality();
void readSettings();
void centerPrint(String msg);
void centerPrint(String msg, boolean extraStuff);
void scrollMessage(String msg);
void displayWeatherData();
void configModeCallback(WiFiManager *myWiFiManager);
void flashLED(int number, int delayTime);
String getTempSymbol();
String getSpeedSymbol();
String getPressureSymbol();
int8_t getWifiQuality();
String getTimeTillUpdate();
int getMinutesFromLastRefresh();
int getMinutesFromLastDisplay();
void enableDisplay(boolean enable);
void checkDisplay();
String writeSettings();
void readSettings();
void scrollMessage(String msg);
void centerPrint(String msg);
void centerPrint(String msg, boolean extraStuff);
String decodeHtmlString(String msg);
void handlePull();
void handleSaveWideClock();
void handleLocations();
void handleSystemReset();
void handleForgetWifi();
void restartEsp();
void handleWideClockConfigure();
void handleConfigure();
void handleDisplay();
void sendHeader();
void sendFooter();
void redirectHome();
String secondsIndicator(boolean isRefresh);
void getWeatherData();
String hourMinutes(boolean isRefresh);
void displayMessage(String message);

// LED Settings
const int offset = 1;
int refresh = 0;
String message = "hello";
int spacer = 1;         // dots between letters
int width = 5 + spacer; // The font width is 5 pixels + spacer
Max72xxPanel m_displayPanel = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);
String Wide_Clock_Style = "1"; // 1="hh:mm Temp", 2="hh:mm:ss", 3="hh:mm"
float UtcOffset;               // time zone offsets that correspond with the CityID above
                               // (offset from GMT)

// Time
TimeDB m_timeDB("");
String m_lastSecond = "xx";
int displayRefreshCount = 1;
long lastEpoch = 0;
long firstEpoch = 0;
long displayOffEpoch = 0;
boolean displayOn = true;

// Weather Client
OpenWeatherMapClient m_openWeatherMapClient(APIKEY, CityIDs, 1, IS_METRIC);
// (some) Default Weather Settings
boolean SHOW_DATE = false;
boolean SHOW_CITY = true;
boolean SHOW_CONDITION = true;
boolean SHOW_HUMIDITY = true;
boolean SHOW_WIND = true;
boolean SHOW_WINDDIR = true;
boolean SHOW_PRESSURE = false;
boolean SHOW_HIGHLOW = true;

ESP8266WebServer webServer(WEBSERVER_PORT);
ESP8266HTTPUpdateServer serverUpdater;

static const char WEB_ACTIONS1[] PROGMEM = "<a class='w3-bar-item w3-button' href='/'><i class='fas fa-home'></i> "
                                           "Home</a>"
                                           "<a class='w3-bar-item w3-button' href='/configure'><i class='fas "
                                           "fa-cog'></i> Configure</a>";

static const char WEB_ACTIONS2[] PROGMEM =
    "<a class='w3-bar-item w3-button' href='/pull'><i class='fas fa-cloud-download-alt'></i> Refresh Data</a>"
    "<a class='w3-bar-item w3-button' href='/display'>";

static const char WEB_ACTION3[] PROGMEM = "</a><a class='w3-bar-item w3-button' href='/systemreset' onclick='return "
                                          "confirm(\"Do you want to reset to default weather settings?\")'><i "
                                          "class='fas fa-undo'></i> Reset Settings</a>"
                                          "<a class='w3-bar-item w3-button' href='/forgetwifi' onclick='return "
                                          "confirm(\"Do you want to forget to WiFi connection?\")'><i class='fas "
                                          "fa-wifi'></i> Forget WiFi</a>"
                                          "<a class='w3-bar-item w3-button' href='/restart'><i class='fas "
                                          "fa-sync'></i> Restart</a>"
                                          "<a class='w3-bar-item w3-button' href='/update'><i class='fas "
                                          "fa-wrench'></i> Firmware Update</a>"
                                          "<a class='w3-bar-item w3-button' "
                                          "href='https://github.com/Qrome/marquee-scroller' target='_blank'><i "
                                          "class='fas fa-question-circle'></i> About</a>";

static const char CHANGE_FORM1[] PROGMEM = "<form class='w3-container' action='/locations' "
                                           "method='get'><h2>Configure:</h2>"
                                           "<label>TimeZone DB API Key (get from <a "
                                           "href='https://timezonedb.com/register' target='_BLANK'>here</a>)</label>"
                                           "<input class='w3-input w3-border w3-margin-bottom' type='text' "
                                           "name='TimeZoneDB' value='%TIMEDBKEY%' maxlength='60'>"
                                           "<label>OpenWeatherMap API Key (get from <a "
                                           "href='https://openweathermap.org/' target='_BLANK'>here</a>)</label>"
                                           "<input class='w3-input w3-border w3-margin-bottom' type='text' "
                                           "name='openWeatherMapApiKey' value='%WEATHERKEY%' maxlength='70'>"
                                           "<p><label>%CITYNAME1% (<a href='http://openweathermap.org/find' "
                                           "target='_BLANK'><i class='fas fa-search'></i> Search for City "
                                           "ID</a>)</label>"
                                           "<input class='w3-input w3-border w3-margin-bottom' type='text' "
                                           "name='city1' value='%CITY1%' onkeypress='return isNumberKey(event)'></p>"
                                           "<p><input name='metric' class='w3-check w3-margin-top' type='checkbox' "
                                           "%CHECKED%> Use Metric (Celsius)</p>"
                                           "<p><input name='showdate' class='w3-check w3-margin-top' type='checkbox' "
                                           "%DATE_CHECKED%> Display Date</p>"
                                           "<p><input name='showcity' class='w3-check w3-margin-top' type='checkbox' "
                                           "%CITY_CHECKED%> Display City Name</p>"
                                           "<p><input name='showhighlow' class='w3-check w3-margin-top' "
                                           "type='checkbox' %HIGHLOW_CHECKED%> Display Daily High/Low Temperatures</p>"
                                           "<p><input name='showcondition' class='w3-check w3-margin-top' "
                                           "type='checkbox' %CONDITION_CHECKED%> Display Weather Condition</p>"
                                           "<p><input name='showhumidity' class='w3-check w3-margin-top' "
                                           "type='checkbox' %HUMIDITY_CHECKED%> Display Humidity</p>"
                                           "<p><input name='showwind' class='w3-check w3-margin-top' type='checkbox' "
                                           "%WIND_CHECKED%> Display Wind</p>"
                                           "<p><input name='showpressure' class='w3-check w3-margin-top' "
                                           "type='checkbox' %PRESSURE_CHECKED%> Display Barometric Pressure</p>"
                                           "<p><input name='is24hour' class='w3-check w3-margin-top' type='checkbox' "
                                           "%IS_24HOUR_CHECKED%> Use 24 Hour Clock (military time)</p>";

static const char CHANGE_FORM2[] PROGMEM = "<p><input name='isPM' class='w3-check w3-margin-top' type='checkbox' "
                                           "%IS_PM_CHECKED%> Show PM indicator (only 12h format)</p>"
                                           "<p><input name='flashseconds' class='w3-check w3-margin-top' "
                                           "type='checkbox' %FLASHSECONDS%> Flash : in the time</p>"
                                           "<p><label>Marquee Message (up to 60 chars)</label><input class='w3-input "
                                           "w3-border w3-margin-bottom' type='text' name='marqueeMsg' value='%MSG%' "
                                           "maxlength='60'></p>"
                                           "<p><label>Start Time </label><input name='startTime' type='time' "
                                           "value='%STARTTIME%'></p>"
                                           "<p><label>End Time </label><input name='endTime' type='time' "
                                           "value='%ENDTIME%'></p>"
                                           "<p>Display Brightness <input class='w3-border w3-margin-bottom' "
                                           "name='ledintensity' type='number' min='0' max='15' "
                                           "value='%INTENSITYOPTIONS%'></p>"
                                           "<p>Display Scroll Speed <select class='w3-option w3-padding' "
                                           "name='scrollspeed'>%SCROLLOPTIONS%</select></p>"
                                           "<p>Minutes Between Refresh Data <select class='w3-option w3-padding' "
                                           "name='refresh'>%OPTIONS%</select></p>"
                                           "<p>Seconds Between Scrolling Data <input class='w3-border "
                                           "w3-margin-bottom' name='refreshDisplay' type='number' min='1' max='60' "
                                           "value='%REFRESH_DISPLAY%'></p>";

static const char CHANGE_FORM3[] PROGMEM = "<hr><p><input name='isBasicAuth' class='w3-check w3-margin-top' "
                                           "type='checkbox' %IS_BASICAUTH_CHECKED%> Use Security Credentials for "
                                           "Configuration Changes</p>"
                                           "<p><label>Marquee User ID (for this web interface)</label><input "
                                           "class='w3-input w3-border w3-margin-bottom' type='text' name='userid' "
                                           "value='%USERID%' maxlength='20'></p>"
                                           "<p><label>Marquee Password </label><input class='w3-input w3-border "
                                           "w3-margin-bottom' type='password' name='stationpassword' "
                                           "value='%STATIONPASSWORD%'></p>"
                                           "<p><button class='w3-button w3-block w3-green w3-section w3-padding' "
                                           "type='submit'>Save</button></p></form>"
                                           "<script>function isNumberKey(e){var "
                                           "h=e.which?e.which:event.keyCode;return!(h>31&&(h<48||h>57))}</script>";

static const char WIDECLOCK_FORM[] PROGMEM = "<form class='w3-container' action='/savewideclock' method='get'><h2>Wide "
                                             "Clock Configuration:</h2>"
                                             "<p>Wide Clock Display Format <select class='w3-option w3-padding' "
                                             "name='wideclockformat'>%WIDECLOCKOPTIONS%</select></p>"
                                             "<button class='w3-button w3-block w3-grey w3-section w3-padding' "
                                             "type='submit'>Save</button></form>";

const int TIMEOUT = 500; // 500 = 1/2 second
int timeoutCount = 0;

// Change the externalLight to the pin you wish to use if other than the
// Built-in LED
int externalLight = LED_BUILTIN; // LED_BUILTIN is is the built in LED on the Wemos

void setup()
{
    Serial.begin(115200);
    LittleFS.begin();
    delay(10);

    // Initialize digital pin for LED
    pinMode(externalLight, OUTPUT);

    // New Line to clear from start garbage
    Serial.println();

    readSettings();

    Serial.println("Number of LED Displays: " + String(numberOfHorizontalDisplays));
    // initialize dispaly
    m_displayPanel.setIntensity(0); // Use a value between 0 and 15 for brightness

    int maxPos = numberOfHorizontalDisplays * numberOfVerticalDisplays;
    for (int i = 0; i < maxPos; i++)
    {
        m_displayPanel.setRotation(i, ledRotation);
        m_displayPanel.setPosition(i, maxPos - i - 1, 0);
    }

    Serial.println("matrix created");
    m_displayPanel.fillScreen(LOW); // show black
    centerPrint("hello");

    for (int inx = 0; inx <= 15; inx++)
    {
        m_displayPanel.setIntensity(inx);
        delay(100);
    }
    for (int inx = 15; inx >= 0; inx--)
    {
        m_displayPanel.setIntensity(inx);
        delay(60);
    }
    delay(1000);
    m_displayPanel.setIntensity(displayIntensity);
    // noTone(BUZZER_PIN);

    // WiFiManager
    // Local intialization. Once its business is done, there is no need to keep it
    // around
    WiFiManager wifiManager;

    // Uncomment for testing wifi manager
    // wifiManager.resetSettings();
    wifiManager.setAPCallback(configModeCallback);

    // Custom Station (client) Static IP Configuration - Set custom IP for your
    // Network (IP, Gateway, Subnet mask)
    // wifiManager.setSTAStaticIPConfig(IPAddress(192,168,0,99),
    // IPAddress(192,168,0,1), IPAddress(255,255,255,0));

    String hostname(HOSTNAME);
    hostname += String(ESP.getChipId(), HEX);
    if (!wifiManager.autoConnect((const char *)hostname.c_str()))
    { // new addition
        delay(3000);
        WiFi.disconnect(true);
        ESP.reset();
        delay(5000);
    }

    // print the received signal strength:
    Serial.print("Signal Strength (RSSI): ");
    Serial.print(getWifiQuality());
    Serial.println("%");

    if (ENABLE_OTA)
    {
        ArduinoOTA.onStart([]() { Serial.println("Start"); });
        ArduinoOTA.onEnd([]() { Serial.println("\nEnd"); });
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
            Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        });
        ArduinoOTA.onError([](ota_error_t error) {
            Serial.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR)
                Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR)
                Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR)
                Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR)
                Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR)
                Serial.println("End Failed");
        });
        ArduinoOTA.setHostname((const char *)hostname.c_str());
        if (OTA_Password != "")
        {
            ArduinoOTA.setPassword(((const char *)OTA_Password.c_str()));
        }
        ArduinoOTA.begin();
    }

    if (WEBSERVER_ENABLED)
    {
        webServer.on("/", displayWeatherData);
        webServer.on("/pull", handlePull);
        webServer.on("/locations", handleLocations);
        webServer.on("/savewideclock", handleSaveWideClock);
        webServer.on("/systemreset", handleSystemReset);
        webServer.on("/forgetwifi", handleForgetWifi);
        webServer.on("/restart", restartEsp);
        webServer.on("/configure", handleConfigure);
        webServer.on("/configurewideclock", handleWideClockConfigure);
        webServer.on("/display", handleDisplay);
        webServer.onNotFound(redirectHome);
        serverUpdater.setup(&webServer, "/update", www_username, www_password);
        // Start the server
        webServer.begin();
        Serial.println("Server started");
        // Print the IP address
        String webAddress = "http://" + WiFi.localIP().toString() + ":" + String(WEBSERVER_PORT) + "/";
        Serial.println("Use this URL : " + webAddress);
        scrollMessage(" IP: " + WiFi.localIP().toString() + "  ");
    }
    else
    {
        Serial.println("Web Interface is Disabled");
        scrollMessage("Web Interface is Disabled");
    }

    flashLED(1, 500);
}

//************************************************************
// Main Looop
//************************************************************
void loop()
{
    // Get some Weather Data to serve
    if ((getMinutesFromLastRefresh() >= minutesBetweenDataRefresh) || lastEpoch == 0)
    {
        getWeatherData();
    }
    checkDisplay(); // this will see if we need to turn it on or off for night
                    // mode.

    if (m_lastSecond != m_timeDB.zeroPad(second()))
    {
        m_lastSecond = m_timeDB.zeroPad(second());

        if (m_openWeatherMapClient.getError() != "")
        {
            scrollMessage(m_openWeatherMapClient.getError());
            return;
        }

        if (displayOn)
        {
            m_displayPanel.shutdown(false);
        }
        m_displayPanel.fillScreen(LOW); // show black

        displayRefreshCount--;
        // Check to see if we need to Scroll some Data
        if (displayRefreshCount <= 0)
        {
            displayRefreshCount = secondsBetweenScrolling;
            String temperature = m_openWeatherMapClient.getTempRounded(0);
            String description = m_openWeatherMapClient.getDescription(0);
            description.toUpperCase();
            String msg;

            msg += " " + temperature + getTempSymbol();

            centerPrint(msg);

            delay(2000);
        }
    }

    String currentTime = hourMinutes(false);

    if (numberOfHorizontalDisplays >= 8)
    {
        if (Wide_Clock_Style == "1")
        {
            // On Wide Display -- show the current temperature as well
            String currentTemp = m_openWeatherMapClient.getTempRounded(0);
            String timeSpacer = "  ";
            if (currentTemp.length() >= 3)
            {
                timeSpacer = " ";
            }
            currentTime += timeSpacer + currentTemp + getTempSymbol();
        }
        if (Wide_Clock_Style == "2")
        {
            currentTime += secondsIndicator(false) + m_timeDB.zeroPad(second());
            m_displayPanel.fillScreen(LOW); // show black
        }
        if (Wide_Clock_Style == "3")
        {
            // No change this is normal clock display
        }
    }
    m_displayPanel.fillScreen(LOW);
    centerPrint(currentTime, true);

    if (WEBSERVER_ENABLED)
    {
        webServer.handleClient();
    }
    if (ENABLE_OTA)
    {
        ArduinoOTA.handle();
    }
}

String hourMinutes(boolean isRefresh)
{
    if (IS_24HOUR)
    {
        return hour() + secondsIndicator(isRefresh) + m_timeDB.zeroPad(minute());
    }
    else
    {
        return hourFormat12() + secondsIndicator(isRefresh) + m_timeDB.zeroPad(minute());
    }
}

String secondsIndicator(boolean isRefresh)
{
    String rtnValue = ":";
    if (isRefresh == false && (flashOnSeconds && (second() % 2) == 0))
    {
        rtnValue = " ";
    }
    return rtnValue;
}

boolean athentication()
{
    if (IS_BASIC_AUTH)
    {
        return webServer.authenticate(www_username, www_password);
    }
    return true; // Authentication not required
}

void handlePull()
{
    getWeatherData(); // this will force a data pull for new weather
    displayWeatherData();
}

void handleSaveWideClock()
{
    if (!athentication())
    {
        return webServer.requestAuthentication();
    }
    if (numberOfHorizontalDisplays >= 8)
    {
        Wide_Clock_Style = webServer.arg("wideclockformat");
        writeSettings();
        m_displayPanel.fillScreen(LOW); // show black
    }
    redirectHome();
}

void handleLocations()
{
    if (!athentication())
    {
        return webServer.requestAuthentication();
    }
    TIMEDBKEY = webServer.arg("TimeZoneDB");
    APIKEY = webServer.arg("openWeatherMapApiKey");
    CityIDs[0] = webServer.arg("city1").toInt();
    flashOnSeconds = webServer.hasArg("flashseconds");
    IS_24HOUR = webServer.hasArg("is24hour");
    IS_PM = webServer.hasArg("isPM");
    SHOW_DATE = webServer.hasArg("showdate");
    SHOW_CITY = webServer.hasArg("showcity");
    SHOW_CONDITION = webServer.hasArg("showcondition");
    SHOW_HUMIDITY = webServer.hasArg("showhumidity");
    SHOW_WIND = webServer.hasArg("showwind");
    SHOW_PRESSURE = webServer.hasArg("showpressure");
    SHOW_HIGHLOW = webServer.hasArg("showhighlow");
    IS_METRIC = webServer.hasArg("metric");
    marqueeMessage = decodeHtmlString(webServer.arg("marqueeMsg"));
    timeDisplayTurnsOn = decodeHtmlString(webServer.arg("startTime"));
    timeDisplayTurnsOff = decodeHtmlString(webServer.arg("endTime"));
    displayIntensity = webServer.arg("ledintensity").toInt();
    minutesBetweenDataRefresh = webServer.arg("refresh").toInt();
    secondsBetweenScrolling = webServer.arg("refreshDisplay").toInt();
    displayScrollSpeed = webServer.arg("scrollspeed").toInt();
    IS_BASIC_AUTH = webServer.hasArg("isBasicAuth");
    String temp = webServer.arg("userid");
    temp.toCharArray(www_username, sizeof(temp));
    temp = webServer.arg("stationpassword");
    temp.toCharArray(www_password, sizeof(temp));
    m_openWeatherMapClient.setMetric(IS_METRIC);
    m_displayPanel.fillScreen(LOW); // show black
    writeSettings();
    getWeatherData(); // this will force a data pull for new weather
    redirectHome();
}

void handleSystemReset()
{
    if (!athentication())
    {
        return webServer.requestAuthentication();
    }
    Serial.println("Reset System Configuration");
    if (LittleFS.remove(CONFIG))
    {
        redirectHome();
        ESP.restart();
    }
}

void handleForgetWifi()
{
    if (!athentication())
    {
        return webServer.requestAuthentication();
    }
    // WiFiManager
    // Local intialization. Once its business is done, there is no need to keep it
    // around
    redirectHome();
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    ESP.restart();
}

void restartEsp()
{
    redirectHome();
    ESP.restart();
}

void handleWideClockConfigure()
{
    if (!athentication())
    {
        return webServer.requestAuthentication();
    }
    digitalWrite(externalLight, LOW);

    webServer.sendHeader("Cache-Control", "no-cache, no-store");
    webServer.sendHeader("Pragma", "no-cache");
    webServer.sendHeader("Expires", "-1");
    webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
    webServer.send(200, "text/html", "");

    sendHeader();

    if (numberOfHorizontalDisplays >= 8)
    {
        // Wide display options
        String form = FPSTR(WIDECLOCK_FORM);
        String clockOptions = "<option value='1'>HH:MM Temperature</option><option "
                              "value='2'>HH:MM:SS</option><option value='3'>HH:MM</option>";
        clockOptions.replace(Wide_Clock_Style + "'", Wide_Clock_Style + "' selected");
        form.replace("%WIDECLOCKOPTIONS%", clockOptions);
        webServer.sendContent(form);
    }

    sendFooter();

    webServer.sendContent("");
    webServer.client().stop();
    digitalWrite(externalLight, HIGH);
}

void handleConfigure()
{
    if (!athentication())
    {
        return webServer.requestAuthentication();
    }
    digitalWrite(externalLight, LOW);
    String html = "";

    webServer.sendHeader("Cache-Control", "no-cache, no-store");
    webServer.sendHeader("Pragma", "no-cache");
    webServer.sendHeader("Expires", "-1");
    webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
    webServer.send(200, "text/html", "");

    sendHeader();

    String form = FPSTR(CHANGE_FORM1);
    form.replace("%TIMEDBKEY%", TIMEDBKEY);
    form.replace("%WEATHERKEY%", APIKEY);

    String cityName = "";
    if (m_openWeatherMapClient.getCity(0) != "")
    {
        cityName = m_openWeatherMapClient.getCity(0) + ", " + m_openWeatherMapClient.getCountry(0);
    }
    form.replace("%CITYNAME1%", cityName);
    form.replace("%CITY1%", String(CityIDs[0]));
    String isDateChecked = "";
    if (SHOW_DATE)
    {
        isDateChecked = "checked='checked'";
    }
    form.replace("%DATE_CHECKED%", isDateChecked);
    String isCityChecked = "";
    if (SHOW_CITY)
    {
        isCityChecked = "checked='checked'";
    }
    form.replace("%CITY_CHECKED%", isCityChecked);
    String isConditionChecked = "";
    if (SHOW_CONDITION)
    {
        isConditionChecked = "checked='checked'";
    }
    form.replace("%CONDITION_CHECKED%", isConditionChecked);
    String isHumidityChecked = "";
    if (SHOW_HUMIDITY)
    {
        isHumidityChecked = "checked='checked'";
    }
    form.replace("%HUMIDITY_CHECKED%", isHumidityChecked);
    String isWindChecked = "";
    if (SHOW_WIND)
    {
        isWindChecked = "checked='checked'";
    }
    form.replace("%WIND_CHECKED%", isWindChecked);
    String isPressureChecked = "";
    if (SHOW_PRESSURE)
    {
        isPressureChecked = "checked='checked'";
    }
    form.replace("%PRESSURE_CHECKED%", isPressureChecked);

    String isHighlowChecked = "";
    if (SHOW_HIGHLOW)
    {
        isHighlowChecked = "checked='checked'";
    }
    form.replace("%HIGHLOW_CHECKED%", isHighlowChecked);

    String is24hourChecked = "";
    if (IS_24HOUR)
    {
        is24hourChecked = "checked='checked'";
    }
    form.replace("%IS_24HOUR_CHECKED%", is24hourChecked);
    String checked = "";
    if (IS_METRIC)
    {
        checked = "checked='checked'";
    }
    form.replace("%CHECKED%", checked);
    webServer.sendContent(form);

    form = FPSTR(CHANGE_FORM2);
    String isPmChecked = "";
    if (IS_PM)
    {
        isPmChecked = "checked='checked'";
    }
    form.replace("%IS_PM_CHECKED%", isPmChecked);
    String isFlashSecondsChecked = "";
    if (flashOnSeconds)
    {
        isFlashSecondsChecked = "checked='checked'";
    }
    form.replace("%FLASHSECONDS%", isFlashSecondsChecked);
    form.replace("%MSG%", marqueeMessage);
    form.replace("%STARTTIME%", timeDisplayTurnsOn);
    form.replace("%ENDTIME%", timeDisplayTurnsOff);
    form.replace("%INTENSITYOPTIONS%", String(displayIntensity));
    String dSpeed = String(displayScrollSpeed);
    String scrollOptions = "<option value='35'>Slow</option><option "
                           "value='25'>Normal</option><option value='15'>Fast</option><option "
                           "value='10'>Very Fast</option>";
    scrollOptions.replace(dSpeed + "'", dSpeed + "' selected");
    form.replace("%SCROLLOPTIONS%", scrollOptions);
    String minutes = String(minutesBetweenDataRefresh);
    String options = "<option>5</option><option>10</option><option>15</option><option>20</"
                     "option><option>30</option><option>60</option>";
    options.replace(">" + minutes + "<", " selected>" + minutes + "<");
    form.replace("%OPTIONS%", options);
    form.replace("%REFRESH_DISPLAY%", String(secondsBetweenScrolling));

    webServer.sendContent(form); // Send another chunk of the form

    form = FPSTR(CHANGE_FORM3);
    String isUseSecurityChecked = "";
    if (IS_BASIC_AUTH)
    {
        isUseSecurityChecked = "checked='checked'";
    }
    form.replace("%IS_BASICAUTH_CHECKED%", isUseSecurityChecked);
    form.replace("%USERID%", String(www_username));
    form.replace("%STATIONPASSWORD%", String(www_password));

    webServer.sendContent(form); // Send the second chunk of Data

    sendFooter();

    webServer.sendContent("");
    webServer.client().stop();
    digitalWrite(externalLight, HIGH);
}

void handleDisplay()
{
    if (!athentication())
    {
        return webServer.requestAuthentication();
    }
    enableDisplay(!displayOn);
    String state = "OFF";
    if (displayOn)
    {
        state = "ON";
    }
    displayMessage("Display is now " + state);
}

//***********************************************************************
void getWeatherData() // client function to send/receive GET request data.
{
    digitalWrite(externalLight, LOW);
    m_displayPanel.fillScreen(LOW); // show black
    Serial.println();

    if (displayOn)
    {
        // only pull the weather data if display is on
        if (firstEpoch != 0)
        {
            centerPrint(hourMinutes(true), true);
        }
        else
        {
            centerPrint("...");
        }
        m_displayPanel.drawPixel(0, 7, HIGH);
        m_displayPanel.drawPixel(0, 6, HIGH);
        m_displayPanel.drawPixel(0, 5, HIGH);
        m_displayPanel.write();

        m_openWeatherMapClient.updateWeather();
        if (m_openWeatherMapClient.getError() != "")
        {
            scrollMessage(m_openWeatherMapClient.getError());
        }
    }

    Serial.println("Updating Time...");
    // Update the Time
    m_displayPanel.drawPixel(0, 4, HIGH);
    m_displayPanel.drawPixel(0, 3, HIGH);
    m_displayPanel.drawPixel(0, 2, HIGH);
    Serial.println("matrix Width:" + m_displayPanel.width());
    m_displayPanel.write();
    m_timeDB.updateConfig(TIMEDBKEY, m_openWeatherMapClient.getLat(0), m_openWeatherMapClient.getLon(0));
    time_t currentTime = m_timeDB.getTime();
    if (currentTime > 5000 || firstEpoch == 0)
    {
        setTime(currentTime);
    }
    else
    {
        Serial.println("Time update unsuccessful!");
    }
    lastEpoch = now();
    if (firstEpoch == 0)
    {
        firstEpoch = now();
        Serial.println("firstEpoch is: " + String(firstEpoch));
    }

    Serial.println("Version: " + String(VERSION));
    Serial.println();
    digitalWrite(externalLight, HIGH);
}

void displayMessage(String message)
{
    digitalWrite(externalLight, LOW);

    webServer.sendHeader("Cache-Control", "no-cache, no-store");
    webServer.sendHeader("Pragma", "no-cache");
    webServer.sendHeader("Expires", "-1");
    webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
    webServer.send(200, "text/html", "");
    sendHeader();
    webServer.sendContent(message);
    sendFooter();
    webServer.sendContent("");
    webServer.client().stop();

    digitalWrite(externalLight, HIGH);
}

void redirectHome()
{
    // Send them back to the Root Directory
    webServer.sendHeader("Location", String("/"), true);
    webServer.sendHeader("Cache-Control", "no-cache, no-store");
    webServer.sendHeader("Pragma", "no-cache");
    webServer.sendHeader("Expires", "-1");
    webServer.send(302, "text/plain", "");
    webServer.client().stop();
    delay(1000);
}

void sendHeader()
{
    String html = "<!DOCTYPE HTML>";
    html += "<html><head><title>GY Clock</title><link rel='icon' "
            "href='data:;base64,='>";
    html += "<meta http-equiv='Content-Type' content='text/html; charset=UTF-8' />";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<link rel='stylesheet' href='https://www.w3schools.com/w3css/4/w3.css'>";
    html += "<link rel='stylesheet' "
            "href='https://www.w3schools.com/lib/w3-theme-blue-grey.css'>";
    html += "<link rel='stylesheet' "
            "href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.8.1/css/"
            "all.min.css'>";
    html += "</head><body>";
    webServer.sendContent(html);
    html = "<nav class='w3-sidebar w3-bar-block w3-card' style='margin-top:88px' "
           "id='mySidebar'>";
    html += "<div class='w3-container w3-theme-d2'>";
    html += "<span onclick='closeSidebar()' class='w3-button w3-display-topright "
            "w3-large'><i class='fas fa-times'></i></span>";
    html += "<div class='w3-left'><img src='http://openweathermap.org/img/w/" + m_openWeatherMapClient.getIcon(0) +
            ".png' alt='" + m_openWeatherMapClient.getDescription(0) + "'></div>";
    html += "<div class='w3-padding'>Menu</div></div>";
    webServer.sendContent(html);

    webServer.sendContent(FPSTR(WEB_ACTIONS1));
    Serial.println("Displays: " + String(numberOfHorizontalDisplays));
    if (numberOfHorizontalDisplays >= 8)
    {
        webServer.sendContent("<a class='w3-bar-item w3-button' href='/configurewideclock'><i "
                              "class='far fa-clock'></i> Wide Clock</a>");
    }
    webServer.sendContent(FPSTR(WEB_ACTIONS2));
    if (displayOn)
    {
        webServer.sendContent("<i class='fas fa-eye-slash'></i> Turn Display OFF");
    }
    else
    {
        webServer.sendContent("<i class='fas fa-eye'></i> Turn Display ON");
    }
    webServer.sendContent(FPSTR(WEB_ACTION3));

    html = "</nav>";
    html += "<header class='w3-top w3-bar w3-theme'><button class='w3-bar-item "
            "w3-button w3-xxxlarge w3-hover-theme' onclick='openSidebar()'><i "
            "class='fas fa-bars'></i></button><h2 class='w3-bar-item'>Weather "
            "Marquee</h2></header>";
    html += "<script>";
    html += "function "
            "openSidebar(){document.getElementById('mySidebar').style.display='"
            "block'}function "
            "closeSidebar(){document.getElementById('mySidebar').style.display='"
            "none'}closeSidebar();";
    html += "</script>";
    html += "<br><div class='w3-container w3-large' style='margin-top:88px'>";
    webServer.sendContent(html);
}

void sendFooter()
{
    int8_t rssi = getWifiQuality();
    Serial.print("Signal Strength (RSSI): ");
    Serial.print(rssi);
    Serial.println("%");
    String html = "<br><br><br>";
    html += "</div>";
    html += "<footer class='w3-container w3-bottom w3-theme w3-margin-top'>";
    html += "<i class='far fa-paper-plane'></i> Version: " + String(VERSION) + "<br>";
    html += "<i class='far fa-clock'></i> Next Update: " + getTimeTillUpdate() + "<br>";
    html += "<i class='fas fa-rss'></i> Signal Strength: ";
    html += String(rssi) + "%";
    html += "</footer>";
    html += "</body></html>";
    webServer.sendContent(html);
}

void displayWeatherData()
{
    digitalWrite(externalLight, LOW);
    String html = "";

    webServer.sendHeader("Cache-Control", "no-cache, no-store");
    webServer.sendHeader("Pragma", "no-cache");
    webServer.sendHeader("Expires", "-1");
    webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
    webServer.send(200, "text/html", "");
    sendHeader();

    String temperature = m_openWeatherMapClient.getTemp(0);

    if ((temperature.indexOf(".") != -1) && (temperature.length() >= (unsigned)(temperature.indexOf(".") + 2)))
    {
        temperature.remove(temperature.indexOf(".") + 2);
    }

    String time = m_timeDB.getDayName() + ", " + m_timeDB.getMonthName() + " " + day() + ", " + hourFormat12() + ":" +
                  m_timeDB.zeroPad(minute()) + " " + m_timeDB.getAmPm();

    Serial.println(m_openWeatherMapClient.getCity(0));
    Serial.println(m_openWeatherMapClient.getCondition(0));
    Serial.println(m_openWeatherMapClient.getDescription(0));
    Serial.println(temperature);
    Serial.println(time);

    if (TIMEDBKEY == "")
    {
        html += "<p>Please <a href='/configure'>Configure TimeZoneDB</a> with API "
                "key.</p>";
    }

    if (m_openWeatherMapClient.getCity(0) == "")
    {
        html += "<p>Please <a href='/configure'>Configure Weather</a> API</p>";
        if (m_openWeatherMapClient.getError() != "")
        {
            html += "<p>Weather Error: <strong>" + m_openWeatherMapClient.getError() + "</strong></p>";
        }
    }
    else
    {
        html += "<div class='w3-cell-row' style='width:100%'><h2>" + m_openWeatherMapClient.getCity(0) + ", " +
                m_openWeatherMapClient.getCountry(0) + "</h2></div><div class='w3-cell-row'>";
        html += "<div class='w3-cell w3-left w3-medium' style='width:120px'>";
        html += "<img src='http://openweathermap.org/img/w/" + m_openWeatherMapClient.getIcon(0) + ".png' alt='" +
                m_openWeatherMapClient.getDescription(0) + "'><br>";
        html += m_openWeatherMapClient.getHumidity(0) + "% Humidity<br>";
        html += m_openWeatherMapClient.getDirectionText(0) + " / " + m_openWeatherMapClient.getWind(0) +
                " <span class='w3-tiny'>" + getSpeedSymbol() + "</span> Wind<br>";
        html += m_openWeatherMapClient.getPressure(0) + " Pressure<br>";
        html += "</div>";
        html += "<div class='w3-cell w3-container' style='width:100%'><p>";
        html += m_openWeatherMapClient.getCondition(0) + " (" + m_openWeatherMapClient.getDescription(0) + ")<br>";
        html += temperature + " " + getTempSymbol() + "<br>";
        html +=
            m_openWeatherMapClient.getHigh(0) + "/" + m_openWeatherMapClient.getLow(0) + " " + getTempSymbol() + "<br>";
        html += time + "<br>";
        html += "<a href='https://www.google.com/maps/@" + m_openWeatherMapClient.getLat(0) + "," +
                m_openWeatherMapClient.getLon(0) +
                ",10000m/data=!3m1!1e3' target='_BLANK'><i class='fas "
                "fa-map-marker' style='color:red'></i> Map It!</a><br>";
        html += "</p></div></div><hr>";
    }

    webServer.sendContent(String(html)); // spit out what we got
    html = "";                           // fresh start

    sendFooter();
    webServer.sendContent("");
    webServer.client().stop();
    digitalWrite(externalLight, HIGH);
}

void configModeCallback(WiFiManager *myWiFiManager)
{
    Serial.println("Entered config mode");
    Serial.println(WiFi.softAPIP());
    Serial.println("Wifi Manager");
    Serial.println("Please connect to AP");
    Serial.println(myWiFiManager->getConfigPortalSSID());
    Serial.println("To setup Wifi Configuration");
    scrollMessage("Please Connect to AP: " + String(myWiFiManager->getConfigPortalSSID()));
    centerPrint("wifi");
}

void flashLED(int number, int delayTime)
{
    for (int inx = 0; inx < number; inx++)
    {
        tone(BUZZER_PIN, 440, delayTime);
        delay(delayTime);
        digitalWrite(externalLight, LOW);
        delay(delayTime);
        digitalWrite(externalLight, HIGH);
        delay(delayTime);
    }
    noTone(BUZZER_PIN);
}

String getTempSymbol()
{
    String rtnValue = "F";
    if (IS_METRIC)
    {
        rtnValue = "C";
    }
    return rtnValue;
}

String getSpeedSymbol()
{
    String rtnValue = "mph";
    if (IS_METRIC)
    {
        rtnValue = "kph";
    }
    return rtnValue;
}

String getPressureSymbol()
{
    String rtnValue = "";
    if (IS_METRIC)
    {
        rtnValue = "mb";
    }
    return rtnValue;
}

// converts the dBm to a range between 0 and 100%
int8_t getWifiQuality()
{
    int32_t dbm = WiFi.RSSI();
    if (dbm <= -100)
    {
        return 0;
    }
    else if (dbm >= -50)
    {
        return 100;
    }
    else
    {
        return 2 * (dbm + 100);
    }
}

String getTimeTillUpdate()
{
    String rtnValue = "";

    long timeToUpdate = (((minutesBetweenDataRefresh * 60) + lastEpoch) - now());

    int hours = numberOfHours(timeToUpdate);
    int minutes = numberOfMinutes(timeToUpdate);
    int seconds = numberOfSeconds(timeToUpdate);

    rtnValue += String(hours) + ":";
    if (minutes < 10)
    {
        rtnValue += "0";
    }
    rtnValue += String(minutes) + ":";
    if (seconds < 10)
    {
        rtnValue += "0";
    }
    rtnValue += String(seconds);

    return rtnValue;
}

int getMinutesFromLastRefresh()
{
    int minutes = (now() - lastEpoch) / 60;
    return minutes;
}

int getMinutesFromLastDisplay()
{
    int minutes = (now() - displayOffEpoch) / 60;
    return minutes;
}

void enableDisplay(boolean enable)
{
    displayOn = enable;
    if (enable)
    {
        if (getMinutesFromLastDisplay() >= minutesBetweenDataRefresh)
        {
            // The display has been off longer than the minutes between refresh --
            // need to get fresh data
            lastEpoch = 0;       // this should force a data pull of the weather
            displayOffEpoch = 0; // reset
        }
        m_displayPanel.shutdown(false);
        m_displayPanel.fillScreen(LOW); // show black
        Serial.println("Display was turned ON: " + now());
    }
    else
    {
        m_displayPanel.shutdown(true);
        Serial.println("Display was turned OFF: " + now());
        displayOffEpoch = lastEpoch;
    }
}

// Toggle on and off the display if user defined times
void checkDisplay()
{
    if (timeDisplayTurnsOn == "" || timeDisplayTurnsOff == "")
    {
        return; // nothing to do
    }
    String currentTime = m_timeDB.zeroPad(hour()) + ":" + m_timeDB.zeroPad(minute());

    if (currentTime == timeDisplayTurnsOn && !displayOn)
    {
        Serial.println("Time to turn display on: " + currentTime);
        flashLED(1, 500);
        enableDisplay(true);
    }

    if (currentTime == timeDisplayTurnsOff && displayOn)
    {
        Serial.println("Time to turn display off: " + currentTime);
        flashLED(2, 500);
        enableDisplay(false);
    }
}

String writeSettings()
{
    // Save decoded message to SPIFFS file for playback on power up.
    File f = LittleFS.open(CONFIG, "w");
    if (!f)
    {
        Serial.println("File open failed!");
    }
    else
    {
        Serial.println("Saving settings now...");
        f.println("TIMEDBKEY=" + TIMEDBKEY);
        f.println("APIKEY=" + APIKEY);
        f.println("CityID=" + String(CityIDs[0]));
        f.println("marqueeMessage=" + marqueeMessage);
        f.println("timeDisplayTurnsOn=" + timeDisplayTurnsOn);
        f.println("timeDisplayTurnsOff=" + timeDisplayTurnsOff);
        f.println("ledIntensity=" + String(displayIntensity));
        f.println("scrollSpeed=" + String(displayScrollSpeed));
        f.println("isFlash=" + String(flashOnSeconds));
        f.println("is24hour=" + String(IS_24HOUR));
        f.println("isPM=" + String(IS_PM));
        f.println("wideclockformat=" + Wide_Clock_Style);
        f.println("isMetric=" + String(IS_METRIC));
        f.println("refreshRate=" + String(minutesBetweenDataRefresh));
        f.println("secondsBetweenScrolling=" + String(secondsBetweenScrolling));
        f.println("www_username=" + String(www_username));
        f.println("www_password=" + String(www_password));
        f.println("IS_BASIC_AUTH=" + String(IS_BASIC_AUTH));
        f.println("SHOW_CITY=" + String(SHOW_CITY));
        f.println("SHOW_CONDITION=" + String(SHOW_CONDITION));
        f.println("SHOW_HUMIDITY=" + String(SHOW_HUMIDITY));
        f.println("SHOW_WIND=" + String(SHOW_WIND));
        f.println("SHOW_PRESSURE=" + String(SHOW_PRESSURE));
        f.println("SHOW_HIGHLOW=" + String(SHOW_HIGHLOW));
        f.println("SHOW_DATE=" + String(SHOW_DATE));
    }
    f.close();
    readSettings();
    m_openWeatherMapClient.updateCityIdList(CityIDs, 1);
    String cityIds = m_openWeatherMapClient.getMyCityIDs();
    return cityIds;
}

void readSettings()
{
    if (LittleFS.exists(CONFIG) == false)
    {
        Serial.println("Settings File does not yet exists.");
        writeSettings();
        return;
    }
    File fr = LittleFS.open(CONFIG, "r");
    String line;
    while (fr.available())
    {
        line = fr.readStringUntil('\n');
        if (line.indexOf("TIMEDBKEY=") >= 0)
        {
            TIMEDBKEY = line.substring(line.lastIndexOf("TIMEDBKEY=") + 10);
            TIMEDBKEY.trim();
            Serial.println("TIMEDBKEY: " + TIMEDBKEY);
        }
        if (line.indexOf("APIKEY=") >= 0)
        {
            APIKEY = line.substring(line.lastIndexOf("APIKEY=") + 7);
            APIKEY.trim();
            Serial.println("APIKEY: " + APIKEY);
        }
        if (line.indexOf("CityID=") >= 0)
        {
            CityIDs[0] = line.substring(line.lastIndexOf("CityID=") + 7).toInt();
            Serial.println("CityID: " + String(CityIDs[0]));
        }
        if (line.indexOf("isFlash=") >= 0)
        {
            flashOnSeconds = line.substring(line.lastIndexOf("isFlash=") + 8).toInt();
            Serial.println("flashOnSeconds=" + String(flashOnSeconds));
        }
        if (line.indexOf("is24hour=") >= 0)
        {
            IS_24HOUR = line.substring(line.lastIndexOf("is24hour=") + 9).toInt();
            Serial.println("IS_24HOUR=" + String(IS_24HOUR));
        }
        if (line.indexOf("isPM=") >= 0)
        {
            IS_PM = line.substring(line.lastIndexOf("isPM=") + 5).toInt();
            Serial.println("IS_PM=" + String(IS_PM));
        }
        if (line.indexOf("wideclockformat=") >= 0)
        {
            Wide_Clock_Style = line.substring(line.lastIndexOf("wideclockformat=") + 16);
            Wide_Clock_Style.trim();
            Serial.println("Wide_Clock_Style=" + Wide_Clock_Style);
        }
        if (line.indexOf("isMetric=") >= 0)
        {
            IS_METRIC = line.substring(line.lastIndexOf("isMetric=") + 9).toInt();
            Serial.println("IS_METRIC=" + String(IS_METRIC));
        }
        if (line.indexOf("refreshRate=") >= 0)
        {
            minutesBetweenDataRefresh = line.substring(line.lastIndexOf("refreshRate=") + 12).toInt();
            if (minutesBetweenDataRefresh == 0)
            {
                minutesBetweenDataRefresh = 15; // can't be zero
            }
            Serial.println("minutesBetweenDataRefresh=" + String(minutesBetweenDataRefresh));
        }
        if (line.indexOf("secondsBetweenScrolling=") >= 0)
        {
            displayRefreshCount = 1;
            secondsBetweenScrolling = line.substring(line.lastIndexOf("secondsBetweenScrolling=") + 24).toInt();
            Serial.println("secondsBetweenScrolling=" + String(secondsBetweenScrolling));
        }
        if (line.indexOf("marqueeMessage=") >= 0)
        {
            marqueeMessage = line.substring(line.lastIndexOf("marqueeMessage=") + 15);
            marqueeMessage.trim();
            Serial.println("marqueeMessage=" + marqueeMessage);
        }
        if (line.indexOf("timeDisplayTurnsOn=") >= 0)
        {
            timeDisplayTurnsOn = line.substring(line.lastIndexOf("timeDisplayTurnsOn=") + 19);
            timeDisplayTurnsOn.trim();
            Serial.println("timeDisplayTurnsOn=" + timeDisplayTurnsOn);
        }
        if (line.indexOf("timeDisplayTurnsOff=") >= 0)
        {
            timeDisplayTurnsOff = line.substring(line.lastIndexOf("timeDisplayTurnsOff=") + 20);
            timeDisplayTurnsOff.trim();
            Serial.println("timeDisplayTurnsOff=" + timeDisplayTurnsOff);
        }
        if (line.indexOf("ledIntensity=") >= 0)
        {
            displayIntensity = line.substring(line.lastIndexOf("ledIntensity=") + 13).toInt();
            Serial.println("displayIntensity=" + String(displayIntensity));
        }
        if (line.indexOf("scrollSpeed=") >= 0)
        {
            displayScrollSpeed = line.substring(line.lastIndexOf("scrollSpeed=") + 12).toInt();
            Serial.println("displayScrollSpeed=" + String(displayScrollSpeed));
        }
        if (line.indexOf("www_username=") >= 0)
        {
            String temp = line.substring(line.lastIndexOf("www_username=") + 13);
            temp.trim();
            temp.toCharArray(www_username, sizeof(temp));
            Serial.println("www_username=" + String(www_username));
        }
        if (line.indexOf("www_password=") >= 0)
        {
            String temp = line.substring(line.lastIndexOf("www_password=") + 13);
            temp.trim();
            temp.toCharArray(www_password, sizeof(temp));
            Serial.println("www_password=" + String(www_password));
        }
        if (line.indexOf("IS_BASIC_AUTH=") >= 0)
        {
            IS_BASIC_AUTH = line.substring(line.lastIndexOf("IS_BASIC_AUTH=") + 14).toInt();
            Serial.println("IS_BASIC_AUTH=" + String(IS_BASIC_AUTH));
        }
        if (line.indexOf("SHOW_CITY=") >= 0)
        {
            SHOW_CITY = line.substring(line.lastIndexOf("SHOW_CITY=") + 10).toInt();
            Serial.println("SHOW_CITY=" + String(SHOW_CITY));
        }
        if (line.indexOf("SHOW_CONDITION=") >= 0)
        {
            SHOW_CONDITION = line.substring(line.lastIndexOf("SHOW_CONDITION=") + 15).toInt();
            Serial.println("SHOW_CONDITION=" + String(SHOW_CONDITION));
        }
        if (line.indexOf("SHOW_HUMIDITY=") >= 0)
        {
            SHOW_HUMIDITY = line.substring(line.lastIndexOf("SHOW_HUMIDITY=") + 14).toInt();
            Serial.println("SHOW_HUMIDITY=" + String(SHOW_HUMIDITY));
        }
        if (line.indexOf("SHOW_WIND=") >= 0)
        {
            SHOW_WIND = line.substring(line.lastIndexOf("SHOW_WIND=") + 10).toInt();
            Serial.println("SHOW_WIND=" + String(SHOW_WIND));
        }
        if (line.indexOf("SHOW_PRESSURE=") >= 0)
        {
            SHOW_PRESSURE = line.substring(line.lastIndexOf("SHOW_PRESSURE=") + 14).toInt();
            Serial.println("SHOW_PRESSURE=" + String(SHOW_PRESSURE));
        }

        if (line.indexOf("SHOW_HIGHLOW=") >= 0)
        {
            SHOW_HIGHLOW = line.substring(line.lastIndexOf("SHOW_HIGHLOW=") + 13).toInt();
            Serial.println("SHOW_HIGHLOW=" + String(SHOW_HIGHLOW));
        }

        if (line.indexOf("SHOW_DATE=") >= 0)
        {
            SHOW_DATE = line.substring(line.lastIndexOf("SHOW_DATE=") + 10).toInt();
            Serial.println("SHOW_DATE=" + String(SHOW_DATE));
        }
    }
    fr.close();
    m_displayPanel.setIntensity(displayIntensity);
    m_openWeatherMapClient.updateWeatherApiKey(APIKEY);
    m_openWeatherMapClient.setMetric(IS_METRIC);
    m_openWeatherMapClient.updateCityIdList(CityIDs, 1);
}

void scrollMessage(String msg)
{
    msg += " "; // add a space at the end
    for (unsigned int i = 0; i < width * msg.length() + m_displayPanel.width() - 1 - spacer; i++)
    {
        if (WEBSERVER_ENABLED)
        {
            webServer.handleClient();
        }
        if (ENABLE_OTA)
        {
            ArduinoOTA.handle();
        }
        if (refresh == 1)
            i = 0;
        refresh = 0;
        m_displayPanel.fillScreen(LOW);

        unsigned int letter = i / width;
        int x = (m_displayPanel.width() - 1) - i % width;
        int y = (m_displayPanel.height() - 8) / 2; // center the text vertically

        while (x + width - spacer >= 0 && letter >= 0)
        {
            if (letter < msg.length())
            {
                m_displayPanel.drawChar(x, y, msg[letter], HIGH, LOW, 1);
            }

            letter--;
            x -= width;
        }

        m_displayPanel.write(); // Send bitmap to display
        delay(displayScrollSpeed);
    }
    m_displayPanel.setCursor(0, 0);
}

void centerPrint(String msg)
{
    centerPrint(msg, false);
}

void centerPrint(String msg, boolean extraStuff)
{
    int x = (m_displayPanel.width() - (msg.length() * width)) / 2;

    // Print the static portions of the display before the main Message
    if (extraStuff)
    {
        if (!IS_24HOUR && IS_PM && isPM())
        {
            m_displayPanel.drawPixel(m_displayPanel.width() - 1, 6, HIGH);
        }
    }

    m_displayPanel.setCursor(x, 0);
    m_displayPanel.print(msg);

    m_displayPanel.write();
}

String decodeHtmlString(String msg)
{
    String decodedMsg = msg;
    // Restore special characters that are misformed to %char by the client
    // browser
    decodedMsg.replace("+", " ");
    decodedMsg.replace("%21", "!");
    decodedMsg.replace("%22", "");
    decodedMsg.replace("%23", "#");
    decodedMsg.replace("%24", "$");
    decodedMsg.replace("%25", "%");
    decodedMsg.replace("%26", "&");
    decodedMsg.replace("%27", "'");
    decodedMsg.replace("%28", "(");
    decodedMsg.replace("%29", ")");
    decodedMsg.replace("%2A", "*");
    decodedMsg.replace("%2B", "+");
    decodedMsg.replace("%2C", ",");
    decodedMsg.replace("%2F", "/");
    decodedMsg.replace("%3A", ":");
    decodedMsg.replace("%3B", ";");
    decodedMsg.replace("%3C", "<");
    decodedMsg.replace("%3D", "=");
    decodedMsg.replace("%3E", ">");
    decodedMsg.replace("%3F", "?");
    decodedMsg.replace("%40", "@");
    decodedMsg.toUpperCase();
    decodedMsg.trim();
    return decodedMsg;
}
