// led_matrix.hpp
#pragma once

#include <array>
#include <initializer_list>
#include <memory>
#include <stdint.h>
#include <tuple>

class LED_Matrix {
    private:
    struct {
        uint32_t fi;
        uint32_t sc;
        uint32_t tr;
    } m_frame;
    std::shared_ptr<class ArduinoLEDMatrix> m_matrix;

    public:
    void generate_frame(uint8_t a, uint8_t b, uint8_t c, uint8_t d);

    LED_Matrix();
    ~LED_Matrix();

    void draw_point(float x, float y);

    void set_bit(int idx, bool val);

    void print(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);
    void print(int32_t value, int base = 10);

    void show();
    void begin();
    void clean();
    void clear();
    void fill();
    void flash(uint8_t times = 3, uint16_t duration = 100);
};
