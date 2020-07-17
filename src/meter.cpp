#include "meter.h"

void Smartmeter::setup(HardwareSerial *ser, PubSubClient *mqttClient, const char *top)
{
    serial = ser;
    topic = top;
    mqtt = mqttClient;
}

void Smartmeter::loop()
{
    switch (stage)
    {
    case 0: // Search startChar
        if (serial->available())
        {
            inByte = serial->read();

            if (inByte == startChar)
            {
                smlMessage[0] = inByte;
                stage = 1;    // go to next stage
                smlIndex = 1; // reset index
            }
        }
        break;
    case 1: // Copy content until stopChar
        if (serial->available())
        {
            // Read and write incoming char to smlMessage buffer
            inByte = serial->read();
            smlMessage[smlIndex] = inByte;
            smlIndex++;

            // Check for buffer overflow
            if (smlIndex >= 1024)
            {
                Log.error("Buffer overflow! Reset to search for next startSequence" CR);
                smlIndex = 0;
                stage = 0;
            }

            // Check if end of message received
            if (inByte == stopChar)
            {
                smlMessage[smlIndex + 1] = '\0'; // add end of line to smlMessage buffer
                curLine = smlMessage;            // set first line of the content to iterate process step on
                stage = 2;
                Log.notice("received from meter, now processing for %s" CR, topic);
            }
        }
        break;
    case 2: // Process data and sent to MQTT
        if (processData())
        {
            // clean up for next data from smart meter
            memset(smlMessage, 0, sizeof(smlMessage));
            smlIndex = 0;
            stage = 0;
        }
        break;
    }
}

void Smartmeter::sendMessage(char *json, const char *topic)
{
    // mqttReconnect();
    mqtt->publish(topic, json);
    //Log.notice("sent %s" CR, json);
}

double Smartmeter::getDoubleValue(String str)
{
    int locStart = str.indexOf("(");
    if (locStart == -1)
        return 0.0;
    int locFinish = str.indexOf("*", locStart);
    if (locFinish == -1)
        return 0.0;
    return atof(str.substring(locStart + 1, locFinish).c_str());
}

bool Smartmeter::processData()
{
    // iterate over message from smart meter to extract relevant parameters
    if (curLine)
    {
        // temporarily terminate the current line
        char *nextLine = strchr(curLine, '\n');
        if (nextLine)
            *nextLine = '\0';

        // Find relevant parameters
        if (strlen(curLine) >= 10)
        {
            double v = -1000000.0;
            char json[30]; //

            //OBIS: 1-0:1.8.0 -> Kumulatives Register der aktiven Energie in kWh T1+T
            if (strncmp("1-0:1.8.0*255", curLine, 13) == 0)
            {
                v = getDoubleValue(curLine);
                if (fabs(v + 1000000.0) > 0.00001)
                {
                    sprintf(json, "{ \"E_total_1\" : %.4f }", v);
                    sendMessage(json, topic);
                }
            }

            //OBIS: 1-0:2.8.0 -> -A Enerige
            else if (strncmp("1-0:2.8.0*255", curLine, 13) == 0)
            {
                v = getDoubleValue(curLine);
                if (fabs(v + 1000000.0) > 0.00001)
                {
                    sprintf(json, "{ \"E_total_2\" : %.4f }", v);
                    sendMessage(json, topic);
                }
            }

            //OBIS: 1-0:16.7.0 -> Stromeffektivwer
            else if (strncmp("1-0:16.7.0*255", curLine, 14) == 0)
            {
                v = getDoubleValue(curLine);
                if (fabs(v + 1000000.0) > 0.00001)
                {
                    sprintf(json, "{ \"P\" : %.0f }", v);
                    sendMessage(json, topic);
                }
            }

            //OBIS: 1-0:32.7.0 -> Spannung L1, Auflösung 0.1 V
            else if (strncmp("1-0:32.7.0*255", curLine, 14) == 0)
            {
                v = getDoubleValue(curLine);
                if (fabs(v + 1000000.0) > 0.00001)
                {
                    sprintf(json, "{ \"U1\" : %.1f }", v);
                    sendMessage(json, topic);
                }
            }

            //OBIS: 1-0:52.7.0 -> Spannung L2, Auflösung 0.1 V
            else if (strncmp("1-0:52.7.0*255", curLine, 14) == 0)
            {
                v = getDoubleValue(curLine);
                if (fabs(v + 1000000.0) > 0.00001)
                {
                    sprintf(json, "{ \"U2\" : %.1f }", v);
                    sendMessage(json, topic);
                }
            }

            //OBIS: 1-0:72.7.0 -> Spannung L3, Auflösung 0.1 V
            else if (strncmp("1-0:72.7.0*255", curLine, 14) == 0)
            {
                v = getDoubleValue(curLine);
                if (fabs(v + 1000000.0) > 0.00001)
                {
                    sprintf(json, "{ \"U3\" : %.1f }", v);
                    sendMessage(json, topic);
                }
            }

            //OBIS: 1-0:31.7.0 -> Strom L1, Auflösung 0.01 A
            else if (strncmp("1-0:31.7.0*255", curLine, 14) == 0)
            {
                v = getDoubleValue(curLine);
                if (fabs(v + 1000000.0) > 0.00001)
                {
                    sprintf(json, "{ \"I1\" : %.2f }", v);
                    sendMessage(json, topic);
                }
            }

            //OBIS: 1-0:51.7.0 -> Strom L2, Auflösung 0.01 A
            else if (strncmp("1-0:51.7.0*255", curLine, 14) == 0)
            {
                v = getDoubleValue(curLine);
                if (fabs(v + 1000000.0) > 0.00001)
                {
                    sprintf(json, "{ \"I2\" : %.2f }", v);
                    sendMessage(json, topic);
                }
            }

            //OBIS: 1-0:71.7.0 -> Strom L3, Auflösung 0.01 A
            else if (strncmp("1-0:71.7.0*255", curLine, 14) == 0)
            {
                v = getDoubleValue(curLine);
                if (fabs(v + 1000000.0) > 0.00001)
                {
                    sprintf(json, "{ \"I3\" : %.2f }", v);
                    sendMessage(json, topic);
                }
            }

            //OBIS: 1-0:81.7.1 -> Phasenwinkel UL2 : UL1
            else if (strncmp("1-0:81.7.1*255", curLine, 14) == 0)
            {
                v = getDoubleValue(curLine);
                if (fabs(v + 1000000.0) > 0.00001)
                {
                    sprintf(json, "{ \"U2_U1_angle\" : %.0f }", v);
                    sendMessage(json, topic);
                }
            }

            //OBIS: 1-0:81.7.1 -> Phasenwinkel UL3 : UL1
            else if (strncmp("1-0:81.7.2*255", curLine, 14) == 0)
            {
                v = getDoubleValue(curLine);
                if (fabs(v + 1000000.0) > 0.00001)
                {
                    sprintf(json, "{ \"U3_U1_angle\" : %.0f }", v);
                    sendMessage(json, topic);
                }
            }

            //OBIS: 1-0:81.7.4 -> Phasenwinkel IL1 : UL1
            else if (strncmp("1-0:81.7.4*255", curLine, 14) == 0)
            {
                v = getDoubleValue(curLine);
                if (fabs(v + 1000000.0) > 0.00001)
                {
                    sprintf(json, "{ \"L1_PF\" : %.3f }", fabs(cos(v)));
                    sendMessage(json, topic);
                }
            }

            //OBIS: 1-0:81.7.15 -> Phasenwinkel IL2 : UL2
            else if (strncmp("1-0:81.7.15*255", curLine, 15) == 0)
            {
                v = getDoubleValue(curLine);
                if (fabs(v + 1000000.0) > 0.00001)
                {
                    sprintf(json, "{ \"L2_PF\" : %.3f }", fabs(cos(v)));
                    sendMessage(json, topic);
                }
            }

            //OBIS: 1-0:81.7.26 -> Phasenwinkel IL3 : UL3
            else if (strncmp("1-0:81.7.26*255", curLine, 15) == 0)
            {
                v = getDoubleValue(curLine);
                if (fabs(v + 1000000.0) > 0.00001)
                {
                    sprintf(json, "{ \"L3_PF\" : %.3f }", fabs(cos(v)));
                    sendMessage(json, topic);
                }
            }

            //OBIS: 1-0:14.7.0 -> Netz Frequenz in Hz
            else if (strncmp("1-0:14.7.0*255", curLine, 14) == 0)
            {
                v = getDoubleValue(curLine);
                if (fabs(v + 1000000.0) > 0.00001)
                {
                    sprintf(json, "{ \"f\" : %.1f }", v);
                    sendMessage(json, topic);
                }
            }
        }

        // then restore newline-char, just to be tidy
        if (nextLine)
            *nextLine = '\n';
        curLine = nextLine ? (nextLine + 1) : NULL;
    }

    return curLine == NULL;
}