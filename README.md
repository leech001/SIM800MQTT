# STM32 HAL library for SIM800 (GPRS) release MQTT client  over AT command

Simple C library (STM32 HAL) for working with the MQTT protocol through AT commands of the SIM800 (GPRS) module
https://aliexpress.ru/item/32284560394.html?spm=a2g0s.9042311.0.0.34a833edF2cSsx

Configure STM32CubeMX by setting "General peripheral Initalizion as a pair of '.c / .h' file per peripheral" in the project settings.
![photo](https://raw.githubusercontent.com/leech001/SIM800MQTT/master/img/ch_pair.png)
Remember to enable global interrupts for your UART.
![photo](https://raw.githubusercontent.com/leech001/SIM800MQTT/master/img/nvic.png)
Copy the library header and source file to the appropriate project directories (Inc, Src).
Configure the UART port where your module is connected in the MQTTSim800.h file.
```
#define UART_SIM800 &huart2
#define FREERTOS 0
#define CMD_DELAY 2000
```
In the head file of your project (main.c), include the header file
```
/ * USER CODE BEGIN Includes * /
#include "MQTTSim800.h"
/ * USER CODE END Includes * /
```
In the head file of your project (main.c), create SIM800 structure
```
/* USER CODE BEGIN PV */
SIM800_t SIM800;
/* USER CODE END PV */
```
add function call Sim800_RxCallBack () to interrupt UART
```
/ * USER CODE BEGIN 0 * /
void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart) {
    if (huart == UART_SIM800) {
        Sim800_RxCallBack();
    }
}
/ * USER CODE END 0 * /
```
Fill structure in the main (void) function section
```
/* USER CODE BEGIN 2 */
    SIM800.sim.apn = "internet";
    SIM800.sim.apn_user = "";
    SIM800.sim.apn_pass = "";
    SIM800.mqttServer.host = "mqtt.mqtt.ru";
    SIM800.mqttServer.port = 1883;
    SIM800.mqttClient.username = "user";
    SIM800.mqttClient.pass = "pass";
    SIM800.mqttClient.clientID = "TestSub";
    SIM800.mqttClient.keepAliveInterval = 120;
/* USER CODE END 2 */
```
add the module initialization code in the main (void) function section
```
/ * USER CODE BEGIN 2 * /
    MQTT_Init();
/ * USER CODE END 2 * /
```
add Subscription status
```
/ * USER CODE BEGIN 2 * /
    uint8_t sub = 0;
/ * USER CODE END 2 * /
```
add a send topic "STM32" and value "test" to the server in an infinite while (1) loop, for example every 1 seconds.
add subscribe to topic "test" 
```
 /* Infinite loop */
 /* USER CODE BEGIN WHILE */
 while (1) {
     if (SIM800.mqttServer.connect == 0) {
         MQTT_Init();
         sub = 0;
     }
     if (SIM800.mqttServer.connect == 1) {
         if(sub == 0){
             MQTT_Sub("test");
             sub = 1;
         }
         MQTT_Pub("STM32", "test");

         if(SIM800.mqttReceive.newEvent) {
             unsigned char *topic = SIM800.mqttReceive.topic;
             int payload = atoi(SIM800.mqttReceive.payload);
             SIM800.mqttReceive.newEvent = 0;
         }
     }
     HAL_Delay(1000);
     /* USER CODE END WHILE */
```
This completes the setup.
Check in loop SIM800.mqttReceive.newEvent for new publish events. Set to 0 (zero) while use new data.

The library also supports sending zero messages to the server to maintain a connection.
```
MQTT_PingReq();
```
The library is implemented based on the code for generating MQTT packages of the project https://github.com/eclipse/paho.mqtt.embedded-c/tree/master/MQTTPacket

The library testing with https://github.com/r2axz/bluepill-serial-monster