#include "Watchy_7_SEG.h"

RTC_DATA_ATTR bool darkMode = true;
RTC_DATA_ATTR weatherInfo weather;
RTC_DATA_ATTR tmElements_t lastWeatherCheck;
RTC_DATA_ATTR bool wasWeatherChecked = false;
RTC_DATA_ATTR String debugError;

const uint8_t BATTERY_SEGMENTS_WIDTH = 30;
const uint8_t BATTERY_SEGMENT_WIDTH = 2;
const uint8_t BATTERY_SEGMENT_HEIGHT = 11;
const uint8_t BATTERY_SEGMENT_SPACING = 3;
const uint8_t WEATHER_ICON_WIDTH = 48;
const uint8_t WEATHER_ICON_HEIGHT = 32;
const uint8_t WEATHER_PROVIDER_ICON_SIZE = 15;
const float APPROXIMATE_MAXIMUM_VOLTAGE = 4.245;
const float APPROXIMATE_MINIMUM_VOLTAGE = 2.8;

void Watchy7SEG::handleButtonPress() {
  Watchy::handleButtonPress();
  uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();

  if (wakeupBit & BACK_BTN_MASK) {
    if (guiState == WATCHFACE_STATE) {
      darkMode = !darkMode;
      RTC.read(currentTime);
      showWatchFace(false);
    }
  }
}

void Watchy7SEG::drawWatchFace() {
    display.fillScreen(darkMode ? GxEPD_BLACK : GxEPD_WHITE);
    display.setTextColor(darkMode ? GxEPD_WHITE : GxEPD_BLACK);
    drawTime();
    drawDate();
    drawSteps(darkMode);
    drawWeather(darkMode);
    drawBattery(darkMode);
    drawDebugError(darkMode);
    display.drawBitmap(116, 75, WIFI_CONFIGURED ? wifi : wifioff, 26, 18, darkMode ? GxEPD_WHITE : GxEPD_BLACK);
    if(BLE_CONFIGURED){
        display.drawBitmap(100, 73, bluetooth, 13, 21, darkMode ? GxEPD_WHITE : GxEPD_BLACK);
    }
    #ifdef ARDUINO_ESP32S3_DEV
    if(USB_PLUGGED_IN){
      display.drawBitmap(140, 75, charge, 16, 18, darkMode ? GxEPD_WHITE : GxEPD_BLACK);
    }
    #endif
}

void Watchy7SEG::drawTime(){
    display.setFont(&DSEG7_Classic_Bold_53);
    display.setCursor(5, 53+5);
    int displayHour;
    if(HOUR_12_24==12){
      displayHour = ((currentTime.Hour+11)%12)+1;
    } else {
      displayHour = currentTime.Hour;
    }
    if(displayHour < 10){
        display.print("0");
    }
    display.print(displayHour);
    display.print(":");
    if(currentTime.Minute < 10){
        display.print("0");
    }
    display.println(currentTime.Minute);
}

void Watchy7SEG::drawDate(){
    display.setFont(&Seven_Segment10pt7b);

    int16_t  x1, y1;
    uint16_t w, h;

    String dayOfWeek = dayStr(currentTime.Wday);
    display.getTextBounds(dayOfWeek, 5, 85, &x1, &y1, &w, &h);
    if(currentTime.Wday == 4){
        w = w - 5;
    }
    display.setCursor(85 - w, 85);
    display.println(dayOfWeek);

    String month = monthShortStr(currentTime.Month);
    display.getTextBounds(month, 60, 110, &x1, &y1, &w, &h);
    display.setCursor(85 - w, 110);
    display.println(month);

    display.setFont(&DSEG7_Classic_Bold_25);
    display.setCursor(5, 120);
    if(currentTime.Day < 10){
    display.print("0");
    }
    display.println(currentTime.Day);
    display.setCursor(5, 150);
    display.println(tmYearToCalendar(currentTime.Year));// offset from 1970, since year is stored in uint8_t
}
void Watchy7SEG::drawSteps(bool darkMode){
    // reset step counter at midnight
    if (currentTime.Hour == 0 && currentTime.Minute == 0){
      sensor.resetStepCounter();
    }
    uint32_t stepCount = sensor.getCounter();
    display.drawBitmap(10, 165, steps, 19, 23, darkMode ? GxEPD_WHITE : GxEPD_BLACK);
    display.setCursor(35, 190);
    display.println(stepCount);
}
void Watchy7SEG::drawBattery(bool darkMode){
    display.drawBitmap(155, 73, battery, 40, 21, darkMode ? GxEPD_WHITE : GxEPD_BLACK);
    display.fillRect(160, 78, BATTERY_SEGMENTS_WIDTH, BATTERY_SEGMENT_HEIGHT, darkMode ? GxEPD_BLACK : GxEPD_WHITE);//clear battery segments
    float VBAT = getBatteryVoltage();
    float maxVoltageDelta = APPROXIMATE_MAXIMUM_VOLTAGE - APPROXIMATE_MINIMUM_VOLTAGE;
    float voltageDelta = maxVoltageDelta - (APPROXIMATE_MAXIMUM_VOLTAGE - VBAT);
    int8_t maxSegmentsAmount = BATTERY_SEGMENTS_WIDTH / BATTERY_SEGMENT_SPACING;
    float batteryPercent = voltageDelta / maxVoltageDelta;

    if (voltageDelta > maxVoltageDelta) {
      batteryPercent = 1;
    }

    int8_t segmentsAmount = batteryPercent * maxSegmentsAmount;

    for(int8_t batterySegment = 0; batterySegment < segmentsAmount; batterySegment++){
        display.fillRect(160 + (batterySegment * BATTERY_SEGMENT_SPACING), 78, BATTERY_SEGMENT_WIDTH, BATTERY_SEGMENT_HEIGHT, darkMode ? GxEPD_WHITE : GxEPD_BLACK);
    }
}

weatherInfo Watchy7SEG::getOpenWeather(String cityID, String lat, String lon, String units, String lang, String url, String apiKey, uint8_t updateInterval) {
  HTTPClient http; // Use Weather API for live data if WiFi is connected
  String weatherQueryURL = url;

  http.setConnectTimeout(3000); // 3 second max timeout

  if(cityID != ""){
    weatherQueryURL.replace("{cityID}", cityID);
  }else{
    weatherQueryURL.replace("{lat}", lat);
    weatherQueryURL.replace("{lon}", lon);
  }
  weatherQueryURL.replace("{units}", units);
  weatherQueryURL.replace("{lang}", lang);
  weatherQueryURL.replace("{apiKey}", apiKey);
  http.begin(weatherQueryURL.c_str());
  int httpResponseCode = http.GET();
  if (httpResponseCode == 200) {
    String payload             = http.getString();
    JSONVar responseObject     = JSON.parse(payload);
    weather.temperature = int(responseObject["main"]["temp"]);
    weather.weatherConditionCode = int(responseObject["weather"][0]["id"]);
    weather.weatherDescription = JSONVar::stringify(responseObject["weather"][0]["main"]);
    weather.external = true;
    breakTime((time_t)(int)responseObject["sys"]["sunrise"], weather.sunrise);
    breakTime((time_t)(int)responseObject["sys"]["sunset"], weather.sunset);
  } else {
    throw 1;
  }
  http.end();
  return weather;
}

weatherInfo Watchy7SEG::getYandexWeather(String url, String apiKey, String lat, String lon) {
  HTTPClient http;
  String weatherQueryURL = url;
  
  http.setConnectTimeout(3000);
  weatherQueryURL.replace("{lat}", lat);
  weatherQueryURL.replace("{lon}", lon);
  http.begin(weatherQueryURL.c_str());
  http.addHeader("X-Yandex-Weather-Key", apiKey);
  int httpResponseCode = http.GET();

  if (httpResponseCode == 200) {
    String payload             = http.getString();
    JSONVar responseObject     = JSON.parse(payload);
    weather.temperature = int(responseObject["fact"]["temp"]);
    weather.external = true;
    weather.weatherConditionCode = 800;
    String condition = responseObject["fact"]["condition"];

    if(condition == "cloudy" || condition == "overcast"){//Cloudy
      weather.weatherConditionCode = 802;
    } else if(condition == "partly-cloudy"){//Few Clouds
      weather.weatherConditionCode = 801;
    } else if(condition == "clear"){//Clear
      weather.weatherConditionCode = 800;
    } else if(condition == "dont-know"){ //!TODO Atmosphere 
      weather.weatherConditionCode = 700;
    } else if(condition == "light-snow" || condition == "snow" || condition == "snow-showers" || condition == "wet-snow"){//Snow
      weather.weatherConditionCode = 600;
    } else if(condition == "rain" || condition == "heavy-rain" || condition == "showers"){//Rain
      weather.weatherConditionCode = 500;
    } else if(condition == "hail"){ //!TODO hail 
      weather.weatherConditionCode = 500;
    } else if(condition == "light-rain"){//Drizzle
      weather.weatherConditionCode = 300;
    } else if(condition == "thunderstorm" || condition == "thunderstorm-with-rain" || condition == "thunderstorm-with-hail"){//Thunderstorm
      weather.weatherConditionCode = 200;
    }
  } else {
    throw 1;
  }

  http.end();

  return weather;
}

uint16_t getDayMinutes(tmElements_t &time) {
  return time.Hour * 60 + time.Minute;
}

weatherInfo Watchy7SEG::getWeather() {
    uint16_t elapsedTime = 0;

    if (wasWeatherChecked) {
      elapsedTime = getDayMinutes(currentTime) - getDayMinutes(lastWeatherCheck);
    }

    if (!wasWeatherChecked || elapsedTime >= settings.weatherUpdateInterval) {
      lastWeatherCheck = currentTime;
      wasWeatherChecked = true;
      weather.isMetric = settings.weatherUnit == String("metric");

      if (connectWiFi()) {
        for (int i = 0; i < 2; i++) {
          try {
            switch (settings.weatherProviders[i]) {
              case WeatherProvider::OpenWeatherMap:
                getOpenWeather(settings.cityID, settings.lat, settings.lon, settings.weatherUnit, settings.weatherLang, settings.weatherURL, settings.weatherAPIKey, settings.weatherUpdateInterval);
                weather.provider = WeatherProvider::OpenWeatherMap;
                break;
              case WeatherProvider::Yandex:
                getYandexWeather(settings.yandexWeather.url, settings.yandexWeather.weatherAPIKey, settings.yandexWeather.lat, settings.yandexWeather.lon);
                weather.provider = WeatherProvider::Yandex;
                break;
            }
            break;
          } catch(int e) {
            continue;
          }
        }

        // turn off radios
        WiFi.mode(WIFI_OFF);
        btStop();
      } else { // No WiFi, use internal temperature sensor
        uint8_t temperature = sensor.readTemperature(); // celsius
    
        if (!weather.isMetric) {
          temperature = temperature * 9. / 5. + 32.; // fahrenheit
        }
    
        weather.temperature          = temperature;
        weather.weatherConditionCode = 800;
        weather.external             = false;
      }
    }

    return weather;
}


void Watchy7SEG::drawWeather(bool darkMode){
    weatherInfo weather = getWeather();

    int8_t temperature = weather.temperature;
    int16_t weatherConditionCode = weather.weatherConditionCode;

    display.setFont(&DSEG7_Classic_Regular_39);
    int16_t  x1, y1;
    uint16_t w, h;
    display.getTextBounds(String(temperature), 0, 0, &x1, &y1, &w, &h);
    if(159 - w - x1 > 87){
        display.setCursor(159 - w - x1, 150);
    }else{
        display.setFont(&DSEG7_Classic_Bold_25);
        display.getTextBounds(String(temperature), 0, 0, &x1, &y1, &w, &h);
        display.setCursor(159 - w - x1, 136);
    }
    display.println(temperature);
    display.drawBitmap(165, 110, weather.isMetric ? celsius : fahrenheit, 26, 20, darkMode ? GxEPD_WHITE : GxEPD_BLACK);
    const unsigned char* weatherIcon;

    if(WIFI_CONFIGURED && weather.external){
      //https://openweathermap.org/weather-conditions
      if(weatherConditionCode > 801){//Cloudy
        weatherIcon = cloudy;
      }else if(weatherConditionCode == 801){//Few Clouds
        weatherIcon = cloudsun;
      }else if(weatherConditionCode == 800){//Clear
        weatherIcon = sunny;
      }else if(weatherConditionCode >=700){//Atmosphere
        weatherIcon = atmosphere;
      }else if(weatherConditionCode >=600){//Snow
        weatherIcon = snow;
      }else if(weatherConditionCode >=500){//Rain
        weatherIcon = rain;
      }else if(weatherConditionCode >=300){//Drizzle
        weatherIcon = drizzle;
      }else if(weatherConditionCode >=200){//Thunderstorm
        weatherIcon = thunderstorm;
      }else 
      return;
    }else{
      weatherIcon = chip;
    }
    
    display.drawBitmap(145, 158, weatherIcon, WEATHER_ICON_WIDTH, WEATHER_ICON_HEIGHT, darkMode ? GxEPD_WHITE : GxEPD_BLACK);

    if (!weather.external) {
      return;
    }
    
    display.drawBitmap(175, 140, weather.provider == WeatherProvider::Yandex  ? yandexIcon : openWeatherIcon, WEATHER_PROVIDER_ICON_SIZE, WEATHER_PROVIDER_ICON_SIZE, darkMode ? GxEPD_WHITE : GxEPD_BLACK);
}

void Watchy7SEG::drawDebugError(bool darkMode) {
  if (!debugError) {
    return;
  }

  int x1 = 0, y1 = 40;
  int16_t  x, y;
  uint16_t w, h;

  display.setFont(&Seven_Segment10pt7b);
  display.setCursor(x1, y1);
  display.getTextBounds(String(debugError), x1, y1, &x, &y, &w, &h);
  display.fillRect(x1, y1 - h + 100, w + 100 , h, darkMode ? GxEPD_BLACK : GxEPD_WHITE);
  display.println(debugError);
}