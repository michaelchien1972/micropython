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

#include "timer.h"

/*****************************************************************************
 *                              External variables
 * ***************************************************************************/

/*****************************************************************************
 *                              Internal functions
 * ***************************************************************************/
STATIC timer_obj_t timer_obj[MAX_HW_TIMER-1] = {
    {.base.type = &timer_type, .isr_handler = mp_const_none, .tmr = REG_TIMER_US_0 , .unit = 0},
    {.base.type = &timer_type, .isr_handler = mp_const_none, .tmr = REG_TIMER_US_1 , .unit = 1},
    {.base.type = &timer_type, .isr_handler = mp_const_none, .tmr = REG_TIMER_US_2 , .unit = 2},
    {.base.type = &timer_type, .isr_handler = mp_const_none, .tmr = REG_TIMER_US_3 , .unit = 3},
#if 0 //Well, Timer 4 migh cause hang, don't use
    {.base.type = &timer_type, .isr_handler = mp_const_none, .tmr = REG_TIMER_MS_0 , .unit = 4},
#endif
    {.base.type = &timer_type, .isr_handler = mp_const_none, .tmr = REG_TIMER_MS_1 , .unit = 4},
    {.base.type = &timer_type, .isr_handler = mp_const_none, .tmr = REG_TIMER_MS_2 , .unit = 5},
    {.base.type = &timer_type, .isr_handler = mp_const_none, .tmr = REG_TIMER_MS_3 , .unit = 6},
};

irq_handler timer_isr(timer_obj_t *self) {
    if (self->isr_handler != mp_const_none) { 
        gc_lock();
        nlr_buf_t nlr;
        if (nlr_push(&nlr) == 0) {
            mp_call_function_0(self->isr_handler);
            nlr_pop(); 
        } else {
            self->isr_handler = mp_const_none;
            mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
        }
        gc_unlock();
    }
    return 0;
}

STATIC mp_obj_t timer_count(mp_obj_t self_in) {
    timer_obj_t *self = self_in;
    return mp_obj_new_int(self->tmr->TMR_CURRENT);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(timer_count_obj, timer_count);

STATIC mp_obj_t timer_stop(mp_obj_t self_in) {
    timer_obj_t *self = self_in;
    hwtmr_stop(self->tmr);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(timer_stop_obj, timer_stop);

STATIC mp_obj_t timer_callback(mp_obj_t self_in, mp_obj_t func_in) {
    timer_obj_t *self = self_in;
    if (!MP_OBJ_IS_FUN(func_in)) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "Error function type"));
    }
    self->isr_handler = func_in;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(timer_callback_obj, timer_callback);

STATIC mp_obj_t timer_start(mp_obj_t self_in, mp_obj_t count_in, mp_obj_t mode_in) {
    timer_obj_t *self = self_in;
    uint8_t mode = 0;
    uint32_t count = 0;
    mode = mp_obj_get_int(mode_in);
    count = mp_obj_get_int(count_in);
    hwtmr_start(self->tmr, count, timer_isr, self, mode);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(timer_start_obj, timer_start);

STATIC const mp_map_elem_t timer_locals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__),    MP_OBJ_NEW_QSTR(MP_QSTR_wdt) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_count),       (mp_obj_t)&timer_count_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_callback),    (mp_obj_t)&timer_callback_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_stop),        (mp_obj_t)&timer_stop_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_start),       (mp_obj_t)&timer_start_obj },

    { MP_OBJ_NEW_QSTR(MP_QSTR_PERIODIC),  MP_OBJ_NEW_SMALL_INT(HTMR_PERIODIC) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ONESHOT),   MP_OBJ_NEW_SMALL_INT(HTMR_ONESHOT) },
};
STATIC MP_DEFINE_CONST_DICT(timer_locals_dict, timer_locals_table);

STATIC mp_obj_t timer_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 1, 1, false);
    uint8_t timer_id = mp_obj_get_int(args[0]);

    if ((timer_id < 0) || (timer_id > 6))
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "Resource not available"));

    timer_obj_t *self = &timer_obj[timer_id];

    // return singleton object
    return self;
}

STATIC void timer_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    timer_obj_t *self = self_in;
    mp_printf(print, "Timer<%u>", self->unit);
}

const mp_obj_type_t timer_type = {
    { &mp_type_type },
    .name        = MP_QSTR_Timer,
    .print       = timer_print,
    .make_new    = timer_make_new,
    .locals_dict = (mp_obj_t)&timer_locals_dict,
};
