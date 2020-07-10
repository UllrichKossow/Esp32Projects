// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-                                          
#include "Plot.h"

const int Plot::m_len = 128;

Plot::Plot(int div)
    :m_idx(0), m_div(div), m_val_cnt(0), m_current(0)
{
}


Plot::~Plot()
{
}

bool Plot::PushValue(double v)
{
    m_current += v;
    ++m_val_cnt;
    if (m_val_cnt % m_div == 0)
    {
	m_values[m_idx++] = m_current/m_div;
	m_current = 0;
	if (m_idx >= m_len)
	{
	    m_idx = 0;
	}
    }
    return m_val_cnt % m_div == 0;
}
