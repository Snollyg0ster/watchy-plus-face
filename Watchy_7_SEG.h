#ifndef WATCHY_7_SEG_H
#define WATCHY_7_SEG_H

#include <Watchy.h>
#include "Seven_Segment10pt7b.h"
#include "DSEG7_Classic_Regular_15.h"
#include "DSEG7_Classic_Bold_25.h"
#include "DSEG7_Classic_Regular_39.h"
#include "icons.h"
#include "settings.h"

typedef struct weatherInfo {
  int8_t temperature;
  int16_t weatherConditionCode;
  bool isMetric;
  String weatherDescription;
  bool external;
  tmElements_t sunrise;
  tmElements_t sunset;
  WeatherProvider provider;
} weatherInfo;

class Watchy7SEG : public Watchy{
    public:
        extendedWatchySettings settings;

        void drawTime();
        void drawDate();
        void drawSteps(bool darkMode);
        void drawWeather(bool darkMode);
        void drawBattery(bool darkMode);
        void drawDebugError(bool darkMode);
        weatherInfo getYandexWeather(String url, String apiKey, String lat, String lon);
        weatherInfo getOpenWeather(String cityID, String lat, String lon, String units, String lang, String url, String apiKey, uint8_t updateInterval);
        weatherInfo getWeather();
        virtual void drawWatchFace() override;
        virtual void handleButtonPress() override;
        
        Watchy7SEG(extendedWatchySettings &s): settings(s), Watchy(s) {}
};

#endif