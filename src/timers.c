#include "timers.h"

void
timers_init(Timers *timers)
{
	timers->dt = 0;
	timers->st = 0;
}

void
timers_reset(Timers *timers)
{
	timers_init(timers);
}

void
timers_update(Timers *timers)
{
	if (timers->dt > 0) {
		timers->dt--;
	}
	if (timers->st > 0) {
		timers->st--;
	}
}
