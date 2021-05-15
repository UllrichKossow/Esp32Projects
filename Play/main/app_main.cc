// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-

/**
 * @file app_main.c
 * @brief Example application for the TM1637 LED segment display
 */

#include <stdio.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <time.h>
#include <sys/time.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <driver/gpio.h>
#include <esp_log.h>

#include "mqtt_client.h"

#include "sdkconfig.h"
#include "tm1637.h"
#include "SetSystemTime.h"

#define TAG "app"

const gpio_num_t LED_CLK = GPIO_NUM_4;
const gpio_num_t LED_DTA = GPIO_NUM_5;

bool connected = false;

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
	esp_mqtt_client_handle_t client = event->client;
	switch (event->event_id)
	{
	case MQTT_EVENT_CONNECTED:
		ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
		connected = true;
		break;
	case MQTT_EVENT_DISCONNECTED:
		ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
		connected = false;
		break;
	case MQTT_EVENT_ANY:
		ESP_LOGI(TAG, "MQTT_EVENT_ANY");
	case MQTT_EVENT_ERROR:
		ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
		break;
	case MQTT_EVENT_SUBSCRIBED:
		ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
		break;
	case MQTT_EVENT_UNSUBSCRIBED:
		ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
		break;
	case MQTT_EVENT_PUBLISHED:
		ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
		break;
	case MQTT_EVENT_DATA:
		ESP_LOGI(TAG, "MQTT_EVENT_DATA");
		break;
	case MQTT_EVENT_BEFORE_CONNECT:
		ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT");
		break;
	}
	return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
	ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
	mqtt_event_handler_cb((esp_mqtt_event_handle_t)event_data);
}

void lcd_tm1637_task(void *arg)
{
	tm1637_led_t *lcd = tm1637_init(LED_CLK, LED_DTA);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
	esp_mqtt_client_config_t mqtt_cfg = {
		.uri = "mqtt://berry",
	};
#pragma GCC diagnostic pop

	esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
	esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqtt_event_handler, client);
	esp_mqtt_client_start(client);
	setenv("TZ", "UTC", 1);
	tzset();

	tm1637_set_brightness(lcd, 1);
	while (true)
	{
		time_t now;
		struct tm timeinfo;
		time(&now);
		localtime_r(&now, &timeinfo);
		int time_number = 100 * timeinfo.tm_hour + timeinfo.tm_min;

		tm1637_set_number_lead_dot(lcd, time_number, true, timeinfo.tm_sec % 2 ? 0xFF : 0x00);
		//int64_t t = esp_timer_get_time();
		//ESP_LOGI(TAG, "t=%lli", t);
		if (connected && (timeinfo.tm_sec == 0))
		{
			int msg_id = esp_mqtt_client_publish(client, "/Play/time", ctime(&now), 0, 1, 0);
			ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
		}
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}

extern "C" void app_main();
void app_main()
{
	esp_log_level_set("*", ESP_LOG_INFO);
	esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
	esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
	esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
	esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
	esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);
	sync_time(true);

	xTaskCreate(&lcd_tm1637_task, "lcd_tm1637_task", 4096, NULL, 5, NULL);
}
