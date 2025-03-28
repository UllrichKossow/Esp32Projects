#ifndef __SH1106_H__
#define __SH1106_H__

#include "driver/i2c_master.h"
#include <cstdint>

typedef uint8_t frame_buffer_t[132][8];

class SH1106 {
  private:
    static SH1106 *m_instance;
    SH1106() = default;
    bool m_chip_exists;
    i2c_master_bus_handle_t m_busHandle;
    i2c_master_dev_handle_t m_deviceHandle;
    void set_display_start_line(uint8_t start_line);

  public:
    static SH1106 *instance();

    SH1106(SH1106 const &) = delete;
    void operator=(SH1106 const &) = delete;

    void init();
    void release();
    void display_pattern();
    void display_clear();
    void print_line(int row, const char *text);
    void write_fb(frame_buffer_t *fb);
    void clear_fb(frame_buffer_t *fb);
    void sh1106_set_pixel(frame_buffer_t *fb, uint8_t x, uint8_t y, bool p);
    void draw_line(frame_buffer_t *fb, int x0, int y0, int x1, int y1); 
};

#endif
