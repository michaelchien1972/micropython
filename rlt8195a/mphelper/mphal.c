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
// Standard header
#include <stdio.h>

// Micropython header
#include "extmod/misc.h"
#include "py/ringbuf.h"

// SDK header
#include "osdep_api.h"
#include "device.h"

STATIC uint8_t input_buf_array[256];

ringbuf_t input_buf = {input_buf_array, sizeof(input_buf_array)};

int interrupt_char;

MON_RAM_TEXT_SECTION
void UartLogIrqHandleRam(void *arg) {
    uint8_t c = -1;
    bool PullMode = _FALSE;
    uint32_t IrqEn = DiagGetIsrEnReg();

    DiagSetIsrEnReg(0);

    c = DiagGetChar(PullMode);

    if (c == interrupt_char) {
        mp_keyboard_interrupt();
    } else {
        ringbuf_put(&input_buf, c);
    }

    DiagSetIsrEnReg(IrqEn);
}

void log_uart_init0(void)
{
    IRQ_HANDLE      UartIrqHandle;
    
    UartIrqHandle.Data     = NULL;
    UartIrqHandle.IrqNum   = UART_LOG_IRQ;
    UartIrqHandle.IrqFun   = UartLogIrqHandleRam;
    UartIrqHandle.Priority = 0;
    
    InterruptUnRegister(&UartIrqHandle);
    InterruptRegister(&UartIrqHandle);
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
    DiagPutChar(c);
    mp_uos_dupterm_tx_strn(&c, 1);
}

void mp_hal_stdout_tx_str(const char *str) {
    while (*str) {
        mp_hal_stdout_tx_char(*str++);
    }
}

void mp_hal_stdout_tx_strn(const char *str, size_t len) {
    while (len--) {
        mp_hal_stdout_tx_char(*str++);
    }
}

void mp_hal_stdout_tx_strn_cooked(const char *str, size_t len) {
    while (len--) {
        if (*str == '\n') {
            mp_hal_stdout_tx_char('\r');
        }
        mp_hal_stdout_tx_char(*str++);
    }
}

void mp_hal_set_interrupt_char(int c) {
    mpexception_set_interrupt_char(c);
}

void mp_hal_delay_ms(uint32_t ms) {
    mdelay(ms);
}

void mp_hal_delay_us(uint32_t us) {
    udelay(us);
}

uint32_t mp_hal_ticks_ms(void) {
    // TODO
    return 0;
}
