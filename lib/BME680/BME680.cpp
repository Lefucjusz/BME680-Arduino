#include "BME680.hpp"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

namespace
{
    constexpr auto chipSelectPin = 46;
    constexpr auto stateSize = BSEC_MAX_STATE_BLOB_SIZE;
    constexpr auto checksumSize = 1;
    constexpr auto rawStateSize = stateSize + checksumSize;
    constexpr auto stateAddress = 0;
    constexpr auto stateChecksumAddress = stateSize;
}

BME680::BME680() {}

BME680::~BME680() {}

bool BME680::init(SPIClass &spi, EEPROM *eeprom)
{
    this->eeprom = eeprom;

    configureChipSelect();
    sensor.begin(chipSelectPin, spi);
    if (!checkSensorStatus()) {
        return false;
    }
    updateSubscription();
    return checkSensorStatus();
}

bool BME680::requestMeasurement()
{
    return sensor.run();
}

float BME680::getIAQ()
{
    return sensor.iaq;
}

const char *BME680::getIAQString()
{
    const auto &unknownString = "unknown";
    if (getIAQAccuracy() == 0) {
        return unknownString;
    }

    const auto currentIAQ = static_cast<uint16_t>(getIAQ());
    for (size_t i = 0; i < ARRAY_SIZE(IAQRangesToString); ++i) {
        const auto &IAQRange = IAQRangesToString[i];
        if ((IAQRange.upperLimit >= currentIAQ) && (IAQRange.lowerLimit <= currentIAQ)) {
            return IAQRange.string;
        }
    }
    return unknownString;
}

uint8_t BME680::getIAQAccuracy()
{
    return sensor.iaqAccuracy;
}

uint8_t BME680::getMaxIAQAccuracy()
{
    return maxIAQAccuracy;
}

float BME680::getStaticIAQ()
{
    return sensor.staticIaq;
}

uint8_t BME680::getStaticIAQAccuracy()
{
    return sensor.staticIaqAccuracy;
}

float BME680::getCO2Equivalent()
{
    return sensor.co2Equivalent;
}

float BME680::getBreathVOCEquivalent()
{
    return sensor.breathVocEquivalent;
}

float BME680::getPressure()
{
    return sensor.pressure;
}

float BME680::getPressureHPa()
{
    return sensor.pressure / 100;
}

float BME680::getHumidity()
{
    return sensor.humidity;
}

float BME680::getTemperature()
{
    return sensor.temperature;
}

float BME680::getGasResistance()
{
    return sensor.gasResistance;
}

float BME680::getGasPercentage()
{
    return sensor.gasPercentage;
}

float BME680::getStabStatus()
{
    return sensor.stabStatus;
}

float BME680::getRunInStatus()
{
    return sensor.runInStatus;
}

bool BME680::loadState()
{
    uint8_t rawState[rawStateSize];
    
    if (!eeprom->init()) {
        return false;
    }
    const auto readStatus = eeprom->read(rawState, sizeof(rawState), stateAddress);
    eeprom->deinit();

    if (!readStatus || !isStateValid(rawState)) {
        return false;
    }

    // for (size_t i = 0; i < sizeof(rawState); ++i) {
    //     char tmp[5];
    //     snprintf(tmp, sizeof(tmp), "%02X ", rawState[i]);
    //     Serial.print(tmp);
    //     if (!((i+1)%8)) {
    //         Serial.print('\n');
    //     }
    // }

    sensor.setState(rawState);
    return true;
}

bool BME680::storeState()
{
    uint8_t rawState[rawStateSize];

    sensor.getState(rawState);
    rawState[stateChecksumAddress] = computeStateChecksum(rawState);

    if (!eeprom->init()) {
        return false;
    }
    const auto writeStatus = eeprom->write(rawState, sizeof(rawState), stateAddress);
    eeprom->deinit();

    return writeStatus;
}

bool BME680::checkSensorStatus()
{
    return (sensor.bsecStatus == BSEC_OK) && (sensor.bme68xStatus == BME68X_OK);
}

bsec_version_t BME680::getBSECVersion()
{
    return sensor.version;
}

void BME680::configureChipSelect()
{
    pinMode(chipSelectPin, OUTPUT);
    digitalWrite(chipSelectPin, HIGH);
}

void BME680::updateSubscription()
{
    bsec_virtual_sensor_t sensorsList[] = {
        BSEC_OUTPUT_IAQ,
        BSEC_OUTPUT_STATIC_IAQ,
        BSEC_OUTPUT_CO2_EQUIVALENT,
        BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
        BSEC_OUTPUT_RAW_TEMPERATURE,
        BSEC_OUTPUT_RAW_PRESSURE,
        BSEC_OUTPUT_RAW_HUMIDITY,
        BSEC_OUTPUT_RAW_GAS,
        BSEC_OUTPUT_STABILIZATION_STATUS,
        BSEC_OUTPUT_RUN_IN_STATUS,
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
        BSEC_OUTPUT_GAS_PERCENTAGE
    };

    sensor.updateSubscription(sensorsList, ARRAY_SIZE(sensorsList), BSEC_SAMPLE_RATE_ULP);
}

uint8_t BME680::computeStateChecksum(const uint8_t *state)
{
    uint8_t checksum = 0xBB;
    for (size_t i = 0; i < stateSize; ++i) {
        checksum ^= state[i];
    }
    return checksum;
}

bool BME680::isStateValid(const uint8_t *rawState) 
{
    return (computeStateChecksum(rawState) == rawState[stateChecksumAddress]);
}
