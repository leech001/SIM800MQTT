/*
 * MQTTSim800.h
 *
 *  Created on: Jan 4, 2020
 *      Author: Bulanov Konstantin
 *
 *  Contact information
 *  -------------------
 *
 * e-mail   :  leech001@gmail.com
 *
 *
 */

#include <main.h>

// === CONFIG ===
#define UART_SIM800 &huart1
#define FREERTOS    0
#define CMD_DELAY   2000
// ==============

void Sim800_RxCallBack(void);
int SIM800_SendCommand(char* command, char* reply, uint16_t delay);
int SIM800_Init(void);
int MQTT_Connect(char* apn, char* apn_user, char* apn_pass, char* host, uint16_t port, char* username, char* pass, char* clientID, unsigned short keepAliveInterval);
int MQTT_Pub(char* topic, char* payload);
int MQTT_PingReq(void);
