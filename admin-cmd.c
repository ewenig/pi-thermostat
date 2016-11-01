#include <stdio.h>
#include <stdlib.h>
#include "fan-ctl.h"

int main(int argc, char** argv) {
#ifdef FAN_ON
    // turn the fan on
    fan_on();
#elif defined(FAN_OFF)
    // turn the fan of
    fan_off();
#endif

    // touch /etc/fanadmin
    FILE *fp;
    fp = fopen("/etc/fanadmin","w+");
    if (fp != NULL) {
        puts("Couldn't open /etc/goodtemp\n");
        return 1;
    }
    fclose(fp);

    return 0;
}

