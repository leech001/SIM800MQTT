# STM32 HAL library for SIM800 release MQTT client over AT command

## English note
Simple C library (STM32 HAL) for working with the MQTT protocol through AT commands of the SIM800 module
https://aliexpress.ru/item/32284560394.html?spm=a2g0s.9042311.0.0.34a833edF2cSsx

Configure STM32CubeMX by setting "General peripheral Initalizion as a pair of '.c / .h' file per peripheral" in the project settings.
Remember to enable global interrupts for your UART.
Copy the library header and source file to the appropriate project directories (Inc, Src).
Configure the UART port where your module is connected in the MQTTSim800.h file.
```
#define UART_SIM800 & huart1
#define FREERTOS 0
#define CMD_DELAY 2000
```
In the head file of your project (main.c), include the header file
```
/ * USER CODE BEGIN Includes * /
#include "MQTTSim800.h"
/ * USER CODE END Includes * /
```
add function call Sim800_RxCallBack () to interrupt UART
```
/ * USER CODE BEGIN 0 * /
void HAL_UART_RxCpltCallback (UART_HandleTypeDef * huart) {
    if (huart == & huart1) {
        Sim800_RxCallBack ();
    }
}
/ * USER CODE END 0 * /
```
add the module initialization code and parameters for connecting to the MQTT server in the main (void) function section
```
/ * USER CODE BEGIN 2 * /

  SIM800_Init ();
  MQTT_Connect ("APN", "APN NAME", "APN PASS", "HOST", PORT, "MQTT_USER", "MQTT_PASS", "CLIENTID", KEEPALIVEINTERVAL);

 / * USER CODE END 2 * /
```
add a topic to the server in an infinite while (1) loop, for example every 10 seconds.
```
/ * Infinite loop * /
  / * USER CODE BEGIN WHILE * /
  while (1)
  {
MQTT_Pub ("/ STM32", "Test");
HAL_Delay (10000);
    / * USER CODE END WHILE * /

    / * USER CODE BEGIN 3 * /
  }
  / * USER CODE END 3 * /
```
This completes the setup. The library also supports sending zero messages to the server to maintain a connection.
```
MQTT_PingReq ();
```
The library is implemented based on the code for generating MQTT packages of the project https://github.com/eclipse/paho.mqtt.embedded-c/tree/master/MQTTPacket

## Russian note
Простая библиотека на С (STM32 HAL) для работы с протоколом MQTT через AT команды модуля SIM800
https://aliexpress.ru/item/32284560394.html?spm=a2g0s.9042311.0.0.34a833edF2cSsx

Сконфигурируйте STM32CubeMX установив "General peripheral Initalizion as a pair of '.c/.h' file per peripheral" в настройках проекта.
Не забудьте включить глобальные прерывания для вашего UART.
Скопируйте заголовочный и исходный файл библиотеки в соответствующие директории проекта (Inc, Src).
Сконфигурируйте UART порт куда подключен ваш модуль в MQTTSim800.h файле.
```
#define UART_SIM800 &huart1
#define FREERTOS    0
#define CMD_DELAY   2000
```
в головном файле вашего проекта (main.c) подключите заголовочный файл
```
/ * USER CODE BEGIN Includes * /
#include "MQTTSim800.h"
/ * USER CODE END Includes * /
```
добавьте функцию вызова Sim800_RxCallBack() по прерыванию UART
```
/* USER CODE BEGIN 0 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if(huart == &huart1){
        Sim800_RxCallBack();
    }
}
/* USER CODE END 0 */
```
добавьте в секцию функции main(void) код инициализации модуля и параметры соединения с MQTT сервером
```
/* USER CODE BEGIN 2 */

  SIM800_Init();
  MQTT_Connect("APN", "APN NAME", "APN PASS", "HOST", PORT, "MQTT_USER", "MQTT_PASS", "CLIENTID", KEEPALIVEINTERVAL);

 /* USER CODE END 2 */
```
добавьте в бесконечный цикл while (1) отправку топика на сервер, например каждые 10 секунд.
```
/* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	MQTT_Pub("/STM32", "Test");
	HAL_Delay(10000);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
```
На этом настройка закончена. Также в библиотеке поддерживается отправка на сервер нулевых сообщений для поддержания соединения
```
MQTT_PingReq();
```
Библиотека реализована на основании кода генерации MQTT пакетов проекта https://github.com/eclipse/paho.mqtt.embedded-c/tree/master/MQTTPacket