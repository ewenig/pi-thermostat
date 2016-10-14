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
void onConnected(struct mosquitto *mqtt, void *ptr, int res);

bool pubSucceeded;

struct mqtt_data {
    char *user;
    char *feed;
    float temp;
};

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
    pubSucceeded = false;

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

    // setup user data structure
    struct mqtt_data *data = (struct mqtt_data *)malloc(sizeof(struct mqtt_data));
    data->user = strdup(user);
    data->feed = strdup(feed);
    data->temp = temp;

    struct mosquitto *mqtt = mosquitto_new(NULL, true, data);
    ret = mosquitto_username_pw_set(mqtt, user, key);
    if (ret != MOSQ_ERR_SUCCESS) {
        printf("Couldn't set PW: %s\n", mosquitto_strerror(ret));
        return 1;
    }

    // setup callback
    mosquitto_connect_callback_set(mqtt, (void *) &onConnected);

    // XXX
    ret = mosquitto_tls_insecure_set(mqtt, true);
    if (ret != MOSQ_ERR_SUCCESS) {
        printf("Couldn't set insecure TLS: %s\n", mosquitto_strerror(ret));
        return 1;
    }

    ret = mosquitto_tls_set(mqtt, "adafruit.pem", NULL, NULL, NULL, NULL);
    if (ret != MOSQ_ERR_SUCCESS) {
        printf("Couldn't set TLS: %s\n", mosquitto_strerror(ret));
        return 1;
    }

    ret = mosquitto_connect(mqtt, "io.adafruit.com", 8883, 60);
    if (ret != MOSQ_ERR_SUCCESS) {
        printf("Couldn't connect to adafruit: %s\n", mosquitto_strerror(ret));
        return 1;
    }

    while (!pubSucceeded) {
        mosquitto_loop(mqtt, 1000, 1);
    }

    ret = mosquitto_disconnect(mqtt);
    if (ret != MOSQ_ERR_SUCCESS) {
        printf("Couldn't disconnect from adafruit: %s\n", mosquitto_strerror(ret));
        return 1;
    }

    printf("Successfully published to adafruit\n");
    mosquitto_destroy(mqtt);
    mosquitto_lib_cleanup();
    return 0;
 }

void onConnected(struct mosquitto *mqtt, void *ptr, int res) {
    struct mqtt_data *data = (struct mqtt_data *)ptr;
    if (res != 0) {
        printf("Failed connecting to adafruit: %s\n", mosquitto_connack_string(res));
        exit(1);
    }

    // construct the message
    char msg[512];
    sprintf(msg, "%.2f", data->temp);

    char feedid[strlen(data->user) + strlen(data->feed) + 7];
    sprintf(feedid, "%s/feed/%s", data->user, data->feed);

    int ret = mosquitto_publish(mqtt, NULL, feedid, sizeof(msg), (void *) msg, 1, false);
    if (ret != MOSQ_ERR_SUCCESS) {
        printf("Couldn't publish to adafruit: %s\n", mosquitto_strerror(ret));
        exit(1);
    }

    while (mosquitto_want_write(mqtt)) {
        ret = mosquitto_loop_write(mqtt, 1);
        if (ret != MOSQ_ERR_SUCCESS && strcmp(mosquitto_strerror(ret), "Success") != 0) {
            printf("Couldn't loop: %s\n", mosquitto_strerror(ret));
            exit(1);
        }
    }

    free(data->user);
    free(data->feed);
    free(data);
    pubSucceeded = true;
}