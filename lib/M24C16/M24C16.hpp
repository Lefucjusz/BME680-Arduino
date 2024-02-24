#pragma once

#include "EEPROM.hpp"

class M24C16 final : public EEPROM
{
   public:
        M24C16();
        ~M24C16();

        bool init() override;
        bool init(uint8_t i2cAddress) override;
        void deinit() override;

        bool read(void *data, size_t size, size_t address) override;
        bool write(const void *data, size_t size, size_t address) override;

        static constexpr uint8_t defaultI2cAddress = 0x50;

    private:
        void computeAddresses(size_t address, uint8_t &chipAddress, uint8_t &memAddress);
        bool isConnected();
        bool waitUntilReady();

        static constexpr uint16_t waitUntilReadyTimeout = 1000;
        bool isInitialized = false;
        uint8_t i2cAddress;
};
