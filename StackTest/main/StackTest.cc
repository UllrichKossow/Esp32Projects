#include "esp_log.h"
#include "freertos/FreeRTOS.h"

const char *TAG = "StackTest";

extern "C" void app_main(void);

void show(uint32_t number) {
    int x[1024];
    for (int i = 0; i < 1024; ++i) {
        x[i] = i;
    }
}

void app_main(void) {
    uint32_t n = 0;
    while (true) {
        if (n > 10) {
            show(n);
        }
        ESP_LOGI(TAG, "heap=%u", xPortGetFreeHeapSize());
        ESP_LOGI(TAG, "stack=%u", uxTaskGetStackHighWaterMark(nullptr));
        ++n;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
