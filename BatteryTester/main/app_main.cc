#include "MeasureController.h"
#include "SetSystemTime.h"

void init() { sync_time(true); }

void loop() {
    MeasureController m;
    m.Start();
    for (;;) {
    }
}

extern "C" void app_main(void);

void app_main(void) {
    init();
    loop();
}
