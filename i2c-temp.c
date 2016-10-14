#include <stdio.h>
#include "temp-ctl.h"

int main(int argc, char** argv) {
    float temp = get_temp();
    
    printf("value=%.2f", temp);
    return 0;
}

