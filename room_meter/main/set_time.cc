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

#include <iomanip>
#include <sstream>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "esp_sntp.h"

#include "sh1106.h"

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
    ESP_ERROR_CHECK(example_connect());
    
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);

    ESP_LOGI(TAG, "We are smooth...");
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);    
    sntp_set_sync_interval(60*60*1000); //1h
    sntp_init();
}

void sntp_sync_time(struct timeval *tv)
{
    static timeval last_sync;
    static timeval sum_adj;
    
    ESP_LOGI(TAG, "Time is synchronized from custom code");
    if (sntp_get_sync_mode() == SNTP_SYNC_MODE_SMOOTH)
    {
        struct timeval outdelta;
        timeval delta, now;

        gettimeofday(&now, NULL);
        if (now.tv_sec < 1000)
        {
            settimeofday(tv,NULL);
            sntp_set_sync_status(SNTP_SYNC_STATUS_COMPLETED);
            ESP_LOGI(TAG, "Initial settimeofday()");
            last_sync = *tv;
	        sum_adj.tv_sec = 0;
            sum_adj.tv_usec = 0;
            return;
        }
        
        timersub(tv, &now, &delta);
        adjtime(&delta, &outdelta);

        ESP_LOGI(TAG, "Adjusting time ... outdelta = %li sec: %li ms: %li us",
                 (long)delta.tv_sec,
                 delta.tv_usec/1000,
                 delta.tv_usec%1000);

        timeval elapsed;
        timersub(tv, &last_sync, &elapsed);
        timeradd (&delta, &sum_adj, &sum_adj);
        
        last_sync = *tv;
        double sum_adj_sec = sum_adj.tv_sec + sum_adj.tv_usec/1000000.0;
        
        timespec now_mo;
        clock_gettime(CLOCK_MONOTONIC, &now_mo);
        double uptime = now_mo.tv_sec + now_mo.tv_nsec/1000000000.0;
        
        ostringstream s;
        //s << fixed << setprecision(3) << adj_sec << " " << setprecision(1) << (1000000*adj_sec/elapsed_sec);
        s << fixed << setprecision(3) << sum_adj_sec / uptime * 1000000 << " ppm";
        sh1106_print_line(7, s.str().c_str());
        
    }
    sntp_set_sync_status(SNTP_SYNC_STATUS_COMPLETED);
}


static void obtain_time(void)
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
    time(&now);
    localtime_r(&now, &timeinfo);
    ESP_LOGI(TAG, "sntp_get_sync_interval()=%i", sntp_get_sync_interval());
}


void sync_time()
{
    obtain_time();
}

