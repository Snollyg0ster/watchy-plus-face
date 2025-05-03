#include "Watchy_7_SEG.h"

#ifndef SETTINGS_H
#define SETTINGS_H

// Open Weather Map settings

//Weather Settings
#define CITY_ID "1496747" //New York City https://openweathermap.org/current#cityid

//You can also use LAT,LON for your location instead of CITY_ID, but not both
//#define LAT "40.7127" //New York City, Looked up on https://www.latlong.net/
//#define LON "-74.0059"

#ifdef CITY_ID
    #define OPENWEATHERMAP_URL "http://api.openweathermap.org/data/2.5/weather?id={cityID}&lang={lang}&units={units}&appid={apiKey}" //open weather api using city ID
#else
    #define OPENWEATHERMAP_URL "http://api.openweathermap.org/data/2.5/weather?lat={lat}&lon={lon}&lang={lang}&units={units}&appid={apiKey}" //open weather api using lat lon
#endif

#define OPENWEATHERMAP_APIKEY "7d949cbe853ef083df5a7a7abd08ea0e" //use your own API key :)
#define TEMP_UNIT "metric" //metric = Celsius , imperial = Fahrenheit
#define TEMP_LANG "ru"
#define WEATHER_UPDATE_INTERVAL 2 //must be greater than 5, measured in minutes
//NTP Settings
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 3600 * 7 //New York is UTC -5 EST, -4 EDT, will be overwritten by weather data


// Yandex Wather settings

#define YANDEX_WEATHER_API_KEY "16d58385-254a-46b2-80d2-725eddac83ec"
#define YANDEX_LAT "55.008354" //Novosibirsk, Looked up on https://www.latlong.net/
#define YANDEX_LON "82.935730"

enum class WeatherProvider {Yandex, OpenWeatherMap};

typedef struct yandexWeatherSettings {
  String weatherAPIKey;
  String lat;
  String lon;
} OpenWeatherSettings;

typedef struct extendedWatchySettings : watchySettings {
  WeatherProvider defaultWeatherProvider;
  yandexWeatherSettings yandexWeather;
} extendedWatchySettings;

#endif