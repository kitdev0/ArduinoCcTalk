
#ifndef _SM_TIMEOUT_h
#define _SM_TIMEOUT_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class SM_TIMEOUT
{
private:
	uint32_t previous_time = 0;

public:
	SM_TIMEOUT();
	void reset(void);
	bool check(uint32_t _time);
};


#endif

