// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-                                          
#include "time_helper.h"
#include <cstdint>


//------------------------------------------------------------------------------
timespec timespec_add(const timespec &t1, const timespec &t2)
{
    timespec tmp = t1;
    tmp.tv_sec += t2.tv_sec;
    tmp.tv_nsec += t2.tv_nsec;
    while (tmp.tv_nsec >= 1000000000)
    {
	tmp.tv_nsec -= 1000000000;
	tmp.tv_sec += 1;
    }
    return tmp;
}

//------------------------------------------------------------------------------
timespec timespec_sub(const timespec &t1, const timespec &t2)
{
    timespec tmp = t1;
    if (t2.tv_nsec > t1.tv_nsec)
    {
	tmp.tv_nsec = tmp.tv_nsec - t2.tv_nsec + 1000000000;
	tmp.tv_sec = tmp.tv_sec - t2.tv_sec + 1;
    }
    else
    {
        tmp.tv_nsec = tmp.tv_nsec - t2.tv_nsec;
        tmp.tv_sec = tmp.tv_sec - t2.tv_sec;
    }
    return tmp;
}

//------------------------------------------------------------------------------
timespec timespec_mult(const timespec &t, int m)
{
    timespec tmp = t;
    tmp.tv_sec *= m;
    uint64_t nsecs = (tmp.tv_nsec * m) % 1000000000;
    int extra_sec = (tmp.tv_nsec * m) / 1000000000;
    tmp.tv_nsec = nsecs;
    tmp.tv_sec += extra_sec;
    return tmp;
}

//------------------------------------------------------------------------------
int timespec_compare(const timespec &t1, const timespec &t2)
{
    if (t1.tv_sec > t2.tv_sec)
        return 1;
    else if (t1.tv_sec < t2.tv_sec)
        return -1;
    else
    {
        if (t1.tv_nsec > t2.tv_nsec)
            return 1;
        else if (t1.tv_nsec < t2.tv_nsec)
            return -1;
        else
            return 0;
    }
}

//------------------------------------------------------------------------------
bool timespec_isNull(const timespec t)
{
    return (t.tv_sec == 0) && (t.tv_nsec == 0);
}
