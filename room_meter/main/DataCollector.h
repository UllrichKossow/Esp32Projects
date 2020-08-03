// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-

#ifndef __DATA_COLLECTOR_H_INCLUDED
#define __DATA_COLLECTOR_H_INCLUDED

#include <ctime>

class DataCollector
{
public:
    DataCollector(int interval);

    void AddValue(double y);

private:
    static const int m_len;
    double *m_values;
    int m_interval;
    int m_counter;
    timespec m_first, m_next;

    double m_avg_sum;
    int m_avg_cnt;
};

#endif
