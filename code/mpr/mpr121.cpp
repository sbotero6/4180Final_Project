#include "mpr121.h"

// Constructor with char type for I2C address
MPR121::MPR121(PinName sda, PinName scl, char address) : _i2c(sda, scl), _address(address) {}

bool MPR121::init() {
    char data[2];

    // Set touch threshold
    data[0] = MPR121_TOUCH_THRESHOLD;
    data[1] = 0x0F;  // Example value
    if (_i2c.write(_address, data, 2) != 0) {
        return false;
    }

    // Set release threshold
    data[0] = MPR121_RELEASE_THRESHOLD;
    data[1] = 0x0A;  // Example value
    if (_i2c.write(_address, data, 2) != 0) {
        return false;
    }

    return true;
}

uint16_t MPR121::getTouchStatus() {
    char cmd = MPR121_TOUCH_STATUS_L;
    char data[2] = {0};

    // Write register address
    if (_i2c.write(_address, &cmd, 1) != 0) {
        return 0;
    }

    // Read 16-bit status
    if (_i2c.read(_address, data, 2) != 0) {
        return 0;
    }

    return (data[1] << 8) | data[0];
}
