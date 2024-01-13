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

#include "MqttClient.h"

#include "sdkconfig.h"
#include "tm1637.h"
#include "SetSystemTime.h"

#define TAG "app"

const gpio_num_t LED_CLK = GPIO_NUM_4;
const gpio_num_t LED_DTA = GPIO_NUM_5;

void lcd_tm1637_task(void *arg)
{
	tm1637_led_t *lcd = tm1637_init(LED_CLK, LED_DTA);
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
		if (timeinfo.tm_sec == 0)
		{
			char msg[80];
			strftime(msg, 80, "%F %T", &timeinfo);
			MqttClient::instance()->publish("SevenSegmentClock/event/time", msg);
			MqttClient::instance()->publish("SevenSegmentClock/last/time", msg, true);
		}
		vTaskDelay(1000 / portTICK_PERIOD_MS);
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
	sync_time(true);

	xTaskCreate(&lcd_tm1637_task, "lcd_tm1637_task", 4096, NULL, 5, NULL);
}
