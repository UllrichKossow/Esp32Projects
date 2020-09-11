// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-

#include "Bme280Controller.h"
#include "bme280_access.h"
#include "sh1106.h"
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
    read_bme();
    Bme280Controller b;
    b.start();
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

