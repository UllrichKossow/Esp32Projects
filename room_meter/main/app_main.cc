// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-

#include "Bme280Controller.h"
#include "bme280_access.h"
#include "sh1106.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void sync_time(void);

void init()
{
    sh1106_init();
    sh1106_display_clear();
    sh1106_print_line(0, "Sync time...");
    sync_time();
}

void loop()
{
#if 0
    Bme280Controller b;
    b.init();
    b.start();
    while (true)
    {
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
#else
    read_bme();
#endif
}

extern "C" void app_main(void);
void app_main()
{
    init();
    while (true)
    {
        loop();
    }
}

