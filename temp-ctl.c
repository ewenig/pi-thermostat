#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <wiringPiI2C.h>
#include "temp-ctl.h"

float get_temp() {
    int fd = wiringPiI2CSetup(I2C_DEV);
    if (fd == -1) {
        printf("Error opening I2C device: %s\n", strerror(errno));
        return 1;
    }

    uint16_t t = wiringPiI2CReadReg16(fd, REG_AMBIENT_TEMP);
    if (t == -1) {
        printf("Error reading I2C temperature register: %s\n", strerror(errno));
        return 1;
    }

    // endian-flip the register value
    t = be16toh(t);

    // translate the register value into degrees Celsius
    float temp = (t & 0x0FFF) / 16.0;
    if (t & 0x1000)
        temp -= 256.0;

    // convert temperature value to degrees Fahrenheit
    temp = CtoF(temp);

    return temp;
}

float CtoF(float celsius) {
    return celsius * (9.0 / 5.0) + 32.0;
}

