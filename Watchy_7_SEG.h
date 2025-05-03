#ifndef WATCHY_7_SEG_H
#define WATCHY_7_SEG_H

#include <Watchy.h>
#include "Seven_Segment10pt7b.h"
#include "DSEG7_Classic_Regular_15.h"
#include "DSEG7_Classic_Bold_25.h"
#include "DSEG7_Classic_Regular_39.h"
#include "icons.h"
#include "settings.h"

class Watchy7SEG : public Watchy{
    public:
        extendedWatchySettings settings;

        void drawTime();
        void drawDate();
        void drawSteps(bool darkMode);
        void drawWeather(bool darkMode);
        void drawBattery(bool darkMode);
        weatherData getYandexWeather();
        weatherData getOpenWeather(String cityID, String lat, String lon, String units, String lang,
          String url, String apiKey,
          uint8_t updateInterval);
        weatherData getWeather();
        virtual void drawWatchFace() override;
        virtual void handleButtonPress() override;
        
        Watchy7SEG(extendedWatchySettings &s): settings(s), Watchy(s) {}
};

#endif