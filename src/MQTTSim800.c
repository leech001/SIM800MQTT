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

uint8_t rx_data = 0;
uint8_t rx_buffer[32] = {0};
int rx_index = 0;
char answer[32] = {0};

/**
  * Call back function for release read SIM800 UART buffer.
  * @param NONE
  * @return SIM800 answer for command (char answer[64])
*/
void Sim800_RxCallBack(void) {
    rx_buffer[rx_index++] = rx_data;

    if (strstr((char *) rx_buffer, "\r\n") != NULL && rx_index == 2) {
        rx_index = 0;
    } else if (strstr((char *) rx_buffer, "\r\n") != NULL) {
        memcpy(answer, rx_buffer, sizeof(rx_buffer));
        rx_index = 0;
        memset(rx_buffer, 0, sizeof(rx_buffer));
    }
    HAL_UART_Receive_IT(&huart1, &rx_data, 1);
}

/**
  * Send AT command to SIM800 over UART.
  * @param command the command to be used the send AT command
  * @param reply to be used to set the correct answer to the command
  * @param delay to be used to the set pause to the reply
  * @return error, 0 is OK
  */
int SIM800_SendCommand(char *command, char *reply, uint16_t delay) {
    HAL_UART_Transmit_IT(UART_SIM800, (unsigned char *) command, (uint16_t) strlen(command));

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
  * Send data over AT command.
  * @param buf the buffer into which the data will be send
  * @param len the length in bytes of the supplied buffer
  * @return error, 0 is OK
  */
int SIM800_SendData(uint8_t *buf, int len) {
    char str[32] = {0};
    sprintf(str, "AT+CIPSEND=%d\r\n", len);
    SIM800_SendCommand(str, "", 200);
    HAL_UART_Transmit_IT(UART_SIM800, buf, len);

#if FREERTOS == 1
    osDelay(200);
#else
    HAL_Delay(CMD_DELAY);
#endif

    if (strstr(answer, "SEND OK") != NULL) {
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
  * @return error the number of mistakes, 0 is OK
  */
int SIM800_Init(void) {
    int error = 0;
    HAL_UART_Receive_IT(UART_SIM800, &rx_data, 1);
    error += SIM800_SendCommand("AT\r\n", "OK\r\n", CMD_DELAY);
    error += SIM800_SendCommand("ATE0\r\n", "OK\r\n", CMD_DELAY);
    return error;
}

/**
  * Connect to MQTT server in Internet over TCP.
  * @param apn to be used the set APN for you GSM net
  * @param apn_user to be used the set user name for APN
  * @param apn_pass to be used the set password for APN
  * @param host to be used the set MQTT broker IP or host name
  * @param port to be used the set MQTT broker TCP port
  * @param username to be used the set authentication user name for MQTT broker
  * @param pass to be used the set authentication password for MQTT broker
  * @param clietID to be used to the set client identifier for MQTT broker
  * @param keepAliveInterval to be used to the set keepalive interval for MQTT broker
  * @return error the number of mistakes, 0 is OK
  */
int MQTT_Connect(char *apn, char *apn_user, char *apn_pass, char *host, uint16_t port, char *username, char *pass,
                 char *clientID, unsigned short keepAliveInterval) {
    int error = 0;
    char str[64] = {0};
    unsigned char buf[64] = {0};
    MQTTPacket_connectData datas = MQTTPacket_connectData_initializer;
    datas.username.cstring = username;
    datas.password.cstring = pass;
    datas.clientID.cstring = clientID;
    datas.keepAliveInterval = keepAliveInterval;
    datas.cleansession = 1;

    error += SIM800_SendCommand("AT+CIPSHUT\r\n", "SHUT OK\r\n", CMD_DELAY);
    error += SIM800_SendCommand("AT+CGATT=1\r\n", "OK\r\n", CMD_DELAY);

    snprintf(str, sizeof(str), "AT+CSTT=\"%s\",\"%s\",\"%s\"\r\n", apn, apn_user, apn_pass);
    error += SIM800_SendCommand(str, "OK\r\n", CMD_DELAY);

    error += SIM800_SendCommand("AT+CIICR\r\n", "OK\r\n", CMD_DELAY);
    SIM800_SendCommand("AT+CIFSR\r\n", "OK\r\n", CMD_DELAY);

    memset(str, 0, sizeof(str));
    sprintf(str, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", host, port);
    error += SIM800_SendCommand(str, "CONNECT OK\r\n", CMD_DELAY);

    int mqtt_len = MQTTSerialize_connect(buf, sizeof(buf), &datas);

    error += SIM800_SendData(buf, mqtt_len);

    return error;
}

/**
  * Public on the MQTT broker of the message in a topic
  * @param topic to be used to the set topic
  * @param payload to be used to the set message for topic
  * @return error the number of mistakes, 0 is OK
  */
int MQTT_Pub(char *topic, char *payload) {
    int error = 0;
    unsigned char buf[256] = {0};

    MQTTString topicString = MQTTString_initializer;
    topicString.cstring = topic;

    int mqtt_len = MQTTSerialize_publish(buf, sizeof(buf), 0, 0, 0, 0, topicString, (unsigned char*) payload, (int) strlen(payload));

    error += SIM800_SendData(buf, mqtt_len);

    return error;
}

/**
  * Send a PINGREQ to the MQTT broker (active session)
  * @param NONE
  * @return error the number of mistakes, 0 is OK
  */
int MQTT_PingReq(void) {
    int error = 0;
    unsigned char buf[16] = {0};

    int mqtt_len = MQTTSerialize_pingreq(buf, sizeof(buf));
    error += SIM800_SendData(buf, mqtt_len);

    return error;
}
