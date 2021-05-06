/*
 * MQTTSim800.h
 *
 *  Created on: Jan 4, 2020
 *      Author: Bulanov Konstantin
 *
 *  Contact information
 *  -------------------
 *
 * e-mail   :   leech001@gmail.com
 * telegram :   https://t.me/leech001
 *
 *
 */

/*
 * -----------------------------------------------------------------------------------------------------------------------------------------------
           DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                    Version 2, December 2004

 Copyright (C) 2020 Bulanov Konstantin <leech001@gmail.com>

 Everyone is permitted to copy and distribute verbatim or modified
 copies of this license document, and changing it is allowed as long
 as the name is changed.

            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

  0. You just DO WHAT THE FUCK YOU WANT TO.

  MQTT packet https://github.com/eclipse/paho.mqtt.embedded-c/tree/master/MQTTPacket
 * ------------------------------------------------------------------------------------------------------------------------------------------------
*/

#include <main.h>

// === CONFIG ===
#define UART_SIM800 &huart2
#define FREERTOS    0
#define CMD_DELAY   2000
// ==============

typedef struct {
    char *apn;
    char *apn_user;
    char *apn_pass;
} sim_t;

typedef struct {
    char *host;
    uint16_t port;
    uint8_t connect;
} mqttServer_t;

typedef struct {
    char *username;
    char *pass;
    char *clientID;
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

void Sim800_RxCallBack(void);

void clearRxBuffer(void);

void clearMqttBuffer(void);

int SIM800_SendCommand(char *command, char *reply, uint16_t delay);

int MQTT_Init(void);

void MQTT_Connect(void);

void MQTT_Pub(char *topic, char *payload);

void MQTT_PubUint8(char *topic, uint8_t data);

void MQTT_PubUint16(char *topic, uint16_t data);

void MQTT_PubUint32(char *topic, uint32_t data);

void MQTT_PubFloat(char *topic, float payload);

void MQTT_PubDouble(char *topic, double data);

void MQTT_PingReq(void);

void MQTT_Sub(char *topic);

void MQTT_Receive(unsigned char *buf);
