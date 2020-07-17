// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-                                          
#include "Plot.h"

#include <cmath>
#include "esp_log.h"
#include "sh1106.h"

static const char *TAG = "plot";

const int Plot::m_len = 128;

Plot::Plot(int div)
    :m_idx(0), m_div(div), m_val_cnt(0), m_values(nullptr), m_current(0)
{
}


Plot::~Plot()
{
    if (m_values)
    {
        delete[] m_values;
        m_values = nullptr;
    }
}

bool Plot::PushValue(double v)
{
    m_current += v;
    ++m_val_cnt;
    if (m_val_cnt % m_div == 0)
    {
        if (!m_values)
        {
            m_values = new double[m_len];
            for (int i = 0; i < m_len; ++i)
            {
                m_values[i] = m_current/m_div;
            }
        }
        m_values[m_idx++] = m_current/m_div;
        m_current = 0;
        if (m_idx >= m_len)
        {
            m_idx = 0;
        }
    }
    return m_val_cnt % m_div == 0;
}


void Plot::Show()
{
    if (!m_values)
	return;
    
    double vmax, vmin;
    vmax = vmin = m_values[0];
    for (int i = 0; i < m_len; ++i)
    {
        vmax = fmax(vmax, m_values[i]);
        vmin = fmin(vmin, m_values[i]);
    }
    
    frame_buffer_t fb;
    sh1106_clear_fb(&fb);

    ESP_LOGD(TAG, "vmin=%f vmax=%f", vmin, vmax); 
    
    double dy = vmax-vmin;
    for (int x = 0; x < m_len; ++x)
    {
        int y = 63 - (int)(m_values[x] - vmin) / dy * 63;
	ESP_LOGD(TAG, "x=%i y=%i", x, y);
	sh1106_set_pixel(&fb, x, y, true);
    }
    sh1106_write_fb(&fb);
}
