/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Chester Tseng
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include "py/mpstate.h"
#include "extmod/misc.h"
#include "lib/utils/pyexec.h"
#include "py/ringbuf.h"

#include "sys/clock.h"
#include "debug-uart.h"
#include "serial_api.h"
#include "drv_uart.h"

#include "mphal.h"

STATIC byte input_buf_array[256];
ringbuf_t input_buf = {input_buf_array, sizeof(input_buf_array)};

void serial_callback(void *arg) {
    S32 input;
    while (1) {
        input = drv_uart_rx(SSV6XXX_UART0);
        if (input == -1)
        {
            break;
        }
        else {
            printf("input is %d\r\n", input);
        }
    }
}

void mphal_init(void) {
    SerialInit(BAUD_115200, serial_callback, NULL);
}

void mp_hal_delay_us(uint32_t us) {
}

int mp_hal_stdin_rx_chr(void) {
    for (;;) {
        int c = ringbuf_get(&input_buf);
        if (c != -1) {
            return c;
        }
        mp_hal_delay_us(1);
    }
}

void mp_hal_stdout_tx_char(char c) {
    dbg_putchar(c);
    //mp_uos_dupterm_tx_strn(&c, 1);
}

void mp_hal_stdout_tx_str(const char *str) {
    while (*str) {
        mp_hal_stdout_tx_char(*str++);
    }
}

void mp_hal_stdout_tx_strn(const char *str, uint32_t len) {
    while (len--) {
        mp_hal_stdout_tx_char(*str++);
    }
}

void mp_hal_stdout_tx_strn_cooked(const char *str, uint32_t len) {
    while (len--) {
        if (*str == '\n') {
            mp_hal_stdout_tx_char('\r');
        }
        mp_hal_stdout_tx_char(*str++);
    }
}

uint32_t mp_hal_ticks_ms(void) {
    //TODO(It might not precise)
    return clock_seconds();
}

uint32_t mp_hal_ticks_us(void) {
    //TODO(It might not precise)
    return clock_time();
}

void mp_hal_delay_ms(uint32_t delay) {
    clock_delay(delay * 1000);
}
