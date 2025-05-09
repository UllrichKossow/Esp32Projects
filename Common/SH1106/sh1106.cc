// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-

#include "sh1106.h"
#include "Font.h"

#include <cstring>
#include <vector>

// #include "driver/i2c.h"
#include "driver/i2c_master.h"

#include "esp_err.h"
#include "freertos/FreeRTOS.h"

#include "sdkconfig.h" // generated by "make menuconfig"

// #define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

#include "font8x8_basic.h"
#include "sh1106_def.h"

static const char *TAG = "SH1106";

SH1106 *SH1106::m_instance = nullptr;

SH1106 *SH1106::instance() {
    if (m_instance == nullptr) {
        m_instance = new SH1106();
    }
    if (!i2c_master_get_bus_handle(I2C_NUM_0, &m_instance->m_busHandle) == ESP_OK) {
        ESP_LOGE(TAG, "i2c_master_get_bus_handle failed.");
    }
    return m_instance;
}

void SH1106::init() {
    i2c_device_config_t dev_cfg = {.dev_addr_length = I2C_ADDR_BIT_LEN_7, .device_address = OLED_I2C_ADDRESS, .scl_speed_hz = 100000};

    auto probe_result = i2c_master_probe(m_busHandle, dev_cfg.device_address, 1000);
    m_chip_exists = probe_result == ESP_OK;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(m_busHandle, &dev_cfg, &m_deviceHandle));
    std::vector<uint8_t> msg{OLED_CONTROL_BYTE_CMD_STREAM,

                             OLED_CMD_SET_CHARGE_PUMP_CTRL,
                             OLED_CMD_SET_CHARGE_PUMP_ON,
                             OLED_CMD_SET_SEGMENT_REMAP_INVERSE, // reverse left-right mapping
                             OLED_CMD_SET_COM_SCAN_MODE_REVERSE, // reverse up-bottom mapping

                             OLED_CMD_SET_CONTRAST,
                             0x10,
                             OLED_CMD_DISPLAY_ON,

                             0x00,                            // reset column low bits
                             0x10,                            // reset column high bits
                             0xB0,                            // reset page
                             OLED_CMD_SET_DISPLAY_START_LINE, // set start line
                             OLED_CMD_SET_DISPLAY_OFFSET,
                             0x00

    };
    i2c_master_transmit(m_deviceHandle, msg.data(), msg.size(), 1000);
}

void SH1106::set_display_start_line(uint8_t start_line) {
    // REQUIRES:
    //   0 <= start_line <= 63
    if (start_line <= 63) {
        uint8_t cmd = OLED_CMD_SET_DISPLAY_START_LINE | start_line;
        i2c_master_transmit(m_deviceHandle, &cmd, 1, 1000);
    }
}

void SH1106::display_pattern() {
    for (uint8_t i = 0; i < 8; i++) {
        uint8_t v = static_cast<uint8_t>(0xB0 | i);
        std::vector<uint8_t> msg{OLED_CONTROL_BYTE_CMD_SINGLE, v, OLED_CONTROL_BYTE_DATA_STREAM};
        for (uint8_t j = 0; j < 132; j++) {
            msg.push_back(0xFF >> (j % 8));
        }
        i2c_master_transmit(m_deviceHandle, msg.data(), msg.size(), 1000);
    }
}

void SH1106::display_clear() {
    for (uint8_t i = 0; i < 8; i++) {
        uint8_t v = static_cast<uint8_t>(0xB0 | i);
        std::vector<uint8_t> msg{OLED_CONTROL_BYTE_CMD_SINGLE, v, OLED_CONTROL_BYTE_DATA_STREAM};
        msg.resize(msg.size() + 132);
        i2c_master_transmit(m_deviceHandle, msg.data(), msg.size(), 1000);
    }

    std::vector<uint8_t> msg = {OLED_CONTROL_BYTE_CMD_STREAM, 0x00, 0x10, 0xB0};
    i2c_master_transmit(m_deviceHandle, msg.data(), msg.size(), 1000);
}

void SH1106::print_line(int row, const char *text) {
    uint8_t text_len = strlen(text);
    char line[18];
    strncpy(line, text, 16);
    while (strlen(line) < 16)
        strcat(line, " ");

    uint8_t cur_page = static_cast<uint8_t>(0xB0 | row);
    std::vector<uint8_t> msg{OLED_CONTROL_BYTE_CMD_STREAM, 0x04, 0x10, cur_page};
    i2c_master_transmit(m_deviceHandle, msg.data(), msg.size(), 1000);

    for (uint8_t i = 0; i < 16; i++) {
        std::vector<uint8_t> msg{OLED_CONTROL_BYTE_DATA_STREAM};
        uint8_t c = line[i];
        for (int j = 0; j < 8; ++j) {
            msg.push_back(font8x8_basic_tr[c][j]);
        }
        i2c_master_transmit(m_deviceHandle, msg.data(), msg.size(), 1000);
    }
}

//--------------------------------------------------------------------------------
void SH1106::write_fb(frame_buffer_t *fb) {
    for (uint8_t i = 0; i < 8; i++) {
        uint8_t v = static_cast<uint8_t>(0xB0 | i);
        uint8_t buffer[132 + 3] = {OLED_CONTROL_BYTE_CMD_SINGLE, v, OLED_CONTROL_BYTE_DATA_STREAM};
        for (uint8_t j = 0; j < 132; j++) {
            buffer[j + 3] = (*fb)[j][i];
        }
        i2c_master_transmit(m_deviceHandle, buffer, sizeof(buffer), 1000);
    }
}

//--------------------------------------------------------------------------------
void SH1106::clear_fb(frame_buffer_t *fb) { memset(fb, 0, 8 * 132); }

//--------------------------------------------------------------------------------
void SH1106::sh1106_set_pixel(frame_buffer_t *fb, uint8_t x, uint8_t y, bool p) {
    if ((x > 127) || (y > 63)) {
        ESP_LOGE(TAG, "sh1106_set_pixel %i, %i out of range.", x, y);
        return;
    }
    uint8_t page;
    uint8_t column;
    uint8_t bit;

    page = y / 8;
    column = x + 4;
    bit = y % 8;
    // ESP_LOGI(tag, "page=%i column=%i bit=%i", page, column, bit);

    uint8_t v = (*fb)[column][page];
    if (p)
        v |= 1 << bit;
    else
        v &= ~(1 << bit);

    (*fb)[column][page] = v;
}

//--------------------------------------------------------------------------------
void SH1106::draw_line(frame_buffer_t *fb, int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    int e2; /* error value e_xy */
    // ESP_LOGD(tag, "%s %i %i %i %i", __FUNCTION__, x0, y0, x1, y1);
    while (true) {
        sh1106_set_pixel(fb, x0, y0, true);
        if (x0 == x1 && y0 == y1)
            break;
        e2 = 2 * err;
        if (e2 > dy) {
            err += dy;
            x0 += sx;
        } /* e_xy+e_x > 0 */
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        } /* e_xy+e_y < 0 */
    }
}

void put(frame_buffer_t *fb, char ch, int x, int y) {
    Font f;
    f.put(fb, ch, x, y);
}
