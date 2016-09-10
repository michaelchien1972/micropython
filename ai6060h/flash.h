#ifndef _FLASH_H_
#define _FLASH_H_

#include "py/mpconfig.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "lib/fatfs/ff.h"
#include "extmod/fsusermount.h"
#include "flash_api.h"
#include "extmod/vfs_fat_file.h"
#include "timeutils.h"

extern const struct _mp_obj_type_t flash_type;

#define FLASH_START_BASE    (0x72000)
#define FLASH_BLOCK_SIZE    (4096)
#define FLASH_START_BLOCK   (0)
#define FLASH_NUM_BLOCKS    (62)

void flash_init0(fs_user_mount_t *vfs);

#endif 
