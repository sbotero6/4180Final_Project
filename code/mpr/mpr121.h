#ifndef MPR121_H
#define MPR121_H

#include "mbed.h"

// Define I2C Address and Threshold Registers
#define MPR121_I2C_ADDR              0x5A
#define MPR121_TOUCH_THRESHOLD       0x41  // Threshold register
#define MPR121_RELEASE_THRESHOLD     0x42  // Release register
#define MPR121_TOUCH_STATUS_L        0x00  // Low byte of touch status

class MPR121 {
public:
    MPR121(PinName sda, PinName scl, char address = MPR121_I2C_ADDR);

    bool init();
    uint16_t getTouchStatus();

private:
    I2C _i2c;
    char _address;  // Use 'char' for I2C address
};

#endif
