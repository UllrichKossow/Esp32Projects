#ifndef FONT_H
#define FONT_H

#include "sh1106.h"
#include <string>

class Font
{
public:
    Font();

    void put(frame_buffer_t *fb, char ch, int x, int y);
    void put(frame_buffer_t *fb, const char *s, int x, int y);
    void put(frame_buffer_t *fb, const std::string &s, int x, int y);

    private:
    const char *m_font;
    int m_width;
    int m_height;
};

#endif // FONT_H
