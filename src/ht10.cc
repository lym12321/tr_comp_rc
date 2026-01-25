//
// Created by fish on 2026/1/25.
//

#include "rc/ht10.h"

#include <cstring>

#include "bsp/time.h"

using namespace rc::ht10;

namespace rc::ht10 {
    static data_t data_;
}

const data_t* rc::ht10::data() {
    return &data_;
}

static struct {
    uint8_t head;

    uint16_t r_x : 11;          // CH1 右摇杆 左右
    uint16_t r_y : 11;          // CH2 右摇杆 上下
    uint16_t l_y  : 11;         // CH3 左摇杆 上下（油门/俯仰）
    uint16_t l_x  : 11;         // CH4 左摇杆 左右（偏航）

    uint16_t swa : 11;          // CH5 三段开关A
    uint16_t swb : 11;          // CH6 三段开关B
    uint16_t swc : 11;          // CH7 三段开关C
    uint16_t swd : 11;          // CH8 三段开关D

    uint16_t vra : 11;          // CH9 左旋钮
    uint16_t vrb : 11;          // CH10 右旋钮

    uint16_t ch11 : 11;
    uint16_t ch12 : 11;
    uint16_t ch13 : 11;
    uint16_t ch14 : 11;
    uint16_t ch15 : 11;
    uint16_t ch16 : 11;

    uint8_t status, end;
} __attribute__((packed)) raw;

static void callback(bsp_uart_e device, const uint8_t *data, size_t size) {
    static_assert(sizeof(raw) * 8 == 11 * 16 + 8 * 3);
    memcpy(&raw, data, sizeof raw);
    if (raw.head != 0x0f or raw.end != 0x00) return;

    data_.rc_l[0] = static_cast <int16_t> (raw.l_x - 992), data_.rc_l[1] = static_cast <int16_t> (raw.l_y - 992);
    data_.rc_r[0] = static_cast <int16_t> (raw.r_x - 992), data_.rc_r[1] = static_cast <int16_t> (raw.r_y - 992);
    data_.swa = static_cast <int8_t> (raw.swa == 992 ? 0 : raw.swa == 192 ? 1 : -1);
    data_.swb = static_cast <int8_t> (raw.swb == 992 ? 0 : raw.swb == 192 ? 1 : -1);
    data_.swc = static_cast <int8_t> (raw.swc == 992 ? 0 : raw.swc == 192 ? 1 : -1);
    data_.swd = static_cast <int8_t> (raw.swd == 992 ? 0 : raw.swd == 192 ? 1 : -1);
    data_.vra = static_cast <int16_t> (raw.vra - 992);
    data_.vrb = static_cast <int16_t> (raw.vrb - 992);

    data_.status = raw.status;
    data_.timestamp = bsp_time_get_ms();
}

void rc::ht10::init(bsp_uart_e port) {
    bsp_uart_set_callback(port, callback);
}