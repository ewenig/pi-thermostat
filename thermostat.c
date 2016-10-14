#include <stdio.h>
#include <stdlib.h>
#include "temp-ctl.h"
#include "fan-ctl.h"

int main(char argc, char** argv) {
    // open goodtemp file
    FILE *fp;
    fp = fopen("/etc/fanadmin","r");
    if (fp != NULL) {
        puts("/etc/fanadmin set, aborting\n");
        fclose(fp);
        return 0;
    }

    fp = fopen("/etc/goodtemp", "r");
    if (fp == NULL) {
        puts("Couldn't open /etc/goodtemp\n");
        return 1;
    }

    char *buf;
    size_t len;
    ssize_t ret = getdelim(&buf, &len, '\0', fp);
    fclose(fp);
    if (ret == -1) {
        puts("Couldn't read /etc/goodtemp\n");
        return 1;
    }

    float good_temp = strtof(buf, NULL);

    float cur_temp = get_temp();

    if (cur_temp > good_temp) {
        fan_on();
    } else {
        fan_off();
    }

    return 0;
}
