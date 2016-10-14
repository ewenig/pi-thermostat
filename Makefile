CC=gcc

CFLAGS=-O2
LDFLAGS=-lwiringPi -lpthread

i2c-temp: i2c-temp.c temp-ctl.c
	$(CC) -o i2c-temp $(CFLAGS) $(LDFLAGS) i2c-temp.c temp-ctl.c

thermostat: thermostat.c temp-ctl.c fan-ctl.c
	$(CC) -o thermostat $(CFLAGS) $(LDFLAGS) thermostat.c temp-ctl.c fan-ctl.c
