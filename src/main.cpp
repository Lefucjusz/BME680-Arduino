#include <BME680.hpp>
#include <M24C16.hpp>
#include <Adafruit_SSD1306.h>

namespace
{
    constexpr auto lcdXResolution = 128;
    constexpr auto lcdYResolution = 32;
    constexpr auto lcdI2CAddress = 0x3C;

    constexpr auto stateStoreIntervalMs = 12UL * 60UL * 60UL * 1000UL; // Every 12h

    BME680 sensor;
    M24C16 eeprom;
    Adafruit_SSD1306 lcd(lcdXResolution, lcdYResolution);

    void clearLcd() {
        lcd.clearDisplay();
        lcd.setCursor(0,0);
    }

    void initLcd() {
        lcd.begin(SSD1306_SWITCHCAPVCC, lcdI2CAddress);
        lcd.setTextSize(1);
        lcd.setTextColor(SSD1306_WHITE);
        lcd.clearDisplay();
        lcd.setCursor(0,0);
        lcd.display();
    }

    void displayMeasurements() {
        lcd.print("T:");
        lcd.print(sensor.getTemperature());
        lcd.print("C P:");
        lcd.print(sensor.getPressureHPa());
        lcd.print("hPa");

        lcd.setCursor(0, 12);
        lcd.print("IAQ:");
        lcd.print(sensor.getIAQString());
        if (sensor.getStabStatus() && sensor.getRunInStatus()) {
            lcd.print(" (");
            lcd.print(sensor.getIAQ());
            lcd.print(')');
        }

        lcd.setCursor(0, 24);
        lcd.print("RH:");
        lcd.print(sensor.getHumidity());
        lcd.print("%  ACC:");
        lcd.print(sensor.getIAQAccuracy());
        lcd.print('/');
        lcd.print(sensor.getMaxIAQAccuracy());

        // lcd.print("%  S:");
        // lcd.print(sensor.getStabStatus() ? "Y" : "N");
        // lcd.print(" R:");
        // lcd.print(sensor.getRunInStatus() ? "Y" : "N");

        lcd.display();
    }

    void updateEepromIfRequired() {
        static uint32_t lastUpdateTime = 0;
        static uint8_t lastAccuracy = 0;

        const auto currentTick = millis();

        /* Store state periodically or after reaching maximum accuracy */
        if ((currentTick - lastUpdateTime) >= stateStoreIntervalMs) {
            sensor.storeState();
            lastUpdateTime = currentTick;
        }
        else if ((sensor.getMaxIAQAccuracy() != lastAccuracy) && (sensor.getIAQAccuracy() == sensor.getMaxIAQAccuracy())) {
            sensor.storeState();
            lastAccuracy = sensor.getMaxIAQAccuracy();
            lastUpdateTime = currentTick;
        }
    }
}

void setup()
{
    initLcd();
    if (!eeprom.init()) {
        lcd.print("EEPROM init error!");
        lcd.display();
        while (true);
    }

    const auto status = sensor.init(SPI, &eeprom);
    if (!status) {
        lcd.print("Failed to init sensor!");
        while (true);
    }

    if (!sensor.loadState()) {
        lcd.print("Error loading state!");
        lcd.display();
        delay(1000);
    }
}

void loop()
{
    if (sensor.requestMeasurement()) {
        clearLcd();
        displayMeasurements();
        updateEepromIfRequired();
    }
}
