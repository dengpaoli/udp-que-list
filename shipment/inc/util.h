#ifndef MQTT_UTIL_H
#define MQTT_UTIL_H

#define TRUE  1
#define FALSE 0

#define uchar unsigned char

#define DEBUG

#ifdef DEBUG
#define PRINT(X,...) printf(X,##__VA_ARGS__)
#else
#define PRINT(X,...)
#endif

#endif //MQTT_UTIL_H

