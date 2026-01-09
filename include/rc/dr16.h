//
// Created by fish on 2026/1/9.
//

#pragma once

#include "bsp/uart.h"

namespace rc::dr16 {
    typedef union {
        uint16_t raw;
        struct __attribute__((packed)) {
            bool w : 1, s : 1, a : 1, d : 1, shift : 1, ctrl : 1, q : 1, e : 1,
            r : 1, f : 1, g : 1, z : 1, x : 1, c : 1, v : 1, b : 1;
        } key;
    } keyboard_u;
    typedef struct {
        int16_t rc_l[2], rc_r[2];
        int8_t s_l, s_r;
        int16_t mouse_x, mouse_y, mouse_z;
        uint8_t mouse_l, mouse_r;
        keyboard_u keyboard;
        int16_t reserved;
        uint32_t timestamp;
    } data_t;

    const data_t *data();
    void init(bsp_uart_e port);
}