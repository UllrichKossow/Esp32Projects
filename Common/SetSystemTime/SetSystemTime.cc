// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-
/* LwIP SNTP example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <time.h>
#include <sys/time.h>


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "esp_sntp.h"
#include "connect.h"


using namespace std;

static const char *TAG = "sntp";

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "%s  tv={%li, %li}", __PRETTY_FUNCTION__, tv->tv_sec, tv->tv_usec);
}


static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(my_wifi_connect());
    
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);

    ESP_LOGI(TAG, "We are smooth...");
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);    
    sntp_set_sync_interval(60*60*1000); //1h
    sntp_init();
}


static void obtain_time(bool keepSync)
{
    initialize_sntp();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo;
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    if (keepSync)
    {
        time(&now);
        localtime_r(&now, &timeinfo);
        ESP_LOGI(TAG, "sntp_get_sync_interval()=%i", sntp_get_sync_interval());
    }
    else
    {
        sntp_stop();
        my_wifi_disconnect();
    }
}


void sync_time(bool keepSync)
{
    obtain_time(keepSync);    
}

