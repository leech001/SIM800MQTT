#ifndef __SRC_SIM800DATA_H__
#define __SRC_SIM800DATA_H__

// === CONFIG ===
#define FREERTOS    0
#define CMD_DELAY   2000
#define MQTT_MSG_MAXSIZE 1460
// ==============

typedef struct {
    char apn[16];
    char apn_user[16];
    char apn_pass[16];
} sim_t;

typedef struct {
    char host[64];
    uint16_t port;
    uint8_t connect;
} mqttServer_t;

typedef struct {
    char username[64];
    char pass[32];
    char clientID[64];
    unsigned short keepAliveInterval;
} mqttClient_t;

typedef struct {
    uint8_t newEvent;
    unsigned char dup;
    int qos;
    unsigned char retained;
    unsigned short msgId;
    unsigned char payload[64];
    int payloadLen;
    unsigned char topic[64];
    int topicLen;
} mqttReceive_t;

typedef struct {
    sim_t sim;
    mqttServer_t mqttServer;
    mqttClient_t mqttClient;
    mqttReceive_t mqttReceive;
} SIM800_t;

#endif