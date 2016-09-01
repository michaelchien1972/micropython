/*
 * Copyright (c) 2014, SouthSilicon Valley Corp.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         A very simple Contiki application showing how Contiki programs look
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "py/mpconfig.h"
#include "lib/utils/pyexec.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/compile.h"

#include "readline.h"
#include "pyexec.h"


#include "sys/clock.h"

#include "mphal.h"
#include "gccollect.h"
#include "flash.h"

#include <stdio.h> /* For printf() */
#include "gpio_api.h"
#include "atcmd.h"
#include "bsp.h"
#include "hw_init.h"

const char *msg_0 = "Starting to execute main.py\r\n";
const char *msg_1 = "REPL start failed\r\n";
static const char fresh_main_py[] =
"# main.py -- put your code here! The script in main.py will be executed when boot up !\r\n"
;

fs_user_mount_t fs_user_mount_flash;

at_cmd_info atcmd_info_tbl[] = 
{
    {"",    NULL}
};
/*---------------------------------------------------------------------------*/
int SetLED (uint8_t nEnable)
{
	GPIO_CONF conf;

   conf.dirt = OUTPUT;
   conf.driving_en = 0;
   conf.pull_en = 0;
   
   pinMode(GPIO_1, conf);
	if(nEnable == 0)
	{
		digitalWrite(GPIO_1, 0);
	}
	else
	{
		digitalWrite(GPIO_1, 1);
	}
	return ERROR_SUCCESS;
}

void flash_vfs_init0 (void) {
    fs_user_mount_t *vfs = &fs_user_mount_flash;
    vfs->str = "/flash";
    vfs->len = 6;
    vfs->flags = 0;
    flash_init0(vfs);
    MP_STATE_PORT(fs_user_mount)[0] = vfs;
    FRESULT res = f_mount(&vfs->fatfs, vfs->str, 1);
    if (res == FR_NO_FILESYSTEM) {
        res = f_mkfs("/flash", 1, 0);
        if (res != FR_OK) {
            MP_STATE_PORT(fs_user_mount)[0] = NULL;
            return;
        }
    } else if (res == FR_OK) {
        // To nothing if vfs mount success
    } else {
        printf("Create fatfs in flash failed, res = %d\r\n", res);
        MP_STATE_PORT(fs_user_mount)[0] = NULL;
        return;
    }

    FIL fp;
    res = f_open(&fp, "/flash/main.py", FA_WRITE | FA_OPEN_ALWAYS);
    UINT n;
    res = f_write(&fp, fresh_main_py, sizeof(fresh_main_py) - 1 /* don't count null terminator */, &n);
    f_close(&fp);
    if (FR_OK != f_chdir ("/flash/lib")) {
        f_mkdir("/flash/lib");
    }
}

int mp_main (void) {
    gc_init(&__heap_start__, &__heap_end__);
    mp_init();
    mp_obj_list_init(mp_sys_path, 0);
    mp_obj_list_init(mp_sys_argv, 0);
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR_));
    clock_init();
    irq_init();
    hwtmr_init();
    mphal_init();
    readline_init0();
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_flash));
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_flash_slash_lib));
    mp_hal_stdout_tx_strn_cooked(msg_0, strlen(msg_0));
    flash_vfs_init0();
    for(;;) {
        if(pyexec_friendly_repl() != 0) {
            api_wdog_reboot();
        }
    }
    return 0;
}


bool gDisableRTS = 0;
int i_config = 0;

void nlr_jump_fail(void *val) { 
    printf("NLR failed\r\n");
}

mp_import_stat_t mp_import_stat(const char *path) {
    return fat_vfs_import_stat(path);
}

mp_lexer_t *mp_lexer_new_from_file(const char *filename) {
    return fat_vfs_lexer_new_from_file(filename);
}

mp_obj_t mp_builtin_open(uint n_args, const mp_obj_t *args, mp_map_t *kwargs) {
    return fatfs_builtin_open(n_args, args, kwargs);
}
MP_DEFINE_CONST_FUN_OBJ_KW(mp_builtin_open_obj, 1, mp_builtin_open);
