//
// Created by fish on 2026/1/25.
//

#pragma once

#include "bsp/uart.h"

namespace rc::ht10 {
    typedef struct {
        int16_t rc_l[2], rc_r[2];
        int8_t swa, swb, swc, swd;
        int16_t vra, vrb;

        uint8_t status;
        uint32_t timestamp;
    } data_t;

    const data_t *data();
    void init(bsp_uart_e port);
}