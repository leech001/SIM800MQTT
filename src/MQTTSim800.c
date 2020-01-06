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
uint8_t rx_buffer[64] = {0};
int rx_index = 0;
char answer[64] = {0};

void Sim800_RxCallBack(void){
    rx_buffer[rx_index++] = rx_data;

    if(strstr((char*) rx_buffer, "\r\n") != NULL && rx_index == 2 ) {
        rx_index = 0;
    } else if (strstr((char*) rx_buffer, "\r\n") != NULL){
        memcpy(answer, rx_buffer, sizeof(rx_buffer));
        rx_index = 0;
        memset(rx_buffer, 0, sizeof(rx_buffer));
    }
    HAL_UART_Receive_IT(&huart1, &rx_data, 1);
}

int SIM800_SendCommand(char* command, char* reply, uint16_t delay){
    HAL_UART_Transmit_IT(UART_SIM800, (unsigned char *) command, (uint16_t) strlen(command));

#if FREERTOS == 1
    osDelay(delay);
#else
    HAL_Delay(delay);
#endif

    if(strstr(answer, reply) != NULL){
        return 0;
    }
    return 1;
}

int SIM800_SendData(uint8_t* buf, int len) {
    char str[64] = {0};
    sprintf(str, "AT+CIPSEND=%d\r\n", len);
    SIM800_SendCommand(str, "", CMD_DELAY);
    HAL_UART_Transmit_IT(UART_SIM800,  buf, len);

#if FREERTOS == 1
    osDelay(CMD_DELAY);
#else
    HAL_Delay(CMD_DELAY);
#endif

    if(strstr(answer, "SEND OK") != NULL){
        return 0;
    }
    return 1;
}

int SIM800_Init(void){
    int error = 0;
    HAL_UART_Receive_IT(UART_SIM800, &rx_data, 1);
    error += SIM800_SendCommand("AT\r\n", "OK\r\n", CMD_DELAY);
    error += SIM800_SendCommand("ATE0\r\n", "OK\r\n", CMD_DELAY);
    return error;
}

int MQTT_Connect(char* apn, char* apn_user, char* apn_pass, char* host, uint16_t port, char* username, char* pass, char* clientID, unsigned short keepAliveInterval){
    int error = 0;
    static char str[64] = {0};
    static unsigned char buf[200] = {0};
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

int MQTT_Pub(char* topic, char* payload){
    int error = 0;
    static unsigned char buf[200]= {0};
    static unsigned char buf_payload[64] = {0};

    MQTTString topicString = MQTTString_initializer;
    topicString.cstring = topic;

    memcpy(buf_payload, payload, strlen(payload));

    int mqtt_len = MQTTSerialize_publish(buf, sizeof(buf), 0, 0, 0, 0, topicString, buf_payload, (int) strlen(payload));

    error += SIM800_SendData(buf, mqtt_len);

    return error;
}

int MQTT_PingReq(void){
    int error = 0;
    static unsigned char buf[16]= {0};

    int mqtt_len = MQTTSerialize_pingreq(buf, sizeof(buf));
    error += SIM800_SendData(buf, mqtt_len);

    return error;
}
