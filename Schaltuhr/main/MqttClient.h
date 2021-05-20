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

    void publish(const char *topic, const char *data, bool retain = false);

    esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event);

private:
    void init();

private:
    esp_mqtt_client_handle_t m_client;
    bool m_connected;
};

#endif