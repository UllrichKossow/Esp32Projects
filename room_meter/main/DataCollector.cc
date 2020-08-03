// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-                                          
#include "DataCollector.h"
#include "time_helper.h"

const int DataCollector::m_len = 128;

DataCollector::DataCollector(int interval)
    : m_interval(interval), m_counter(0), m_first({0,0}), m_next({0,0}), m_avg_sum(0), m_avg_cnt(0)
{
    m_values = new double[m_len];
}


void DataCollector::AddValue(double y)
{
    if (timespec_isNull(m_first))
    {
	clock_gettime(CLOCK_MONOTONIC, &m_first);
	clock_gettime(CLOCK_MONOTONIC, &m_next); 
    }
    
    m_avg_sum += y;
    ++m_avg_cnt;

    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    if (timespec_compare(now, m_next) >= 0)
    {
	++m_counter;
	m_next.tv_sec += m_interval;
    }

}


