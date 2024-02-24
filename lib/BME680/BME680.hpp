#pragma once

#include "bsec.h"
#include "EEPROM.hpp"

class BME680
{
    public:
        BME680();
        ~BME680();

        bool init(SPIClass &spi, EEPROM *eeprom);
        
        bool requestMeasurement();

        float getIAQ();
        const char *getIAQString();
        uint8_t getIAQAccuracy();
        uint8_t getMaxIAQAccuracy();

        float getStaticIAQ();
        uint8_t getStaticIAQAccuracy();
        
        float getCO2Equivalent();
        float getBreathVOCEquivalent();

        float getPressure();
        float getPressureHPa();
        float getTemperature();
        float getHumidity();

        float getGasResistance();
        float getGasPercentage();

        float getStabStatus();
        float getRunInStatus();

        bool loadState();
        bool storeState();

        bool checkSensorStatus();
        bsec_version_t getBSECVersion();

    private:
        struct IAQLevelToStringMap
        {
            uint16_t lowerLimit;
            uint16_t upperLimit;
            const char *string;
        };

        const IAQLevelToStringMap IAQRangesToString[7] = {
            {0, 50, "Excellent"},
            {51, 100, "Good"},
            {101, 150, "Fair"},
            {151, 200, "Poor"},
            {201, 250, "Bad"},
            {251, 350, "Very bad"},
            {351, 500, "Terrible"}
        };

        static constexpr auto maxIAQAccuracy = 3;
        
        Bsec sensor;
        EEPROM *eeprom;

        void configureChipSelect();
        void updateSubscription();
        uint8_t computeStateChecksum(const uint8_t *config);
        bool isStateValid(const uint8_t *rawState);
};
