## [2.1.1] - 2021-08-07
- Add "DEACT" error;
- Correct printf placeholder params for unsigned;

## [2.1.0] - 2021-05-06
- Corrected delay on PUB SUB functions;
- Added publishing functions (MQTT_PubUint8, MQTT_PubUint16, MQTT_PubDouble, etc ) for numeric types:
    - uint8_t;
    - uint16_t;
    - uint32_t;
    - float;
    - double;

## [2.0.0] - 2021-01-26
- Added support for MQTT subscriptions.
- work is performed through the transparent mode;
- increased the buffer size to the maximum for SIM800 and now is 1460 bytes.
- Small bugfixes by code.
