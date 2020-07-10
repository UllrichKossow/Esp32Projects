#ifndef __SH1160_H__
#define __SH1160_H__

#include <cstdint>

typedef uint8_t frame_buffer_t[132][8];


void sh1106_init();
void sh1106_print_line(int line, const char *text);
void sh1106_display_clear();

#endif
