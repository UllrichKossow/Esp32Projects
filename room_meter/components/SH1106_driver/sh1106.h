#ifndef __SH1160_H__
#define __SH1160_H__

#include <cstdint>

typedef uint8_t frame_buffer_t[132][8];


void sh1106_init();
void sh1106_print_line(int line, const char *text);
void sh1106_display_clear();

void sh1106_set_pixel(frame_buffer_t *fb, uint8_t x, uint8_t y, bool p);
void sh1106_clear_fb(frame_buffer_t *fb);
void sh1106_write_fb(frame_buffer_t *fb);




#endif
