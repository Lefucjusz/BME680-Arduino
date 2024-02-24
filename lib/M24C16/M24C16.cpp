#include "M24C16.hpp"
#include "Wire.h"
#include <Arduino.h>

namespace
{
    constexpr auto dataChunkSize = 16;
}

M24C16::M24C16() {}

M24C16::~M24C16() 
{
    deinit();
}

bool M24C16::init()
{
   return init(defaultI2cAddress);
}

bool M24C16::init(uint8_t i2cAddress)
{
    if (isInitialized) {
        return true;
    }

    this->i2cAddress = i2cAddress;
    Wire.begin();
    isInitialized = isConnected();
    return isInitialized;
}

void M24C16::deinit()
{
    if (!isInitialized) { 
        return;
    }

    Wire.end();
    isInitialized = false;
}

bool M24C16::read(void *data, size_t size, size_t address)
{
    auto dataPtr = reinterpret_cast<char *>(data);
    size_t bytesRead = 0;

    uint8_t chipAddress;
    uint8_t memAddress;
    computeAddresses(address, chipAddress, memAddress);

    while (bytesRead < size) {
        const auto bytesToRead = min(size - bytesRead, dataChunkSize);

        Wire.beginTransmission(chipAddress); // Address the chip
        Wire.write(memAddress + bytesRead); // Address byte in page
        if (Wire.endTransmission() != 0) {
            return false;
        }
        Wire.requestFrom(chipAddress, bytesToRead); // Read size bytes
        for (size_t i = 0; i < bytesToRead; ++i) {
            dataPtr[bytesRead + i] = Wire.read();
        }

        bytesRead += bytesToRead;
    }
    return true;
}

bool M24C16::write(const void *data, size_t size, size_t address)
{
    const auto dataPtr = reinterpret_cast<const char *>(data);
    size_t bytesWritten = 0;

    uint8_t chipAddress;
    uint8_t memAddress;
    computeAddresses(address, chipAddress, memAddress);

    while (bytesWritten < size) {
        const auto bytesToWrite = min(size - bytesWritten, dataChunkSize);

        Wire.beginTransmission(chipAddress); // Address the chip
        Wire.write(memAddress + bytesWritten); // Address byte in page
        Wire.write(&dataPtr[bytesWritten], bytesToWrite); // Write data
        if (Wire.endTransmission() != 0) {
            return false;
        }
        waitUntilReady();

        bytesWritten += bytesToWrite;
    }
    return true;
}

bool M24C16::waitUntilReady()
{
    const auto currentTick = millis();
    while ((millis() - currentTick) <= waitUntilReadyTimeout) {
        if (isConnected()) {
            return true;
        }
    }
    return false;
}

void M24C16::computeAddresses(size_t address, uint8_t &chipAddress, uint8_t &memAddress)
{
    const auto page = (address >> 8) & 0b111;
	memAddress = address & 0xFF;
	chipAddress = (i2cAddress | page);
}

bool M24C16::isConnected()
{
    Wire.beginTransmission(i2cAddress);
    return (Wire.endTransmission() == 0);
}
