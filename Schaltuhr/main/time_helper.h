// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-

#ifndef __TIME_HERLPER_H_INCLUDED
#define __TIME_HERLPER_H_INCLUDED

#include <ctime>


timespec timespec_add(const timespec &t1, const timespec &t2);
timespec timespec_sub(const timespec &t1, const timespec &t2);
timespec timespec_mult(const timespec &t, int m);
int timespec_compare(const timespec &t1, const timespec &t2);
bool timespec_isNull(const timespec &t);

uint64_t timespec_to_ms(const timespec &t);
uint64_t timespec_to_us(const timespec &t);

#endif

