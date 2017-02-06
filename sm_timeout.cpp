#include "sm_timeout.h"

SM_TIMEOUT::SM_TIMEOUT()
{

}

void SM_TIMEOUT::reset(void)
{
	previous_time = millis();
}

bool SM_TIMEOUT::check(uint32_t _time)
{
	if (millis() < previous_time)
		reset();
	if (millis() - previous_time >= _time)
	{
		return 1;
	}
	return 0;
}


