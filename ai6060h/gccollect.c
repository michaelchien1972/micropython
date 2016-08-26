

#include "py/mpconfig.h"
#include "py/gc.h"
#include "gccollect.h"

void gc_collect(void) {
    // start the GC
    gc_collect_start();

    // get the registers and the sp
    mp_uint_t regs[10];
    //(TODO)(Wait to implement)
    //mp_uint_t sp = gc_helper_get_regs_and_sp(regs);
    mp_uint_t sp = 0;

    // trace the stack, including the registers (since they live on the stack in this function)
    //(TODO)(Wait to implement)
    //gc_collect_root((void**)sp, (stackend - sp) / sizeof(uint32_t));

    // end the GC
    gc_collect_end();
}
