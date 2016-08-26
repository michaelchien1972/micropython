


extern uint32_t __heap_start__;
extern uint32_t __heap_end__;

void gc_collect_init (uint32_t sp);
void gc_collect(void);
