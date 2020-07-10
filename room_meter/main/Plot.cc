// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-                                          
#include "Plot.h"
#include "sh1106.h"

#include <cmath>

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
    double vmax, vmin;
    vmax = vmin = m_values[0];
    for (int i = 0; i < m_len; ++i)
    {
        vmax = fmax(vmax, m_values[i]);
        vmin = fmin(vmin, m_values[i]);
    }
    
    frame_buffer_t fb;
    sh1106_clear_fb(&fb);
    
    double dy = vmax-vmin;
    for (int x = 0; x < m_len; ++x)
    {
        int y = 64 - (int)(m_values[x] - vmin) / dy * 64;
    }
}
