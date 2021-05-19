#include "MqttClient.h"
#include "mqtt_client.h"

MqttClient* MqttClient::m_instance = nullptr;

MqttClient::MqttClient()
: m_client(nullptr)
{
    
}

MqttClient* MqttClient::instance()
{
    if (!m_instance)
    {
        m_instance = new MqttClient();
    }
    return m_instance;
}
 
void MqttClient::init()
{
    if (!m_client)
    {
  #pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
        esp_mqtt_client_config_t mqtt_cfg = {
                .uri = "mqtt://bpi",
        };
#pragma GCC diagnostic pop

        esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
        //esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqtt_event_handler, client);
        esp_mqtt_client_start(client);
  
    }
} 