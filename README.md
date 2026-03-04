Add the following code to main.c in CFW stage2, then recompile and use as external Cobra.

```
#include "tbfreq.h"

	_target_tbfreq = calc_target_tbfreq();
	if (_target_tbfreq != 0)
	{
		tbfreq_apply_to_lv1(_target_tbfreq);
		tbfreq_apply_to_kernel(_target_tbfreq);
	}
```
