/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014, 2015 Damien P. George
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

#include "pin.h"

STATIC pin_obj_t pin_obj[GPIO_MAX] = {
    {{&pin_type}, GPIO_1,  {OUTPUT, GPIO_PULL_NONE, GPIO_DRIVE_DISABLE}, 0},
    {{&pin_type}, GPIO_2,  {OUTPUT, GPIO_PULL_NONE, GPIO_DRIVE_DISABLE}, 0},
    {{&pin_type}, GPIO_3,  {OUTPUT, GPIO_PULL_NONE, GPIO_DRIVE_DISABLE}, 0},
    {{&pin_type}, GPIO_5,  {OUTPUT, GPIO_PULL_NONE, GPIO_DRIVE_DISABLE}, 0},
    {{&pin_type}, GPIO_6,  {OUTPUT, GPIO_PULL_NONE, GPIO_DRIVE_DISABLE}, 0},
    {{&pin_type}, GPIO_8,  {OUTPUT, GPIO_PULL_NONE, GPIO_DRIVE_DISABLE}, 0},
    {{&pin_type}, GPIO_15, {OUTPUT, GPIO_PULL_NONE, GPIO_DRIVE_DISABLE}, 0},
    {{&pin_type}, GPIO_18, {OUTPUT, GPIO_PULL_NONE, GPIO_DRIVE_DISABLE}, 0},
    {{&pin_type}, GPIO_19, {OUTPUT, GPIO_PULL_NONE, GPIO_DRIVE_DISABLE}, 0},
    {{&pin_type}, GPIO_20, {OUTPUT, GPIO_PULL_NONE, GPIO_DRIVE_DISABLE}, 0},
};

STATIC mp_obj_t pin_call(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 1, false);
    pin_obj_t *self = self_in;
    if (n_args == 0) {
        // get pin
        return MP_OBJ_NEW_SMALL_INT(digitalRead(self->id));
    } else {
        // set pin
        digitalWrite(self->id, mp_obj_is_true(args[0]));
        self->value = mp_obj_is_true(args[0]);
        return mp_const_none;
    }
}

STATIC void pin_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    pin_obj_t *self = self_in;

    // pin name
    mp_printf(print, "Pin(%u)", self->id);
}

STATIC mp_obj_t pin_obj_init_helper(pin_obj_t *self, mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_mode, ARG_pull, ARG_drive, ARG_value };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_dir,      MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_pull,     MP_ARG_INT,                  {.u_int = GPIO_PULL_NONE}},
        { MP_QSTR_drive,    MP_ARG_INT,                  {.u_int = GPIO_DRIVE_DISABLE}},
        { MP_QSTR_value,    MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // get io mode
    uint dir = args[ARG_mode].u_int;

    // get pull mode
    uint pull = args[ARG_pull].u_int;

    // get drive mode
    uint drive = args[ARG_drive].u_int;

    // get initial value
    int value = 0;
    if (args[ARG_value].u_obj == MP_OBJ_NULL) {
        value = -1;
    } else {
        value = mp_obj_is_true(args[ARG_value].u_obj);
    }

    self->conf.dirt = dir;
    self->conf.driving_en = drive;
    self->conf.pull_en = pull;
    self->value = value;

    pinMode(self->id, self->conf);

    digitalWrite(self->id, value);

    return mp_const_none;
}

STATIC mp_obj_t pin_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // get the wanted pin object
    int wanted_pin = mp_obj_get_int(args[0]);
    pin_obj_t *pin = NULL;
    if (0 <= wanted_pin && wanted_pin < MP_ARRAY_SIZE(pin_obj)) {
        pin = (pin_obj_t*)&pin_obj[wanted_pin];
    }
    if (pin == NULL || pin->base.type == NULL) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "invalid pin"));
    }

    if (n_args > 1 || n_kw > 0) {
        // pin mode given, so configure this GPIO
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        pin_obj_init_helper(pin, n_args - 1, args + 1, &kw_args);
    }

    return (mp_obj_t)pin;
}

STATIC mp_obj_t pin_value(mp_uint_t n_args, const mp_obj_t *args) {
    return pin_call(args[0], n_args - 1, 0, args + 1);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pin_value_obj, 1, 2, pin_value);

//TODO( not complete)
STATIC mp_obj_t pin_toggle(mp_obj_t self_in) {
    pin_obj_t *self = self_in;

    self->value ^= 1;

    mp_obj_t value = self->value ? mp_const_true:mp_const_false;

    return pin_call(self, 1, 0, value);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pin_toggle_obj, pin_toggle);

STATIC const mp_map_elem_t pin_locals_dict_table[] = {
#if 0
    // instance methods
    { MP_OBJ_NEW_QSTR(MP_QSTR_id),             (mp_obj_t)&pin_id_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_dir),            (mp_obj_t)&pin_dir_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pull),           (mp_obj_t)&pin_pull_obj },
    
    { MP_OBJ_NEW_QSTR(MP_QSTR_irq_register),   (mp_obj_t)&pin_irq_register_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_irq_unregister), (mp_obj_t)&pin_irq_unregister_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_irq_mask),       (mp_obj_t)&pin_irq_mask_obj },
#endif

    { MP_OBJ_NEW_QSTR(MP_QSTR_value),          (mp_obj_t)&pin_value_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_toggle),         (mp_obj_t)&pin_toggle_obj },

    // class constants
    { MP_OBJ_NEW_QSTR(MP_QSTR_IN),             MP_OBJ_NEW_SMALL_INT(INPUT) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_OUT),            MP_OBJ_NEW_SMALL_INT(OUTPUT) },

    { MP_OBJ_NEW_QSTR(MP_QSTR_PULL_NONE),      MP_OBJ_NEW_SMALL_INT(GPIO_PULL_NONE) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_PULL_DOWN),      MP_OBJ_NEW_SMALL_INT(GPIO_PULL_DOWN) },

    { MP_OBJ_NEW_QSTR(MP_QSTR_DRV_DIS),        MP_OBJ_NEW_SMALL_INT(GPIO_DRIVE_DISABLE) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_DRV_EN),         MP_OBJ_NEW_SMALL_INT(GPIO_DRIVE_ENABLE) },

    { MP_OBJ_NEW_QSTR(MP_QSTR_IRQ_RISE),       MP_OBJ_NEW_SMALL_INT(RISING_EDGE) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_IRQ_FALL),       MP_OBJ_NEW_SMALL_INT(FALLING_EDGE) },
};
STATIC MP_DEFINE_CONST_DICT(pin_locals_dict, pin_locals_dict_table);

const mp_obj_type_t pin_type = {
    { &mp_type_type },
    .name        = MP_QSTR_Pin,
    .print       = pin_print,
    .make_new    = pin_make_new,
    .call        = pin_call,
    .locals_dict = (mp_obj_t)&pin_locals_dict,
};
