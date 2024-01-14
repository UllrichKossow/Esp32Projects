#include <ctime>
#include <cstdint>
#include "esp_timer.h"

struct measure_t
{
    time_t timestamp;
    uint32_t sequence;
};

class MeasureController
{
private:
    enum state_t
    {
        S_IDLE,
        S_BAT_DETECTED,
        S_DISCHARGING,
        S_INTERRUPTED,
        S_BAT_EMPTY
    };

public:
    void Start();

    void timer_callback(void);

private:
    uint32_t m_currentInterval;
    esp_timer_handle_t m_timer;
};