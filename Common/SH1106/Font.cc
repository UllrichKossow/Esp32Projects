
#include "Font.h"

#include "esp_log.h"

#define F_8X14
#ifdef F_8X14
#include "LCD-fonts/8x14_horizontal_LSB_2.h"
#define FONT_WIDTH 8
#define FONT_HEIGHT 14
#endif

#ifdef F_10X16
#endif
#ifdef F_12X16
#endif

#ifdef F_16X26
#endif

static const char *TAG = "Font";

Font::Font() : m_font(reinterpret_cast<const char *>(font)), m_width(FONT_WIDTH), m_height(FONT_HEIGHT) {}

void Font::put(frame_buffer_t *fb, char ch, int x, int y) {
    if (((1 + x) * m_width) > 127) {
        ESP_LOGE(TAG, "x out of range");
    }
    if (((1 + y) * m_height) > 63) {
        ESP_LOGE(TAG, "x out of range");
    }

    for (int px = x; px < x + m_width; ++px) {
        for (int py = y; py < x + m_height; ++py) {
        }
    }
}

void Font::put(frame_buffer_t *fb, const char *s, int x, int y) {}

void Font::put(frame_buffer_t *fb, const std::string &s, int x, int y) {}
