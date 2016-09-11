#include <stdint.h>

// options to control how Micro Python is built

#define MICROPY_QSTR_BYTES_IN_HASH          (1)
#define MICROPY_ALLOC_PATH_MAX              (128)
#define MICROPY_NLR_SETJMP                  (1)
#define MICROPY_COMP_MODULE_CONST           (0)
#define MICROPY_COMP_CONST                  (1)
#define MICROPY_COMP_DOUBLE_TUPLE_ASSIGN    (0)
#define MICROPY_COMP_TRIPLE_TUPLE_ASSIGN    (0)
#define MICROPY_MEM_STATS                   (0)
#define MICROPY_MODULE_WEAK_LINKS           (1)
#define MICROPY_ENABLE_GC                   (1)
#define MICROPY_HELPER_REPL                 (1)
#define MICROPY_REPL_EVENT_DRIVEN           (1)
#define MICROPY_ENABLE_COMPILER             (1)
#define MICROPY_ENABLE_FINALISER            (0)
#define MICROPY_ENABLE_SOURCE_LINE          (1)
#define MICROPY_ENABLE_DOC_STRING           (0)
#define MICROPY_REPL_AUTO_INDENT            (1)
#define MICROPY_ERROR_REPORTING             (MICROPY_ERROR_REPORTING_NORMAL)
#define MICROPY_BUILTIN_METHOD_CHECK_SELF_ARG (0)
#define MICROPY_PY_BUILTINS_BYTEARRAY       (1)
#define MICROPY_PY_BUILTINS_ENUMERATE       (1)
#define MICROPY_PY_BUILTINS_FROZENSET       (1)
#define MICROPY_PY_BUILTINS_REVERSED        (0)
#define MICROPY_PY_BUILTINS_SET             (1)
#define MICROPY_PY_BUILTINS_SLICE           (1)
#define MICROPY_PY_BUILTINS_PROPERTY        (1)
#define MICROPY_PY___FILE__                 (1)

#define MICROPY_USE_INTERNAL_PRINTF         (0)
#define MICROPY_PY_GC                       (1)
#define MICROPY_PY_ARRAY                    (0)
#define MICROPY_PY_ATTRTUPLE                (1)
#define MICROPY_PY_COLLECTIONS              (0)
#define MICROPY_PY_MATH                     (0)
#define MICROPY_PY_CMATH                    (0)
#define MICROPY_PY_IO                       (0)
#define MICROPY_PY_STRUCT                   (0)
#define MICROPY_PY_SYS                      (1)
#define MICROPY_CPYTHON_COMPAT              (0)
#define MICROPY_LONGINT_IMPL                (MICROPY_LONGINT_IMPL_NONE)
#define MICROPY_FLOAT_IMPL                  (MICROPY_FLOAT_IMPL_FLOAT)

#define MICROPY_FSUSERMOUNT                     (1)  
#define MICROPY_FATFS_ENABLE_LFN                (1)
#define MICROPY_FATFS_LFN_CODE_PAGE             (437) /* 1=SFN/ANSI 437=LFN/U.S.(OEM) */
#define MICROPY_FATFS_VOLUMES                   (4)
#define MICROPY_FATFS_RPATH                     (2)
#define MICROPY_FATFS_MAX_SS                    (4096)

#define AI6060H_CONSOLE_UART_PORT           (SSV6XXX_UART0)
#define MICROPY_HW_BOARD_NAME               "AI6060H"
#define MICROPY_HW_MCU_NAME                 "SSV6060"

// type definitions for the specific machine

#define BYTES_PER_WORD (4)

#define MICROPY_MAKE_POINTER_CALLABLE(p) ((void*)((mp_uint_t)(p) | 1))

#define UINT_FMT "%lu"
#define INT_FMT "%ld"

typedef int32_t mp_int_t; // must be pointer size
typedef uint32_t mp_uint_t; // must be pointer size
typedef long mp_off_t;

// dummy print
#define MP_PLAT_PRINT_STRN(str, len) mp_hal_stdout_tx_strn_cooked(str, len)

// extra built in names to add to the global namespace
extern const struct _mp_obj_fun_builtin_t mp_builtin_open_obj;
#define MICROPY_PORT_BUILTINS \
    { MP_OBJ_NEW_QSTR(MP_QSTR_open), (mp_obj_t)&mp_builtin_open_obj },

extern const struct _mp_obj_module_t mp_module_utime;
extern const struct _mp_obj_module_t mp_module_uos;
extern const struct _mp_obj_module_t mp_module_umachine;
extern const struct _mp_obj_module_t mp_module_uwireless;

#define MICROPY_PORT_BUILTIN_MODULES \
    { MP_OBJ_NEW_QSTR(MP_QSTR_utime),     (mp_obj_t)&mp_module_utime },     \
    { MP_OBJ_NEW_QSTR(MP_QSTR_uos),       (mp_obj_t)&mp_module_uos },       \
    { MP_OBJ_NEW_QSTR(MP_QSTR_umachine),  (mp_obj_t)&mp_module_umachine },  \
    { MP_OBJ_NEW_QSTR(MP_QSTR_uwireless), (mp_obj_t)&mp_module_uwireless },  \

#define MICROPY_PORT_BUILTIN_MODULE_WEAK_LINKS \
    { MP_OBJ_NEW_QSTR(MP_QSTR_time),     (mp_obj_t)&mp_module_utime },     \
    { MP_OBJ_NEW_QSTR(MP_QSTR_os),       (mp_obj_t)&mp_module_uos },       \
    { MP_OBJ_NEW_QSTR(MP_QSTR_machine),  (mp_obj_t)&mp_module_umachine },  \
    { MP_OBJ_NEW_QSTR(MP_QSTR_wireless), (mp_obj_t)&mp_module_uwireless },  \

#define MP_STATE_PORT MP_STATE_VM
#define MICROPY_PORT_ROOT_POINTERS                                        \
    const char *readline_hist[8];                                         \
    mp_obj_t mp_const_user_interrupt;   \
    vstr_t *repl_line; \

// We need to provide a declaration/definition of alloca()
#include <alloca.h>
