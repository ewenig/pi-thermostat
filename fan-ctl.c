#include <stdio.h>
#include <wiringPi.h>
#include "fan-ctl.h"

void fan_setup_pins() {
    wiringPiSetup();
    pinMode(FAN_PIN, OUTPUT);
}

void fan_on() {
    fan_setup_pins();
    digitalWrite(FAN_PIN, HIGH);
}

void fan_off() {
    fan_setup_pins();
    digitalWrite(FAN_PIN, LOW);
}
    
