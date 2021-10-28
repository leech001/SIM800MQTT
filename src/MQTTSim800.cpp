/*
 * MQTTSim800.cpp
 *
 *  Created on: Oct 27, 2021
 *      Author: Nazmi Febrian with forking from Bulanov Konstantin's Github
 *
 *  Contact information
 *  -------------------
 * Bulanov Konstantin
 * e-mail   :   leech001@gmail.com
 * telegram :   https://t.me/leech001
 *
 * Nazmi Febrian
 * email    :   nazmi.febrian@gmail.com
 * telegram :   @nazmibojan
 * 
 */

/*
 * -----------------------------------------------------------------------------------------------------------------------------------------------
           DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                    Version 2, December 2004

 Copyright (C) 2020 Bulanov Konstantin <leech001@gmail.com> & 2021 Nazmi Febrian <nazmi.febrian@gmail.com>

 Everyone is permitted to copy and distribute verbatim or modified
 copies of this license document, and changing it is allowed as long
 as the name is changed.

            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

  0. You just DO WHAT THE FUCK YOU WANT TO.

  MQTT packet https://github.com/eclipse/paho.mqtt.embedded-c/tree/master/MQTTPacket
 * ------------------------------------------------------------------------------------------------------------------------------------------------
*/

#include <MQTTSim800.hpp>
#include <string.h>
#include "MQTTPacket.h"

MQTTSim800::MQTTSim800(UART_HandleTypeDef *uartHandle)
{
  mUart = uartHandle;
}

MQTTSim800::~MQTTSim800()
{
}

void MQTTSim800::setModemApn(const char *name, const char *username, const char *password) {
  strcpy(&mModem.sim.apn[0], name);
  strcpy(&mModem.sim.apn_user[0], username);
  strcpy(&mModem.sim.apn_pass[0], password);
}

void MQTTSim800::setMqttServer(const char *host, uint16_t port) {
  strcpy(&mModem.mqttServer.host[0], host);
  mModem.mqttServer.port = port;
}

void MQTTSim800::setClientData(const char *userMqtt, const char* passMqtt, const char *clientId, unsigned short interval) {
  strcpy(&mModem.mqttClient.username[0], userMqtt);
  strcpy(&mModem.mqttClient.pass[0], passMqtt);
  strcpy(&mModem.mqttClient.clientID[0], clientId);
  mModem.mqttClient.keepAliveInterval = interval;
}

/**
 * Call back function for release read SIM800 UART buffer.
 * @param NONE
 * @return NONE
 */
void MQTTSim800::rxCallBack(void)
{
    mRxBuffer[mRxIndex++] = mRxData;

    if (mModem.mqttServer.connect == 0)
    {
        if (strstr((char *)mRxBuffer, "\r\n") != NULL && mRxIndex == 2)
        {
            mRxIndex = 0;
        }
        else if (strstr((char *)mRxBuffer, "\r\n") != NULL)
        {
            memcpy(mqttBuffer, mRxBuffer, sizeof(mRxBuffer));
            clearRxBuffer();
            if (strstr(mqttBuffer, "DY CONNECT\r\n"))
            {
                mModem.mqttServer.connect = 0;
            }
            else if (strstr(mqttBuffer, "CONNECT\r\n"))
            {
                mModem.mqttServer.connect = 1;
            }
        }
    }
    if (strstr((char *)mRxBuffer, "CLOSED\r\n") || strstr((char *)mRxBuffer, "ERROR\r\n") || strstr((char *)mRxBuffer, "DEACT\r\n"))
    {
        mModem.mqttServer.connect = 0;
    }
    if (mModem.mqttServer.connect == 1 && mRxData == 48)
    {
        mqttReceive = 1;
    }
    if (mqttReceive == 1)
    {
        mqttBuffer[mqttIndex++] = mRxData;
        if (mqttIndex > 1 && mqttIndex - 1 > mqttBuffer[1])
        {
            receive((unsigned char *)mqttBuffer);
            clearRxBuffer();
            clearMqttBuffer();
        }
        if (mqttIndex >= sizeof(mqttBuffer))
        {
            clearMqttBuffer();
        }
    }
    if (mRxIndex >= sizeof(mqttBuffer))
    {
        clearRxBuffer();
        clearMqttBuffer();
    }
    HAL_UART_Receive_IT(mUart, &mRxData, 1);
}

/**
 * Clear SIM800 UART RX buffer.
 * @param NONE
 * @return NONE
 */
void MQTTSim800::clearRxBuffer(void)
{
    mRxIndex = 0;
    memset(mRxBuffer, 0, sizeof(mRxBuffer));
}

/**
 * Clear MQTT buffer.
 * @param NONE
 * @return NONE
 */
void MQTTSim800::clearMqttBuffer(void)
{
    mqttReceive = 0;
    mqttIndex = 0;
    memset(mqttBuffer, 0, sizeof(mqttBuffer));
}

/**
 * Send AT command to SIM800 over UART.
 * @param command the command to be used the send AT command
 * @param reply to be used to set the correct answer to the command
 * @param delay to be used to the set pause to the reply
 * @return error, 0 is OK
 */
int MQTTSim800::sendCommand(const char *command, const char *reply, uint16_t delay)
{
    HAL_UART_Transmit_IT(mUart, (unsigned char *)command,
                         (uint16_t)strlen(command));

#if FREERTOS == 1
    osDelay(delay);
#else
    HAL_Delay(delay);
#endif

    if (strstr(mqttBuffer, reply) != NULL)
    {
        clearRxBuffer();
        return 0;
    }
    clearRxBuffer();
    return 1;
}

/**
 * initialization SIM800.
 * @param NONE
 * @return error status, 0 - OK
 */
int MQTTSim800::init(void)
{
    mModem.mqttServer.connect = 0;
    int error = 0;
    char str[SIM800_MAXCMD_LEN] = {0};
    HAL_UART_Receive_IT(mUart, &mRxData, 1);

    sendCommand("AT\r\n", "OK\r\n", CMD_DELAY);
    sendCommand("ATE0\r\n", "OK\r\n", CMD_DELAY);
    error += sendCommand("AT+CIPSHUT\r\n", "SHUT OK\r\n", CMD_DELAY);
    error += sendCommand("AT+CGATT=1\r\n", "OK\r\n", CMD_DELAY);
    error += sendCommand("AT+CIPMODE=1\r\n", "OK\r\n", CMD_DELAY);

    snprintf(str, sizeof(str), "AT+CSTT=\"%s\",\"%s\",\"%s\"\r\n", mModem.sim.apn, mModem.sim.apn_user,
             mModem.sim.apn_pass);
    error += sendCommand(str, "OK\r\n", CMD_DELAY);

    error += sendCommand("AT+CIICR\r\n", "OK\r\n", CMD_DELAY);
    sendCommand("AT+CIFSR\r\n", "", CMD_DELAY);
    if (error == 0)
    {
        connect();
        return error;
    }
    else
    {
        return error;
    }
}

/**
 * Connect to MQTT server in Internet over TCP.
 * @param NONE
 * @return NONE
 */
void MQTTSim800::connect(void)
{
    mModem.mqttReceive.newEvent = 0;
    mModem.mqttServer.connect = 0;
    char str[128] = {0};
    unsigned char buf[128] = {0};
    sprintf(str, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", mModem.mqttServer.host, mModem.mqttServer.port);
    sendCommand(str, "OK\r\n", CMD_DELAY);
#if FREERTOS == 1
    osDelay(5000);
#else
    HAL_Delay(5000);
#endif
    if (mModem.mqttServer.connect == 1)
    {
        MQTTPacket_connectData datas = MQTTPacket_connectData_initializer;
        datas.username.cstring = mModem.mqttClient.username;
        datas.password.cstring = mModem.mqttClient.pass;
        datas.clientID.cstring = mModem.mqttClient.clientID;
        datas.keepAliveInterval = mModem.mqttClient.keepAliveInterval;
        datas.cleansession = 1;
        int mqtt_len = MQTTSerialize_connect(buf, sizeof(buf), &datas);
        HAL_UART_Transmit_IT(mUart, buf, mqtt_len);
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
void MQTTSim800::publish(char *topic, char *payload)
{
    unsigned char buf[256] = {0};

    MQTTString topicString = MQTTString_initializer;
    topicString.cstring = topic;

    int mqtt_len = MQTTSerialize_publish(buf, sizeof(buf), 0, 0, 0, 0,
                                         topicString, (unsigned char *)payload, (int)strlen(payload));
    HAL_UART_Transmit_IT(mUart, buf, mqtt_len);
#if FREERTOS == 1
    osDelay(100);
#else
    HAL_Delay(100);
#endif
}

/**
 * Public on the MQTT broker of the message in a topic
 * @param topic (uint8_t)  to be used to the set topic
 * @param payload to be used to the set message for topic
 * @return NONE
 */
void MQTTSim800::publishUint8(char *topic, uint8_t payload)
{
    char str[32] = {0};
    sprintf(str, "%u", payload);
    publish(topic, str);
}

/**
 * Public on the MQTT broker of the message in a topic
 * @param topic (uint16_t)  to be used to the set topic
 * @param payload to be used to the set message for topic
 * @return NONE
 */
void MQTTSim800::publishUint16(char *topic, uint16_t payload)
{
    char str[32] = {0};
    sprintf(str, "%u", payload);
    publish(topic, str);
}

/**
 * Public on the MQTT broker of the message in a topic
 * @param topic (uint32_t)  to be used to the set topic
 * @param payload to be used to the set message for topic
 * @return NONE
 */
void MQTTSim800::publishUint32(char *topic, uint32_t payload)
{
    char str[32] = {0};
    sprintf(str, "%lu", payload);
    publish(topic, str);
}

/**
 * Public on the MQTT broker of the message in a topic
 * @param topic (float)  to be used to the set topic
 * @param payload to be used to the set message for topic
 * @return NONE
 */
void MQTTSim800::publishFloat(char *topic, float payload)
{
    char str[32] = {0};
    sprintf(str, "%f", payload);
    publish(topic, str);
}

/**
 * Public on the MQTT broker of the message in a topic
 * @param topic (double)  to be used to the set topic
 * @param payload to be used to the set message for topic
 * @return NONE
 */
void MQTTSim800::publishDouble(char *topic, double payload)
{
    char str[32] = {0};
    sprintf(str, "%f", payload);
    publish(topic, str);
}

/**
 * Send a PINGREQ to the MQTT broker (active session)
 * @param NONE
 * @return NONE
 */
void MQTTSim800::pingReq(void)
{
    unsigned char buf[16] = {0};

    int mqtt_len = MQTTSerialize_pingreq(buf, sizeof(buf));
    HAL_UART_Transmit_IT(mUart, buf, mqtt_len);
}

/**
 * Subscribe on the MQTT broker of the message in a topic
 * @param topic to be used to the set topic
 * @return NONE
 */
void MQTTSim800::subscribe(char *topic)
{
    unsigned char buf[256] = {0};

    MQTTString topicString = MQTTString_initializer;
    topicString.cstring = topic;

    int mqtt_len = MQTTSerialize_subscribe(buf, sizeof(buf), 0, 1, 1,
                                           &topicString, 0);
    HAL_UART_Transmit_IT(mUart, buf, mqtt_len);
#if FREERTOS == 1
    osDelay(100);
#else
    HAL_Delay(100);
#endif
}

/**
 * Receive message from MQTT broker
 * @param receive mqtt bufer
 * @return NONE
 */
void MQTTSim800::receive(unsigned char *buf)
{
    memset(mModem.mqttReceive.topic, 0, sizeof(mModem.mqttReceive.topic));
    memset(mModem.mqttReceive.payload, 0, sizeof(mModem.mqttReceive.payload));
    MQTTString receivedTopic;
    unsigned char *payload;
    MQTTDeserialize_publish(&mModem.mqttReceive.dup, &mModem.mqttReceive.qos, &mModem.mqttReceive.retained,
                            &mModem.mqttReceive.msgId,
                            &receivedTopic, &payload, &mModem.mqttReceive.payloadLen, buf,
                            sizeof(buf));
    memcpy(mModem.mqttReceive.topic, receivedTopic.lenstring.data, receivedTopic.lenstring.len);
    mModem.mqttReceive.topicLen = receivedTopic.lenstring.len;
    memcpy(mModem.mqttReceive.payload, payload, mModem.mqttReceive.payloadLen);
    mModem.mqttReceive.newEvent = 1;
}