// It's a dummy file
//
void initAirKiss(void) {
}
void rx_process_airkiss(void) {
}
void clearAirKissBuf(void) {
}

#include "ssv_types.h"
#if 1
void ssv_delay (U32 clk) {
    while (clk--)
        __asm volatile ("nop");
}

#endif
