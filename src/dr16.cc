//
// Created by fish on 2026/1/9.
//

#include "rc/dr16.h"

#include <algorithm>
#include <cstring>
#include <bsp/time.h>
#include <bsp/sys.h>

using namespace rc::dr16;

namespace rc::dr16 {
    static data_t data_;
}

const data_t* rc::dr16::data() {
    return &data_;
}

data_t rc::dr16::state() {
    const unsigned long state = bsp_sys_enter_critical();
    const data_t copy = data_;
    bsp_sys_exit_critical(state);
    return copy;
}

typedef struct {
    uint16_t ch0 : 11, ch1 : 11, ch2 : 11, ch3 : 11;
    uint8_t s1 : 2, s2 : 2;
    int16_t mouse_x : 16, mouse_y : 16, mouse_z : 16;
    uint8_t mouse_l : 8, mouse_r : 8;
    uint16_t keyboard : 16, reserved : 16;
} __attribute__((packed)) bsp_rc_raw_t;
static uint8_t rx_stream[sizeof(bsp_rc_raw_t) * 2];
static size_t rx_size;

static void callback(bsp_uart_e device, const uint8_t *data, size_t size) {
    static_assert(sizeof(bsp_rc_raw_t) * 8 == 128 + 16);
    if (data == nullptr || size == 0) return;
    if (size > sizeof(rx_stream)) {
        data += size - sizeof(rx_stream);
        size = sizeof(rx_stream);
        rx_size = 0;
    } else if (size > sizeof(rx_stream) - rx_size) {
        const size_t keep = std::min(rx_size, sizeof(rx_stream) - size);
        memmove(rx_stream, rx_stream + rx_size - keep, keep);
        rx_size = keep;
    }
    memcpy(rx_stream + rx_size, data, size);
    rx_size += size;

    size_t offset = 0;
    while (offset + sizeof(bsp_rc_raw_t) <= rx_size) {
        bsp_rc_raw_t raw;
        memcpy(&raw, rx_stream + offset, sizeof raw);
        if (raw.ch0 < 364 || raw.ch0 > 1684 || raw.ch1 < 364 || raw.ch1 > 1684 ||
            raw.ch2 < 364 || raw.ch2 > 1684 || raw.ch3 < 364 || raw.ch3 > 1684 ||
            raw.s1 < 1 || raw.s1 > 3 || raw.s2 < 1 || raw.s2 > 3) {
            offset++;
            continue;
        }

        data_t next = {};
        next.rc_l[0] = (int16_t) ((int16_t) raw.ch2 - 1024), next.rc_l[1] = (int16_t) ((int16_t) raw.ch3 - 1024);
        next.rc_r[0] = (int16_t) ((int16_t) raw.ch0 - 1024), next.rc_r[1] = (int16_t) ((int16_t) raw.ch1 - 1024);
        next.s_l = (int8_t) (raw.s2 == 3 ? 0 : raw.s2 == 2 ? -1 : 1);
        next.s_r = (int8_t) (raw.s1 == 3 ? 0 : raw.s1 == 2 ? -1 : 1);
        next.mouse_x = raw.mouse_x, next.mouse_y = raw.mouse_y, next.mouse_z = raw.mouse_z;
        next.mouse_l = raw.mouse_l, next.mouse_r = raw.mouse_r;
        next.keyboard.raw = raw.keyboard, next.reserved = (int16_t) (1024 - raw.reserved);
        next.timestamp = bsp_time_get_ms();

        const unsigned long state = bsp_sys_enter_critical();
        data_ = next;
        bsp_sys_exit_critical(state);
        offset += sizeof(bsp_rc_raw_t);
    }

    if (offset > 0) {
        memmove(rx_stream, rx_stream + offset, rx_size - offset);
        rx_size -= offset;
    }
}

void rc::dr16::init(bsp_uart_e port) {
    bsp_uart_set_callback(port, callback);
}
