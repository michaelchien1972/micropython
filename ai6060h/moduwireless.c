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

/*****************************************************************************
 *                              Header includes
 * ***************************************************************************/
#include "py/mpstate.h"
#include "py/runtime.h"
#include "py/mphal.h"

const mp_obj_type_t wlan_type;

typedef struct {
    mp_obj_base_t base;
} wlan_obj_t;

STATIC wlan_obj_t wlan_obj = {
    {{&wlan_type}}
};

STATIC mp_obj_t wlan_start_ap(mp_obj_t self_in) {
    wlan_obj_t *self = self_in;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(wlan_start_ap_obj, wlan_start_ap);

STATIC const mp_map_elem_t wlan_locals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__),   MP_OBJ_NEW_QSTR(MP_QSTR_wlan) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_start_ap),   (mp_obj_t)&wlan_start_ap_obj },
};
STATIC MP_DEFINE_CONST_DICT(wlan_locals_dict, wlan_locals_table);

STATIC mp_obj_t wlan_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 1, false);
    return (mp_obj_t)&wlan_obj;
}

const mp_obj_type_t wlan_type = {
    { &mp_type_type },
    .name        = MP_QSTR_WLAN,
    .make_new    = wlan_make_new,
    .locals_dict = (mp_obj_t)&wlan_locals_dict,
};

STATIC const mp_map_elem_t wireless_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_umachine) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_WLAN),     (mp_obj_t)&wlan_type },
};
STATIC MP_DEFINE_CONST_DICT(wireless_module_globals, wireless_module_globals_table);

const mp_obj_module_t mp_module_uwireless = {
    .base    = { &mp_type_module },
    .name    = MP_QSTR_uwireless,
    .globals = (mp_obj_dict_t*)&wireless_module_globals,
};
