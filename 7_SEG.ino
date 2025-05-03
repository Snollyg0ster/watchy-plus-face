#include "Watchy_7_SEG.h"
#include "settings.h"

void setup(){
  extendedWatchySettings settings;
  
  settings.yandexWeather = {
      .url = YANDEX_WEATHER_URL,
      .weatherAPIKey = YANDEX_WEATHER_API_KEY,
      .lat = YANDEX_LAT,
      .lon = YANDEX_LON,
  };
  
  // Open Weather Map settings
  #ifdef CITY_ID
      settings.cityID = CITY_ID;
  #else
      settings.cityID = "";
      settings.lat = LAT;
      settings.lon = LON;
  #endif
  settings.weatherAPIKey = OPENWEATHERMAP_APIKEY;
  settings.weatherURL = OPENWEATHERMAP_URL;
  settings.weatherUnit = TEMP_UNIT;
  settings.weatherLang = TEMP_LANG;
  settings.weatherUpdateInterval = WEATHER_UPDATE_INTERVAL;
  
  settings.ntpServer = NTP_SERVER;
  settings.gmtOffset = GMT_OFFSET_SEC;
  settings.vibrateOClock = true;

  Watchy7SEG watchy(settings);

  watchy.init();
}

void loop(){}



