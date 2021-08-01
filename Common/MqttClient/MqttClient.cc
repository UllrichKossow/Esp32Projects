#include "MqttClient.h"
#include "mqtt_client.h"

#include "esp_log.h"

static const char *TAG = "MqttClient";

MqttClient* MqttClient::m_instance = nullptr;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    MqttClient *obj = reinterpret_cast<MqttClient *>(handler_args);
    obj->mqtt_event_handler_cb((esp_mqtt_event_handle_t)event_data);
}

MqttClient::MqttClient()
    : m_client(nullptr), m_connected(false)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
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

        m_client = esp_mqtt_client_init(&mqtt_cfg);
        esp_mqtt_client_register_event(m_client, MQTT_EVENT_ANY, mqtt_event_handler, this);
        esp_mqtt_client_start(m_client);
      }
}

esp_err_t MqttClient::mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{ //esp_mqtt_client_handle_t client = event->client;

    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGD(TAG, "MQTT_EVENT_CONNECTED");
        m_connected = true;
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "MQTT_EVENT_DISCONNECTED");
        m_connected = false;
        break;
    case MQTT_EVENT_ANY:
        ESP_LOGD(TAG, "MQTT_EVENT_ANY");
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGD(TAG, "MQTT_EVENT_ERROR");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGD(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGD(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGD(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGD(TAG, "MQTT_EVENT_DATA");
        break;
    case MQTT_EVENT_BEFORE_CONNECT:
        ESP_LOGD(TAG, "MQTT_EVENT_BEFORE_CONNECT");
        break;
    case MQTT_EVENT_DELETED:
        ESP_LOGD(TAG, "MQTT_EVENT_DELETED");
        break;
    }
    return ESP_OK;
}

void MqttClient::publish(const char *topic, const char *data, bool retain)
{
    if (!m_client)
    {
        init();
    }
    esp_mqtt_client_publish(m_client, topic, data, 0, 1, retain ? 1 : 0);
}
