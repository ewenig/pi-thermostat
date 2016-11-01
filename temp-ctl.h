#ifndef TEMP_CTL_H
#define TEMP_CTL_H

float get_temp();
float CtoF(float celsius);

#define I2C_DEV 0x18
#define REG_AMBIENT_TEMP 0x05

#endif

