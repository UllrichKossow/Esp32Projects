#ifndef __MQTT_CLIENT_H__
#define __MQTT_CLIENT_H__


#include "mqtt_client.h"


class MqttClient
{
private:
    MqttClient();
    static MqttClient *m_instance;

public:
    static MqttClient* instance();

private:    
    void init();

private:
    esp_mqtt_client_handle_t m_client;
};

#endif