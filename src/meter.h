#ifndef meter_h
#define meter_h
#include <Arduino.h>
#include <ArduinoLog.h>
#include <PubSubClient.h>

class Smartmeter
{
private:
    const char startChar = '/';
    const char stopChar = '!';

    char inByte;           // for reading from serial
    char smlMessage[1024]; // for storing the the isolated message. Mine was 280 bytes, but may vary...
    int smlIndex = 0;      // represents the actual position in smlMessage
    int stage = 0;         // defines what to do next. 0 = searchStart, 1 = searchStop, 2 = publish message
    char *curLine;
    const char *topic; // mqtt to publish to

    HardwareSerial *serial;
    PubSubClient *mqtt; // PubSub mqtt client

    void sendMessage(char *json, const char *topic);
    double getDoubleValue(String str);
    bool processData();

public:
    void setup(HardwareSerial *ser, PubSubClient *mqttClient, const char *top);
    void loop();
};
#endif
