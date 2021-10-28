/*
 * MQTTSim800.hpp
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

#include <main.h>
#include <usart.h>
#include <Sim800Data.h>
#if FREERTOS == 1
#include <cmsis_os.h>
#endif
#include <stdint.h>

class MQTTSim800
{
private:
  /* data */
public:
  MQTTSim800(UART_HandleTypeDef *uartHandle);
  ~MQTTSim800();

  void setModemApn(const char *name, const char *username, const char *password);
  void setMqttServer(const char *host, uint16_t port);
  void setClientData(const char *userMqtt, const char* passMqtt, const char *clientId, unsigned short interval);
  void rxCallBack(void);
  void clearRxBuffer(void);
  void clearMqttBuffer(void);
  int sendCommand(const char *command, const char *reply, uint16_t delay);
  int init(void);
  void connect(void);
  void publish(char *topic, char *payload);
  void publishUint8(char *topic, uint8_t data);
  void publishUint16(char *topic, uint16_t data);
  void publishUint32(char *topic, uint32_t data);
  void publishFloat(char *topic, float payload);
  void publishDouble(char *topic, double data);
  void pingReq(void);
  void subscribe(char *topic);
  void receive(unsigned char *buf);

private:
  SIM800_t mModem;
  UART_HandleTypeDef *mUart;

  uint8_t mRxData = 0;
  uint8_t mRxBuffer[MQTT_MSG_MAXSIZE] = {0};
  uint16_t mRxIndex = 0;
  uint8_t mqttReceive = 0;
  char mqttBuffer[MQTT_MSG_MAXSIZE] = {0};
  uint16_t mqttIndex = 0;
};
