#ifndef __MPHAL_H_
#define __MPHAL_H

void mphal_init(void);
void mp_hal_delay_ms(uint32_t delay);
void mp_hal_stdout_tx_strn_cooked(const char *str, uint32_t len);
void mp_hal_set_interrupt_char(int c);
#endif
