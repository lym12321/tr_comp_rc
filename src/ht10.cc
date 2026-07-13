//
// Created by fish on 2026/1/25.
//

#include "rc/ht10.h"

#include <algorithm>
#include <cstring>

#include "bsp/time.h"
#include "bsp/sys.h"

using namespace rc::ht10;

namespace rc::ht10 {
    static data_t data_;
}

const data_t* rc::ht10::data() {
    return &data_;
}

data_t rc::ht10::state() {
    const unsigned long state = bsp_sys_enter_critical();
    const data_t copy = data_;
    bsp_sys_exit_critical(state);
    return copy;
}

typedef struct {
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
} __attribute__((packed)) ht10_raw_t;
static uint8_t rx_stream[sizeof(ht10_raw_t) * 2];
static size_t rx_size;

static void callback(bsp_uart_e device, const uint8_t *data, size_t size) {
    static_assert(sizeof(ht10_raw_t) * 8 == 11 * 16 + 8 * 3);
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
    while (offset + sizeof(ht10_raw_t) <= rx_size) {
        ht10_raw_t raw;
        memcpy(&raw, rx_stream + offset, sizeof raw);
        if (raw.head != 0x0f || raw.end != 0x00) {
            offset++;
            continue;
        }

        data_t next = {};
        next.rc_l[0] = static_cast <int16_t> (raw.l_x - 992), next.rc_l[1] = static_cast <int16_t> (raw.l_y - 992);
        next.rc_r[0] = static_cast <int16_t> (raw.r_x - 992), next.rc_r[1] = static_cast <int16_t> (raw.r_y - 992);
        next.swa = static_cast <int8_t> (raw.swa == 992 ? 0 : raw.swa == 192 ? 1 : -1);
        next.swb = static_cast <int8_t> (raw.swb == 992 ? 0 : raw.swb == 192 ? 1 : -1);
        next.swc = static_cast <int8_t> (raw.swc == 992 ? 0 : raw.swc == 192 ? 1 : -1);
        next.swd = static_cast <int8_t> (raw.swd == 992 ? 0 : raw.swd == 192 ? 1 : -1);
        next.vra = static_cast <int16_t> (raw.vra - 992);
        next.vrb = static_cast <int16_t> (raw.vrb - 992);
        next.status = raw.status;
        next.timestamp = bsp_time_get_ms();

        const unsigned long state = bsp_sys_enter_critical();
        data_ = next;
        bsp_sys_exit_critical(state);
        offset += sizeof(ht10_raw_t);
    }

    if (offset > 0) {
        memmove(rx_stream, rx_stream + offset, rx_size - offset);
        rx_size -= offset;
    }
}

void rc::ht10::init(bsp_uart_e port) {
    bsp_uart_set_callback(port, callback);
}
