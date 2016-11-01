CFLAGS=-O2
LDFLAGS=-lwiringPi -lpthread

PREFIX=/usr

i2c-temp: i2c-temp.c temp-ctl.c
	$(CC) -o i2c-temp $(CFLAGS) $(LDFLAGS) i2c-temp.c temp-ctl.c

thermostat: thermostat.c temp-ctl.c fan-ctl.c
	$(CC) -o thermostat $(CFLAGS) $(LDFLAGS) thermostat.c temp-ctl.c fan-ctl.c

fan-on: admin-cmd.c fan-ctl.c
	$(CC) -o fan-on -DFAN_ON $(CFLAGS) $(LDFLAGS) admin-cmd.c fan-ctl.c

fan-off: admin-cmd.c fan-ctl.c
	$(CC) -o fan-off -DFAN_OFF $(CFLAGS) $(LDFLAGS) admin-cmd.c fan-ctl.c

install: i2c-temp thermostat fan-on fan-off
	install -m 0755 thermostat $(PREFIX)/bin
	install -m 0755 i2c-temp $(PREFIX)/bin
	install -m 0755 fan-on $(PREFIX)/bin
	install -m 0755 fan-off $(PREFIX)/bin

.PHONY: install

