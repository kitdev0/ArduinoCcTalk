#include "hm_7seg_shift_v201.h"

HM_7SEG_SHIFT::HM_7SEG_SHIFT(uint8_t _clk_pin, uint8_t _data_pin, uint8_t _latch_pin)
{
	clk_pin = _clk_pin;
	data_pin = _data_pin;
	latch_pin = _latch_pin;
}

HM_7SEG_SHIFT::~HM_7SEG_SHIFT()
{

}

//public
void HM_7SEG_SHIFT::init7SegDis(void)
{
	unsigned char a = 1, b = 2, c = 4, d = 8, e = 16, f = 32, g = 64, h = 128;
	//    _a_
	//  b|_d_|c
	//  h|_g_|e
	pinMode(clk_pin, OUTPUT);
	pinMode(data_pin, OUTPUT);
	pinMode(latch_pin, OUTPUT);

	segment_dis[0] = 0xFF - (a + b + c + e + g + h);			//0
	segment_dis[1] = 0xFF - (c + e);							//1
	segment_dis[2] = 0xFF - (a + c + d + h + g);				//2
	segment_dis[3] = 0xFF - (a + c + d + e + g);				//3
	segment_dis[4] = 0xFF - (b + d + c + e);					//4
	segment_dis[5] = 0xFF - (a + b + d + e + g);				//5
	segment_dis[6] = 0xFF - (a + b + d + h + g + e);			//6
	segment_dis[7] = 0xFF - (a + c + e);						//7
	segment_dis[8] = 0xFF - (a + b + c + d + e + g + h);		//8
	segment_dis[9] = 0xFF - (a + c + d + b + e + g);			//9

}

//############################################## Display Function ##################################################//
void HM_7SEG_SHIFT::displayNumeral(uint16_t _num, uint16_t _digit)
{
	register_array[0] = segment_dis[_num];
	register_array[1] = _digit;
	sendSerialData(2, register_array);
//	delay(_DIS_DELAY_TIME);
}

void HM_7SEG_SHIFT::displayHyphen(uint16_t _digit)
{
	register_array[0] = 0xFF - _DISPLAY_HYPHEN;
	register_array[1] = _digit;
	sendSerialData(2, register_array);
//	delay(_DIS_DELAY_TIME);
}

void HM_7SEG_SHIFT::displayChar(uint16_t _char, uint16_t _digit)
{
	register_array[0] = 0xFF - _char;
	register_array[1] = _digit;
	sendSerialData(2, register_array);
//	delay(_DIS_DELAY_TIME);
}

void HM_7SEG_SHIFT::displayCharWithDotAndValue(uint16_t _char, uint16_t _value)
{
	next_value_dis++;
	if (next_value_dis > 3)
		next_value_dis = 0;
	switch (next_value_dis)
	{
	case 0:
		register_array[0] = segment_dis[_value % 10];
		register_array[1] = _DIG_0;
		break;
	case 1:
		if (_value >= 10)
		{
			register_array[0] = segment_dis[(_value % 100) / 10];
			register_array[1] = _DIG_1;
		}
		else
		{
			register_array[0] = 0xFF;
			register_array[1] = 0xFF;
		}
		break;
	case 2:
		if (_value >= 100)
		{
			register_array[0] = segment_dis[(_value % 1000) / 100];
			register_array[1] = _DIG_2;
		}
		else
		{
			register_array[0] = 0xFF;
			register_array[1] = 0xFF;
		}
		break;
	case 3:
		register_array[0] = 0xFF - _char;
		register_array[0] &= (0xFF - _DISPLAY_DP);
		register_array[1] = _DIG_3;
		break;
	}
	sendSerialData(2, register_array);
//	delay(_DIS_DELAY_TIME);
}

void HM_7SEG_SHIFT::timeDis(uint16_t seconds, uint16_t minutes)
{
	next_time_dis++;
	if (next_time_dis > 3)
		next_time_dis = 0;
	switch (next_time_dis)
	{
	case 0:
		register_array[0] = segment_dis[seconds % 10];
		register_array[1] = _DIG_0;
		break;
	case 1:
		register_array[0] = segment_dis[seconds / 10];
		register_array[1] = _DIG_1;
		break;
	case 2:
		register_array[0] = segment_dis[minutes % 10];
		register_array[0] &= (0xFF - _DISPLAY_DP);
		register_array[1] = _DIG_2;
		break;
	case 3:
		if (minutes >= 10)
		{
			register_array[0] = segment_dis[(minutes % 100) / 10];
			register_array[1] = _DIG_3;
		}
		else
		{
			register_array[0] = 0xFF;
			register_array[1] = 0xFF;
		}
		break;
	}
	sendSerialData(2, register_array);
//	delay(_DIS_DELAY_TIME);
}

void HM_7SEG_SHIFT::valueDis(uint16_t value)
{
//	value_dis_time--;
	//value_dis_time = 0;
	//if (value_dis_time <= 0)
	//{
	uint16_t _true_value = value;
	if (value != last_value_dis)
	{
		if (value > 49999)
			value = 49999;

		if (value >= 40000)
			value -= 40000;
		else if (value >= 30000)
			value -= 30000;
		else if (value >= 20000)
			value -= 20000;
		else if (value >= 10000)
			value -= 10000;

		last_value_dis = value;
	}

	switch (next_value_dis)
	{
	case 0:
		register_array[0] = segment_dis[value % 10];
		if (_true_value >= 10000)
			register_array[0] &= (0xFF - _DISPLAY_DP);
		register_array[1] = _DIG_0;
		break;
	case 1:
		if (_true_value >= 20000)
		{
			if (value >= 10) {
				register_array[0] = segment_dis[(value % 100) / 10];
			}
			else {
				register_array[0] = segment_dis[0];
			}
			register_array[0] &= (0xFF - _DISPLAY_DP);
			register_array[1] = _DIG_1;
		}
		else if (value >= 10)
		{
			register_array[0] = segment_dis[(value % 100) / 10];
			register_array[1] = _DIG_1;
		}
		else
		{
			register_array[0] = 0xFF;
			register_array[1] = 0xFF;
		}
		break;
	case 2:
		if (_true_value >= 30000)
		{
			if (value >= 100) {
				register_array[0] = segment_dis[(value % 1000) / 100];
			}
			else {
				register_array[0] = segment_dis[0];
			}
			register_array[0] &= (0xFF - _DISPLAY_DP);
			register_array[1] = _DIG_2;
		}
		else if (value >= 100)
		{
			register_array[0] = segment_dis[(value % 1000) / 100];
			register_array[1] = _DIG_2;
		}
		else
		{
			register_array[0] = 0xFF;
			register_array[1] = 0xFF;
		}
		break;
	case 3:
		if (_true_value >= 40000)
		{
			if (value >= 1000) {
				register_array[0] = segment_dis[value / 1000];
			}
			else {
				register_array[0] = segment_dis[0];
			}
			register_array[0] &= (0xFF - _DISPLAY_DP);
			register_array[1] = _DIG_3;
		}
		else if (value >= 1000)
		{
			register_array[0] = segment_dis[value / 1000];
			register_array[1] = _DIG_3;
		}
		else
		{
			register_array[0] = 0xFF;
			register_array[1] = 0xFF;
		}
		break;
	}

	next_value_dis++;
	if (next_value_dis > 3)
		next_value_dis = 0;

	//value_dis_time = _VALUE_DIS_DELAY_TIME;
	sendSerialData(2, register_array);
	//delay(_DIS_DELAY_TIME);
//	}
	//else
	//	delay(_DIS_DELAY_TIME);
}

void HM_7SEG_SHIFT::valueDis2(uint16_t value)
{
	uint16_t _true_value = value;
	//	if(value != last_value_dis)
	//	{
	//		if(value > 49999)
	//			value = 49999;
	//		
	//		if (value >= 40000)
	//			value -= 40000;
	//		else if (value >= 30000)
	//			value -= 30000;
	//		else if (value >= 20000)
	//			value -= 20000;
	//		else if (value >= 10000)
	//			value -= 10000;
	//		
	//		last_value_dis = value;
	//	}

	switch (next_value_dis)
	{
	case 0:
		register_array[0] = segment_dis[value % 10];
		if (_true_value >= 10000)
			register_array[0] &= (0xFF - _DISPLAY_DP);
		register_array[1] = _DIG_0;
		break;
	case 1:
		if (_true_value >= 20000)
		{
			if (value >= 10) {
				register_array[0] = segment_dis[(value % 100) / 10];
			}
			else {
				register_array[0] = segment_dis[0];
			}
			register_array[0] &= (0xFF - _DISPLAY_DP);
			register_array[1] = _DIG_1;
		}
		else if (value >= 10)
		{
			register_array[0] = segment_dis[(value % 100) / 10];
			register_array[1] = _DIG_1;
		}
		else
		{
			register_array[0] = 0xFF;
			register_array[1] = 0xFF;
		}
		break;
	case 2:
		if (_true_value >= 30000)
		{
			if (value >= 100) {
				register_array[0] = segment_dis[(value % 1000) / 100];
			}
			else {
				register_array[0] = segment_dis[0];
			}
			register_array[0] &= (0xFF - _DISPLAY_DP);
			register_array[1] = _DIG_2;
		}
		else if (value >= 100)
		{
			register_array[0] = segment_dis[(value % 1000) / 100];
			register_array[1] = _DIG_2;
		}
		else
		{
			register_array[0] = 0xFF;
			register_array[1] = 0xFF;
		}
		break;
	case 3:
		if (_true_value >= 40000)
		{
			if (value >= 1000) {
				register_array[0] = segment_dis[value / 1000];
			}
			else {
				register_array[0] = segment_dis[0];
			}
			register_array[0] &= (0xFF - _DISPLAY_DP);
			register_array[1] = _DIG_3;
		}
		else if (value >= 1000)
		{
			register_array[0] = segment_dis[value / 1000];
			register_array[1] = _DIG_3;
		}
		else
		{
			register_array[0] = 0xFF;
			register_array[1] = 0xFF;
		}
		break;
	}

	next_value_dis++;
	if (next_value_dis > 3)
		next_value_dis = 0;

	sendSerialData(2, register_array);
//	delay(_DIS_DELAY_TIME + 1);
}

void HM_7SEG_SHIFT::ballDisplay(void)
{
	ball_dis_time--;
	if (ball_dis_time <= 0)
	{
		switch (next_ball_dis)
		{
		case 0:
			register_array[1] = _DIG_3;
			ballUpDis();
			break;
		case 1:
			register_array[1] = _DIG_2;
			ballDownDis();
			break;
		case 2:
			register_array[1] = _DIG_1;
			ballUpDis();
			break;
		case 3:
			register_array[1] = _DIG_0;
			ballDownDis();
			break;
		case 4:
			register_array[1] = _DIG_0;
			ballUpDis();
			break;
		case 5:
			register_array[1] = _DIG_1;
			ballDownDis();
			break;
		case 6:
			register_array[1] = _DIG_2;
			ballUpDis();
			break;
		case 7:
			register_array[1] = _DIG_3;
			ballDownDis();
			break;
		}

		next_ball_dis++;
		if (next_ball_dis > 7)
			next_ball_dis = 0;

		ball_dis_time = _BAll_DIS_DELAY;

//		delay(_DIS_DELAY_TIME);
	}
//	else
//		delay(_DIS_DELAY_TIME);
}

void HM_7SEG_SHIFT::loadingDisplay(void)
{
	step_loading_delay_time--;
	if (step_loading_delay_time <= 0)
	{
		switch (next_step)
		{
		case 0:
			register_array[0] = 0xFF - _DIS_UPPER_LINE;
			register_array[1] = _DIG_3;
			break;
		case 1:
			register_array[0] = 0xFF - _DIS_UPPER_LINE;
			register_array[1] = _DIG_2;
			break;
		case 2:
			register_array[0] = 0xFF - _DIS_UPPER_LINE;
			register_array[1] = _DIG_1;
			break;
		case 3:
			register_array[0] = 0xFF - _DIS_UPPER_LINE;
			register_array[1] = _DIG_0;
			break;
		case 4:
			register_array[0] = 0xFF - _DIS_UPPER_RIGHT_VER_LINE;
			register_array[1] = _DIG_0;
			break;
		case 5:
			register_array[0] = 0xFF - _DIS_LOWER_RIGHT_VER_LINE;
			register_array[1] = _DIG_0;
			break;
		case 6:
			register_array[0] = 0xFF - _DIS_LOWER_LINE;
			register_array[1] = _DIG_0;
			break;
		case 7:
			register_array[0] = 0xFF - _DIS_LOWER_LINE;
			register_array[1] = _DIG_1;
			break;
		case 8:
			register_array[0] = 0xFF - _DIS_LOWER_LINE;
			register_array[1] = _DIG_2;
			break;
		case 9:
			register_array[0] = 0xFF - _DIS_LOWER_LINE;
			register_array[1] = _DIG_3;
			break;
		case 10:
			register_array[0] = 0xFF - _DIS_LOWER_LEFT_VER_LINE;
			register_array[1] = _DIG_3;
			break;
		case 11:
			register_array[0] = 0xFF - _DIS_UPPER_LEFT_VER_LINE;
			register_array[1] = _DIG_3;
			break;
		}

		step_loading_delay_time = _STEP_LOADING_DELAY_TIME;
		next_step++;
		if (next_step > 11)
			next_step = 0;

		sendSerialData(2, register_array);
//		delay(_DIS_DELAY_TIME);
	}
//	else
//		delay(_DIS_DELAY_TIME);
}

void HM_7SEG_SHIFT::clrBallDis(void)
{
	next_ball_dis = 0;
	ball_dis_time = _BAll_DIS_DELAY;
}

void HM_7SEG_SHIFT::clrLoadingDis(void)
{
	next_step = 0;
	step_loading_delay_time = _STEP_LOADING_DELAY_TIME;
}

void HM_7SEG_SHIFT::ballUpDis(void)
{
	register_array[0] = 0xFF - _DISPLAY_BAll_UP; //BollUp
	sendSerialData(2, register_array);
}

void HM_7SEG_SHIFT::ballDownDis(void)
{
	register_array[0] = 0xFF - _DISPLAY_BAll_DOWN; //BollLow
	sendSerialData(2, register_array);
}

void HM_7SEG_SHIFT::disOff1(void)
{
	register_array[0] = 0xFF;
	register_array[1] = 0xFF;
	sendSerialData(2, register_array);
}

void HM_7SEG_SHIFT::disOff2(void)
{
	register_array[0] = 0xFF;
	register_array[1] = 0xFF;
	sendSerialData(2, register_array);
	register_array[0] = 0xFF;
	register_array[1] = 0xFF;
	sendSerialData(2, register_array);
}

void HM_7SEG_SHIFT::testDis(void)
{
	register_array[0] = 0xFF - _DISPLAY_C;
	register_array[1] = _DIG_3;
	sendSerialData(2, register_array);
	delay(10);

	register_array[0] = 0xFF;
	register_array[1] = _DIG_2;
	sendSerialData(2, register_array);
	delay(1);

	register_array[0] = 0xFF;
	register_array[1] = _DIG_1;
	sendSerialData(2, register_array);
	delay(1);

	register_array[0] = 0xFF;
	register_array[1] = _DIG_0;
	sendSerialData(2, register_array);
	delay(1);

	//	disOff1();
	//	delay(5);
}

void HM_7SEG_SHIFT::loopTime(uint16_t _t)
{
	uint16_t _i = 0;
	uint8_t p = 0;
	for (_i = 0; _i < _t; _i++)
	{
		p = 0;
	}
}

void HM_7SEG_SHIFT::sendSerialData(int16_t registerCount, int16_t *pValueArray)   // How many shift registers?, // Array of bytes with LSByte in array [0]
{
	int reg, bitMask, value;
	uint16_t _loop = 50;
	digitalWrite(latch_pin, LOW);
	for (reg = registerCount; reg > 0; reg--)
	{
		value = pValueArray[reg - 1];
		for (bitMask = 128; bitMask > 0; bitMask >>= 1)
		{
			digitalWrite(clk_pin, LOW);
//			data_pin = value & bitMask ? 1 : 0;
			digitalWrite(data_pin, value & bitMask ? 1 : 0);
			digitalWrite(clk_pin, HIGH);
		}
	}
	digitalWrite(latch_pin, HIGH);
}// sendSerialData


