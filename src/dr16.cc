//
// Created by fish on 2026/1/9.
//

#include "rc/dr16.h"

#include <cstring>
#include <bsp/time.h>

using namespace rc::dr16;

namespace rc::dr16 {
    static data_t data_;
}

const data_t* rc::dr16::data() {
    return &data_;
}

typedef struct {
    uint16_t ch0 : 11, ch1 : 11, ch2 : 11, ch3 : 11;
    uint8_t s1 : 2, s2 : 2;
    int16_t mouse_x : 16, mouse_y : 16, mouse_z : 16;
    uint8_t mouse_l : 8, mouse_r : 8;
    uint16_t keyboard : 16, reserved : 16;
} __attribute__((packed)) bsp_rc_raw_t;
static bsp_rc_raw_t raw;

static void callback(bsp_uart_e device, const uint8_t *data, size_t size) {
    static_assert(sizeof(bsp_rc_raw_t) * 8 == 128 + 16);
    memcpy(&raw, data, sizeof raw);
    data_.rc_l[0] = (int16_t) ((int16_t) raw.ch2 - 1024), data_.rc_l[1] = (int16_t) ((int16_t) raw.ch3 - 1024);
    data_.rc_r[0] = (int16_t) ((int16_t) raw.ch0 - 1024), data_.rc_r[1] = (int16_t) ((int16_t) raw.ch1 - 1024);
    data_.s_l = (int8_t) (raw.s2 == 3 ? 0 : raw.s2 == 2 ? -1 : 1);
    data_.s_r = (int8_t) (raw.s1 == 3 ? 0 : raw.s1 == 2 ? -1 : 1);
    data_.mouse_x = raw.mouse_x, data_.mouse_y = raw.mouse_y, data_.mouse_z = raw.mouse_z;
    data_.mouse_l = raw.mouse_l, data_.mouse_r = raw.mouse_r;
    data_.keyboard.raw = raw.keyboard, data_.reserved = (int16_t) (1024 - raw.reserved);
    data_.timestamp = bsp_time_get_ms();
}

void rc::dr16::init(bsp_uart_e port) {
    bsp_uart_set_callback(port, callback);
}
