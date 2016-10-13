#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <wiringPiI2C.h>
#include <mosquitto.h>

#define I2C_DEV 0x18
#define REG_AMBIENT_TEMP 0x05

int main(int argc, char** argv);
float CtoF(float celsius);
int publishToAdafruit(float temp);

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

    printf("%.2f\n",temp);
    return publishToAdafruit(temp);
}

float CtoF(float celsius) {
    return celsius * (9.0 / 5.0) + 32.0;
}

int publishToAdafruit(float temp) {
    int ret;
    char *key = getenv("AIO_KEY");
    if (key == NULL) {
        printf("AIO_KEY not set\n");
        return 1;
    }

    char *user = getenv("AIO_USER");
    if (user == NULL) {
        printf("AIO_FEED not set\n");
        return 1;
    }

    char *feed = getenv("AIO_FEED");
    if (feed == NULL) {
        printf("AIO_FEED not set\n");
        return 1;
    }

    mosquitto_lib_init();
    struct mosquitto *mqtt = mosquitto_new(NULL, true, NULL);
    ret = mosquitto_username_pw_set(mqtt, user, key);
    if (ret != MOSQ_ERR_SUCCESS) {
        printf("Couldn't set PW\n");
        return 1;
    }

    // XXX
    ret = mosquitto_tls_insecure_set(mqtt, true);
    if (ret != MOSQ_ERR_SUCCESS) {
        printf("Couldn't set insecure TLS\n");
        return 1;
    }

    ret = mosquitto_connect(mqtt, "io.adafruit.com", 8883, 60);
    if (ret != MOSQ_ERR_SUCCESS) {
        printf("Couldn't connect to adafruit\n");
        return 1;
    }


    // construct the message
    char msg[512];
    sprintf(msg, "%.2f", temp);

    char feedid[strlen(user) + strlen(feed) + 7];
    sprintf(feedid, "%s/feed/%s", user, feed);

    ret = mosquitto_publish(mqtt, NULL, feedid, sizeof(msg), (void *) msg, 0, false);
    if (ret != MOSQ_ERR_SUCCESS) {
        printf("Couldn't publish to adafruit\n");
        return 1;
    }

    ret = mosquitto_disconnect(mqtt);
    if (ret != MOSQ_ERR_SUCCESS) {
        printf("Couldn't disconnect from adafruit\n");
        return 1;
    }

    mosquitto_destroy(mqtt);
    mosquitto_lib_cleanup();
    return 0;
}