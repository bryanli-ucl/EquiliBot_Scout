// led_frames.cpp
#include "led_matrix.hpp"
#include "Arduino_LED_Matrix.h"

namespace __details {

struct Font {
    uint8_t data[5];

    constexpr Font(uint8_t r0, uint8_t r1, uint8_t r2, uint8_t r3, uint8_t r4)
    : data{ r0, r1, r2, r3, r4 } {}

    constexpr Font() : data{ 0, 0, 0, 0, 0 } {}
};

constexpr Font digits_font[17] = {
    { 0b111, 0b101, 0b101, 0b101, 0b111 }, // 0
    { 0b010, 0b110, 0b010, 0b010, 0b111 }, // 1
    { 0b111, 0b001, 0b111, 0b100, 0b111 }, // 2
    { 0b111, 0b001, 0b111, 0b001, 0b111 }, // 3
    { 0b101, 0b101, 0b111, 0b001, 0b001 }, // 4
    { 0b111, 0b100, 0b111, 0b001, 0b111 }, // 5
    { 0b111, 0b100, 0b111, 0b101, 0b111 }, // 6
    { 0b111, 0b001, 0b001, 0b001, 0b001 }, // 7
    { 0b111, 0b101, 0b111, 0b101, 0b111 }, // 8
    { 0b111, 0b101, 0b111, 0b001, 0b111 }, // 9
    { 0b010, 0b101, 0b111, 0b101, 0b101 }, // A
    { 0b110, 0b101, 0b110, 0b101, 0b110 }, // B
    { 0b111, 0b100, 0b100, 0b100, 0b111 }, // C
    { 0b110, 0b001, 0b001, 0b001, 0b110 }, // D
    { 0b111, 0b100, 0b111, 0b100, 0b111 }, // E
    { 0b111, 0b100, 0b111, 0b100, 0b100 }, // F
    { 0b000, 0b000, 0b111, 0b000, 0b000 }, // -
};

constexpr uint32_t full_off[] = {
    0x00000000,
    0x00000000,
    0x00000000,
};

constexpr uint32_t full_on[] = {
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF
};

} // namespace __details

LED_Matrix::LED_Matrix()
: m_frame{ 0, 0, 0 }, m_matrix(new ArduinoLEDMatrix()) {
}

LED_Matrix::~LED_Matrix() {
    m_matrix->clear();
}

void LED_Matrix::draw_point(float rx, float ry) {
    set_bit(ry * 12 + rx, true);
}

void LED_Matrix::set_bit(int idx, bool val) {

    uint32_t& f = (reinterpret_cast<uint32_t*>(&m_frame))[0];
    uint32_t& s = (reinterpret_cast<uint32_t*>(&m_frame))[1];
    uint32_t& t = (reinterpret_cast<uint32_t*>(&m_frame))[2];

    if (idx < 32)
        f |= (val) << (31 - idx);
    else if (idx < 64)
        s |= (val) << (63 - idx);
    else
        t |= (val) << (95 - idx);
}

void LED_Matrix::generate_frame(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    /*
    frame combined with 96 bits (12 * 8)
    '-' when val = 16

    f31 f30 f29 | f28 f27 f26 | f25 f24 f23 | f22 f21 f20
    f19 f18 f17 | f16 f15 f14 | f13 f12 f11 | f10 f09 f08
    f07 f06 f05 | f04 f03 f02 | f01 f00 s31 | s30 s29 s28
    s27 s26 s25 | s24 s23 s22 | s21 s20 s19 | s18 s17 s16
    -----------------------------------------------------
    s15 s14 s13 | s12 s11 s10 | s09 s08 s07 | s06 s05 s04
    s03 s02 s01 | t00 t31 t30 | t29 t28 t27 | t26 t25 t24
    t23 t22 t21 | t20 t19 t18 | t17 t16 t15 | t14 t13 t12
    t11 t10 t09 | t08 t07 t06 | t05 t04 t03 | t02 t01 t00
    */

    auto& font = __details::digits_font;

    a = constrain(a, 0, 16);
    b = constrain(b, 0, 16);
    c = constrain(c, 0, 16);
    d = constrain(d, 0, 16);

    auto set_digit = [this, font](uint8_t dig, int r, int c) {
        for (int i = 0; i < 5; i++) {
            uint8_t p = font[dig].data[i];
            for (int j = 0; j < 3; j++) {
                set_bit((r + i) * 12 + (c + j), (p >> (2 - j)) & 0x01);
            }
        }
    };

    set_digit(a, 0, 0);
    set_digit(b, 0, 3);
    set_digit(c, 0, 6);
    set_digit(d, 0, 9);
}

void LED_Matrix::print(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3) {
    clean();
    generate_frame(d0, d1, d2, d3);
    show();
}

void LED_Matrix::print(int32_t val, int base) {
    uint32_t d0, d1, d2, d3;


    base = constrain(base, 2, 16);

    int32_t max_positive = base * base * base * base - 1;
    int32_t min_negative = -(base * base * base - 1);

    val = constrain(val, min_negative, max_positive);

    if (val < 0) {
        d0  = 16;
        val = -val;

        d3 = val % base;
        val /= base;
        d2 = val % base;
        val /= base;
        d1 = val % base;
    } else {
        d3 = val % base;
        val /= base;
        d2 = val % base;
        val /= base;
        d1 = val % base;
        val /= base;
        d0 = val % base;
    }

    print(d0, d1, d2, d3);
}

void LED_Matrix::show() {
    m_matrix->loadFrame((const uint32_t*)(&m_frame));
}

void LED_Matrix::begin() {
    m_matrix->begin();
}

void LED_Matrix::clean() {
    (reinterpret_cast<uint32_t*>(&m_frame))[0] = 0;
    (reinterpret_cast<uint32_t*>(&m_frame))[1] = 0;
    (reinterpret_cast<uint32_t*>(&m_frame))[2] = 0;
}

void LED_Matrix::clear() {
    m_matrix->loadFrame(__details::full_off);
}

void LED_Matrix::fill() {
    m_matrix->loadFrame(__details::full_on);
}

void LED_Matrix::flash(uint8_t times, uint16_t period) {
    for (uint8_t i = 0; i < times; i++) {
        m_matrix->loadFrame(__details::full_on);
        delay(period >> 1);
        m_matrix->loadFrame(__details::full_off);
        delay(period >> 1);
    }
}