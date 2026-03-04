extern uint64_t _target_tbfreq;

extern uint64_t calc_target_tbfreq();

extern void tbfreq_apply_to_kernel(uint64_t value);
extern void tbfreq_apply_to_memory(uint64_t value, void* ptr, uint64_t size);
extern void tbfreq_apply_to_lv1(uint64_t value);
