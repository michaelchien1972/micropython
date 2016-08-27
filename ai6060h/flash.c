#include "flash.h"

STATIC const mp_obj_base_t flash_obj = {&flash_type};
static bool spi_inited = false;
static uint32_t flash_addr_cache = FLASH_START_BASE;

void flash_init(void) {
    if (!spi_inited) {
        spi_flash_init();
        spi_inited = true;
    }
}

void flash_flush(void) {
    spi_flash_finalize(flash_addr_cache);
}

uint32_t flash_get_block_count(void) {
    return FLASH_START_BLOCK + FLASH_NUM_BLOCKS;
}

uint32_t flash_get_block_size(void) {
    return FLASH_BLOCK_SIZE;
}

static uint32_t convert_block_to_flash_addr(uint32_t block) {
    if (FLASH_START_BLOCK <= block && block < FLASH_START_BLOCK + FLASH_NUM_BLOCKS) {
        block -= FLASH_START_BLOCK;
        return FLASH_START_BLOCK + block * FLASH_BLOCK_SIZE + FLASH_START_BASE;
    }
    // bad block
    return -1;
}

bool flash_read_block(uint8_t *dest, uint32_t block) {
    uint32_t flash_addr = convert_block_to_flash_addr(block);
    if (flash_addr == -1) {
        return false;
    }
    //TODO need error handle
    spi_flash_read(flash_addr, dest, FLASH_BLOCK_SIZE);
    return true;
}

bool flash_write_block(uint8_t *src, uint32_t block) {
    uint32_t flash_addr = convert_block_to_flash_addr(block);
    if (flash_addr == -1) {
        return false;
    }
    //TODO need error handle
    spi_flash_sector_erase(flash_addr); 
    spi_flash_write(flash_addr, src, FLASH_BLOCK_SIZE);
    flash_addr_cache = flash_addr;
    // TODO (there might something problem in their flash_api.h driver, so I need to flush every time)
    flash_flush();
    return true;
}

mp_uint_t flash_read_blocks(uint8_t *dest, uint32_t block_num, uint32_t num_blocks) {
    for (size_t i = 0; i < num_blocks; i++) {
        if (!flash_read_block(dest + i * FLASH_BLOCK_SIZE, block_num + i)) {
            return 1; // error
        }
    }
    return 0; // success
}

mp_uint_t flash_write_blocks(uint8_t *src, uint32_t block_num, uint32_t num_blocks) {
    for (size_t i = 0; i < num_blocks; i++) {
        if (!flash_write_block(src + i * FLASH_BLOCK_SIZE, block_num + i)) {
            return 1; // error
        }
    }
    return 0; // success
}

STATIC mp_obj_t flash_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    // return singleton object
    return (mp_obj_t)&flash_obj;
}

STATIC mp_obj_t flash_readblocks(mp_obj_t self, mp_obj_t block_num, mp_obj_t buf) {
    mp_uint_t ret;
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf, &bufinfo, MP_BUFFER_WRITE);
    ret = flash_read_blocks(bufinfo.buf, mp_obj_get_int(block_num), bufinfo.len / FLASH_BLOCK_SIZE);
    return MP_OBJ_NEW_SMALL_INT(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(flash_readblocks_obj, flash_readblocks);

STATIC mp_obj_t flash_writeblocks(mp_obj_t self, mp_obj_t block_num, mp_obj_t buf) {
    mp_uint_t ret;
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf, &bufinfo, MP_BUFFER_READ);
    ret = flash_write_blocks(bufinfo.buf, mp_obj_get_int(block_num), bufinfo.len / FLASH_BLOCK_SIZE);
    return MP_OBJ_NEW_SMALL_INT(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(flash_writeblocks_obj, flash_writeblocks);

STATIC mp_obj_t flash_ioctl(mp_obj_t self, mp_obj_t cmd_in, mp_obj_t arg_in) {
    mp_int_t cmd = mp_obj_get_int(cmd_in);
    switch (cmd) {
        case BP_IOCTL_INIT: flash_init(); return MP_OBJ_NEW_SMALL_INT(0);
        //case BP_IOCTL_DEINIT: flash_flush(); return MP_OBJ_NEW_SMALL_INT(0); // TODO properly
        case BP_IOCTL_SYNC: flash_flush(); return MP_OBJ_NEW_SMALL_INT(0);
        case BP_IOCTL_SEC_COUNT: return MP_OBJ_NEW_SMALL_INT(flash_get_block_count());
        case BP_IOCTL_SEC_SIZE: return MP_OBJ_NEW_SMALL_INT(flash_get_block_size());

        default: return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(flash_ioctl_obj, flash_ioctl);

STATIC const mp_map_elem_t flash_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_readblocks),  (mp_obj_t)&flash_readblocks_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_writeblocks), (mp_obj_t)&flash_writeblocks_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ioctl),       (mp_obj_t)&flash_ioctl_obj },
};
STATIC MP_DEFINE_CONST_DICT(flash_locals_dict, flash_locals_dict_table);

const mp_obj_type_t flash_type = {
    { &mp_type_type },
    .name = MP_QSTR_Flash,
    .make_new = flash_make_new,
    .locals_dict = (mp_obj_t)&flash_locals_dict,
};

void flash_init0 (fs_user_mount_t *vfs) {
    vfs->flags |= FSUSER_NATIVE | FSUSER_HAVE_IOCTL;
    vfs->readblocks[0]  = (mp_obj_t)&flash_readblocks_obj;
    vfs->readblocks[1]  = (mp_obj_t)&flash_obj;
    vfs->readblocks[2]  = (mp_obj_t)flash_read_blocks; // native version
    vfs->writeblocks[0] = (mp_obj_t)&flash_writeblocks_obj;
    vfs->writeblocks[1] = (mp_obj_t)&flash_obj;
    vfs->writeblocks[2] = (mp_obj_t)flash_write_blocks; // native version
    vfs->u.ioctl[0]     = (mp_obj_t)&flash_ioctl_obj;
    vfs->u.ioctl[1]     = (mp_obj_t)&flash_obj;
}
