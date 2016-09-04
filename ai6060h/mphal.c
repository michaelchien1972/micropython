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

#include "drv_uart.h"
#include "clock.h"

#include "mphal.h"

#include "irq.h"

#define BUFFRING_LEN 256

byte input_buf_array[BUFFRING_LEN];
ringbuf_t input_buf = {input_buf_array, BUFFRING_LEN};
int interrupt_char;

irq_handler rx_irq_handler(void *arg) {
    byte c = drv_uart_rx(AI6060H_CONSOLE_UART_PORT);
    //printf("c = %d\r\n", c);
    if (c == interrupt_char) {
        //TODO(not support yet)
        //mp_keyboard_interrupt();
    }
    else
        ringbuf_put(&input_buf, c);
}

void mphal_init(void) {
    drv_uart_init(AI6060H_CONSOLE_UART_PORT);

    // Need to this function to calibrate clock..
    drv_uart_40M();

    irq_request(IRQ_UART0_RX, rx_irq_handler, NULL);
}

int mp_hal_stdin_rx_chr(void) {
    int c = 0;
    for (;;) {
        c = ringbuf_get(&input_buf);
        if (c != -1) {
            return c;
        }
        ssv_delay(1000);
    }
}

void mp_hal_stdout_tx_char(char c) {
    drv_uart_tx(AI6060H_CONSOLE_UART_PORT, c);
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

void mp_hal_set_interrupt_char(int c) {
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
    return clock_time();
}

uint32_t mp_hal_ticks_us(void) {
    //TODO(It might not precise)
    return clock_time();
}

void mp_hal_delay_ms(uint32_t delay) {
    mp_hal_delay_us(delay * 1000);
}

void mp_hal_delay_us(uint32_t us) {
    //TODO(May not precise)
    ssv_delay(1000 * 1300);
}
