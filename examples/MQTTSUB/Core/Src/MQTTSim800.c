/*
 * MQTTSim800.c
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

/*
 * |-----------------------------------------------------------------------------------------------------------------------------------------------
 * | Copyright (C) Bulanov Konstantin,2020
 * |
 * | This program is free software: you can redistribute it and/or modify
 * | it under the terms of the GNU General Public License as published by
 * | the Free Software Foundation, either version 3 of the License, or
 * | any later version.
 * |
 * | This program is distributed in the hope that it will be useful,
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * | GNU General Public License for more details.
 * |
 * | You should have received a copy of the GNU General Public License
 * | along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * |
 * | MQTT packet https://github.com/eclipse/paho.mqtt.embedded-c/tree/master/MQTTPacket
 * |------------------------------------------------------------------------------------------------------------------------------------------------
 */

#include "MQTTSim800.h"
#include "main.h"
#include "usart.h"
#include <string.h>
#include "MQTTPacket.h"

#if FREERTOS == 1
#include <cmsis_os.h>
#endif

extern SIM800_t SIM800;

uint8_t rx_data = 0;
uint8_t rx_buffer[256] = {0};
uint8_t rx_index = 0;

char answer[256] = {0};

uint8_t mqtt_receive = 0;
char mqtt_buffer[256] = {0};
uint8_t mqtt_index = 0;

/**
 * Call back function for release read SIM800 UART buffer.
 * @param NONE
 * @return SIM800 answer for command (char answer[64])
 */
void Sim800_RxCallBack(void) {
    rx_buffer[rx_index++] = rx_data;

    if (SIM800.mqttServer.connect == 0) {
        if (strstr((char *) rx_buffer, "\r\n") != NULL && rx_index == 2) {
            rx_index = 0;
        } else if (strstr((char *) rx_buffer, "\r\n") != NULL) {
            memcpy(answer, rx_buffer, sizeof(rx_buffer));
            rx_index = 0;
            memset(rx_buffer, 0, sizeof(rx_buffer));
            if (strstr(answer, "DY CONNECT\r\n")) {
                SIM800.mqttServer.connect = 0;
            } else if (strstr(answer, "CONNECT\r\n")) {
                SIM800.mqttServer.connect = 1;
            }
        }
    }
    if (strstr(answer, "CLOSED\r\n")) {
        SIM800.mqttServer.connect = 0;
    }
    if (strstr(answer, "ERROR\r\n")) {
        SIM800.mqttServer.connect = 0;
    }
    if (SIM800.mqttServer.connect == 1 && rx_data == 48) {
        mqtt_receive = 1;
    }
    if (mqtt_receive == 1) {
        mqtt_buffer[mqtt_index++] = rx_data;
        if (mqtt_index > 1 && mqtt_index - 1 > mqtt_buffer[1]) {
            mqtt_receive = 0;
            MQTT_Receive((unsigned char *) mqtt_buffer);
            mqtt_index = 0;
            memset(mqtt_buffer, 0, sizeof(mqtt_buffer));
            rx_index = 0;
            memset(rx_buffer, 0, sizeof(rx_buffer));
        }
    }
    HAL_UART_Receive_IT(UART_SIM800, &rx_data, 1);
}

/**
 * Send AT command to SIM800 over UART.
 * @param command the command to be used the send AT command
 * @param reply to be used to set the correct answer to the command
 * @param delay to be used to the set pause to the reply
 * @return error, 0 is OK
 */
int SIM800_SendCommand(char *command, char *reply, uint16_t delay) {
    HAL_UART_Transmit_IT(UART_SIM800, (unsigned char *) command,
                         (uint16_t) strlen(command));

#if FREERTOS == 1
    osDelay(delay);
#else
    HAL_Delay(delay);
#endif

    if (strstr(answer, reply) != NULL) {
        rx_index = 0;
        memset(rx_buffer, 0, sizeof(rx_buffer));
        return 0;
    }
    rx_index = 0;
    memset(rx_buffer, 0, sizeof(rx_buffer));
    return 1;
}

/**
 * initialization SIM800.
 * @param NONE
 * @return error status
 */
int MQTT_Init(void) {
    SIM800.mqttServer.connect = 0;
    int error = 0;
    char str[32] = {0};
    HAL_UART_Receive_IT(UART_SIM800, &rx_data, 1);

    SIM800_SendCommand("AT\r\n", "OK\r\n", CMD_DELAY);
    SIM800_SendCommand("ATE0\r\n", "OK\r\n", CMD_DELAY);
    error += SIM800_SendCommand("AT+CIPSHUT\r\n", "SHUT OK\r\n", CMD_DELAY);
    error += SIM800_SendCommand("AT+CGATT=1\r\n", "OK\r\n", CMD_DELAY);
    error += SIM800_SendCommand("AT+CIPMODE=1\r\n", "OK\r\n", CMD_DELAY);

    snprintf(str, sizeof(str), "AT+CSTT=\"%s\",\"%s\",\"%s\"\r\n", SIM800.sim.apn,
             SIM800.sim.apn_user, SIM800.sim.apn_pass);
    error += SIM800_SendCommand(str, "OK\r\n", CMD_DELAY);

    error += SIM800_SendCommand("AT+CIICR\r\n", "OK\r\n", CMD_DELAY);
    SIM800_SendCommand("AT+CIFSR\r\n", "OK\r\n", CMD_DELAY);
    if (error == 0) {
        MQTT_Connect();
        return error;
    } else {
        return error;
    }
}

/**
 * Connect to MQTT server in Internet over TCP.
 * @param NONE
 * @return NONE
 */
void MQTT_Connect(void) {
    SIM800.mqttReceive.newEvent = 0;
    SIM800.mqttServer.connect = 0;
    char str[64] = {0};
    unsigned char buf[64] = {0};

    sprintf(str, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", SIM800.mqttServer.host, SIM800.mqttServer.port);
    SIM800_SendCommand(str, "OK\r\n", CMD_DELAY);

    MQTTPacket_connectData datas = MQTTPacket_connectData_initializer;
    datas.username.cstring = SIM800.mqttClient.username;
    datas.password.cstring = SIM800.mqttClient.pass;
    datas.clientID.cstring = SIM800.mqttClient.clientID;
    datas.keepAliveInterval = SIM800.mqttClient.keepAliveInterval;
    datas.cleansession = 1;

#if FREERTOS == 1
    osDelay(5000);
#else
    HAL_Delay(5000);
#endif
    if (SIM800.mqttServer.connect == 1) {
        int mqtt_len = MQTTSerialize_connect(buf, sizeof(buf), &datas);
        HAL_UART_Transmit_IT(UART_SIM800, buf, mqtt_len);
#if FREERTOS == 1
        osDelay(5000);
#else
        HAL_Delay(5000);
#endif
    }
}

/**
 * Public on the MQTT broker of the message in a topic
 * @param topic to be used to the set topic
 * @param payload to be used to the set message for topic
 * @return NONE
 */
void MQTT_Pub(char *topic, char *payload) {
    unsigned char buf[256] = {0};

    MQTTString topicString = MQTTString_initializer;
    topicString.cstring = topic;

    int mqtt_len = MQTTSerialize_publish(buf, sizeof(buf), 0, 0, 0, 0,
                                         topicString, (unsigned char *) payload, (int) strlen(payload));
    HAL_UART_Transmit_IT(UART_SIM800, buf, mqtt_len);
}

/**
 * Send a PINGREQ to the MQTT broker (active session)
 * @param NONE
 * @return NONE
 */
void MQTT_PingReq(void) {
    unsigned char buf[16] = {0};

    int mqtt_len = MQTTSerialize_pingreq(buf, sizeof(buf));
    HAL_UART_Transmit_IT(UART_SIM800, buf, mqtt_len);
}

/**
 * Subscribe on the MQTT broker of the message in a topic
 * @param topic to be used to the set topic
 * @return NONE
 */
void MQTT_Sub(char *topic) {
    unsigned char buf[256] = {0};

    MQTTString topicString = MQTTString_initializer;
    topicString.cstring = topic;

    int mqtt_len = MQTTSerialize_subscribe(buf, sizeof(buf), 0, 1, 1,
                                           &topicString, 0);
    HAL_UART_Transmit_IT(UART_SIM800, buf, mqtt_len);
#if FREERTOS == 1
    osDelay(5000);
#else
    HAL_Delay(5000);
#endif
    rx_index = 0;
    memset(rx_buffer, 0, sizeof(rx_buffer));
}

/**
 * Receive message from MQTT broker
 * @param receive mqtt bufer
 * @return NONE
 */
void MQTT_Receive(unsigned char *buf) {
    memset(SIM800.mqttReceive.topic, 0, sizeof(SIM800.mqttReceive.topic));
    memset(SIM800.mqttReceive.payload, 0, sizeof(SIM800.mqttReceive.payload));
    MQTTString receivedTopic;
    unsigned char * payload;
    MQTTDeserialize_publish(&SIM800.mqttReceive.dup, &SIM800.mqttReceive.qos, &SIM800.mqttReceive.retained,
                            &SIM800.mqttReceive.msgId,
                            &receivedTopic, &payload, &SIM800.mqttReceive.payloadLen, buf,
                            sizeof(buf));
    memcpy(SIM800.mqttReceive.topic, receivedTopic.lenstring.data, receivedTopic.lenstring.len);
    SIM800.mqttReceive.topicLen = receivedTopic.lenstring.len;
    memcpy(SIM800.mqttReceive.payload, payload, SIM800.mqttReceive.payloadLen);
    SIM800.mqttReceive.newEvent = 1;
}