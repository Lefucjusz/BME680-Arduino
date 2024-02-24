#pragma once

#include <stddef.h>
#include <stdint.h>

class EEPROM
{
    public:
        virtual ~EEPROM() = default;

        virtual bool init() = 0;
        virtual bool init(uint8_t i2cAddress) = 0;
        virtual void deinit() = 0;

        virtual bool read(void *data, size_t size, size_t address) = 0;
        virtual bool write(const void *data, size_t size, size_t address) = 0;
};
