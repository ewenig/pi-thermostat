#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <wiringPiI2C.h>

#define I2C_DEV 0x18
#define REG_AMBIENT_TEMP 0x05

int main(int argc, char** argv);
float CtoF(float celsius);

int main(int argc, char** argv) {
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

    printf("Temp is %.2f\n", temp);
}

float CtoF(float celsius) {
    return celsius * (9.0 / 5.0) + 32.0;
}
