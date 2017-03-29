/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 * Copyright (c) 2015 Daniel Campora
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
#include "py/mpconfig.h"
#include "lib/utils/pyexec.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/compile.h"
#include "py/gc.h"

#include "gccollect.h"
#include "gchelper.h"
#include "exception.h"

#include "lib/fatfs/ff.h"
#include "extmod/fsusermount.h"
#include "extmod/vfs_fat_file.h"
#include "cmsis_os.h"
#include "sys_api.h"

#include "flash.h"

#include "flash_api.h"
#include "osdep_api.h"

#include "gpio_irq_api.h"

//#include <mDNS/mDNS.h>
#include <mdns/mDNS.h> 

/*****************************************************************************
 *                              Internal variables
 * ***************************************************************************/
osThreadId main_tid = 0;
osThreadId ftpd_tid = 0;
fs_user_mount_t fs_user_mount_flash;
static const char fresh_main_py[] =
"# main.py -- put your code here! The script in main.py will be executed when boot up !\r\n"
;

/*****************************************************************************
 *                              External functions
 * ***************************************************************************/
void main_task(void const *arg) {
    const char *msg_0 = "Starting to execute main.py\r\n";
    const char *msg_1 = "Execute main.py error\r\n";
    const char *msg_2 = "Can not find main.py, skipping\r\n";
    const char *msg_3 = "Soft reset\r\n";

    mp_hal_stdout_tx_strn_cooked(msg_0, strlen(msg_0));
    const uint8_t *main_py = "main.py";
    FRESULT res = f_stat(main_py, NULL);
    if (res == FR_OK) {
        int32_t ret = pyexec_file(main_py);
        if (!ret) {
            mp_hal_stdout_tx_strn_cooked(msg_1, strlen(msg_1));
        }
    } else {
        mp_hal_stdout_tx_strn_cooked(msg_2, strlen(msg_2));
    }
    if (pyexec_friendly_repl() != 0) {
        mp_hal_stdout_tx_strn_cooked(msg_3, strlen(msg_3));
        sys_reset();
    }
}

void ftpd_task(void const *arg) {
    ftpd_init();
    for(;;);
}

int main(void)
{
    __libc_init_array();
    // Get ARM's stack pointer
    uint32_t sp = gc_helper_get_sp();
    gc_collect_init (sp);
    // Init micropython gc from mp_heap_head to mp_heap_end
    gc_init(&_mp_gc_head, &_mp_gc_end);
    
    // Kernel initialization
    osKernelInitialize();

    // Init micropython basic system
    mp_init();
    mp_obj_list_init(mp_sys_path, 0);
    mp_obj_list_init(mp_sys_argv, 0);
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR_)); 

    log_uart_init0();
    pin_init0();
    network_init0();
    netif_init0();
    rtc_init0();
    crypto_init0();
    // wifi should br init after lwip init, or it will hang.
    wlan_init0(); 
    mpexception_init0();
    readline_init0();
    flash_vfs_init0();
    os_init0();
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_flash));
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_flash_slash_lib));
    // Create main task
    osThreadDef(main_task, MICROPY_MAIN_TASK_PRIORITY, 1, MICROPY_MAIN_TASK_STACK_SIZE);
    main_tid = osThreadCreate (osThread (main_task), NULL);
    osKernelStart();
    for(;;);
    return 0;
}

void NORETURN __fatal_error(const char *msg) {
    mp_hal_stdout_tx_strn("\nFATAL ERROR:\n", 14);
    mp_hal_stdout_tx_strn(msg, strlen(msg));
    for (uint i = 0;;) {
        for (volatile uint delay = 0; delay < 10000000; delay++);
    }
}

void flash_vfs_init0(void) {
    fs_user_mount_t *vfs = &fs_user_mount_flash;
    vfs->str = "/flash";
    vfs->len = 6;
    vfs->flags = 0;
    flash_init0(vfs);
// put the flash device in slot 0 (it will be unused at this point)
    MP_STATE_PORT(fs_user_mount)[0] = vfs;

    FRESULT res = f_mount(&vfs->fatfs, vfs->str, 1);

    if (res == FR_NO_FILESYSTEM) {
        res = f_mkfs("/flash", 0, 0);
        if (res == FR_OK) {
            // success creating fresh
        } else {
            MP_STATE_PORT(fs_user_mount)[0] = NULL;
            return;
        }
        // create empty main.py
        FIL fp;
        res = f_open(&fp, "/flash/main.py", FA_WRITE | FA_CREATE_ALWAYS);
        UINT n;
        res = f_write(&fp, fresh_main_py, sizeof(fresh_main_py) - 1 /* don't count null terminator */, &n);
        // TODO check we could write n bytes
        f_close(&fp);
    } else if (res == FR_OK) {
        // mount successful
    } else {
        __fatal_error("PYB: can't mount flash\n");
        MP_STATE_PORT(fs_user_mount)[0] = NULL;
        return;
    }
}

void nlr_jump_fail(void *val) {
    DiagPrintf("FATAL: uncaught exception %p\n", val);
    mp_obj_print_exception(&mp_plat_print, (mp_obj_t)val);
    __fatal_error("error");
}

mp_import_stat_t mp_import_stat(const char *path) {
    return fat_vfs_import_stat(path);
}

mp_lexer_t *mp_lexer_new_from_file(const char *filename) {
    return fat_vfs_lexer_new_from_file(filename);
}
void mp_keyboard_interrupt(void) {
    MP_STATE_VM(mp_pending_exception) = MP_STATE_PORT(mp_kbd_exception);
}

mp_obj_t mp_builtin_open(uint n_args, const mp_obj_t *args, mp_map_t *kwargs) {
    return fatfs_builtin_open(n_args, args, kwargs);
}
MP_DEFINE_CONST_FUN_OBJ_KW(mp_builtin_open_obj, 1, mp_builtin_open);

mp_obj_t mp_builtin_ftpd(mp_obj_t enable_in) {
    bool enable = mp_obj_is_true(enable_in);
    if (enable) {
        if (ftpd_tid != NULL) {
            nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Thread has been created"));
        }
        osThreadDef(ftpd_task, MICROPY_FTPD_TASK_PRIORITY, 1, MICROPY_FTPD_STACK_SIZE);
        ftpd_tid = osThreadCreate (osThread (ftpd_task), NULL);
    } else {
        if (ftpd_tid == NULL) {
            nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Thread has not created"));
        }
        osStatus status = osThreadTerminate(ftpd_tid);
        if (status == osOK) {
            ftpd_tid = NULL;
        } else {
            nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Thread terminate failed"));
        }
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(mp_builtin_ftpd_obj, mp_builtin_ftpd);
