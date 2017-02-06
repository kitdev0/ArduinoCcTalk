#ifndef _HM_MINI_COFFEE_REV1_PIN_DEFINE_h
#define _HM_MINI_COFFEE_REV1_PIN_DEFINE_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

//#define _PIN_SIGNAL_BUZZER_INPUT		0		//Analog pin
#define _PIN_SIGNAL_BUZZER_INPUT		27

#define _PIN_SIGNAL_LED_OPERATING	43
#define _PIN_SIGNAL_LED_HEATING		44
#define _PIN_CONFIRM_ORDER_1		45
#define _PIN_CONFIRM_ORDER_2		46
#define _PIN_CONFIRM_ORDER_3		47
#define _PIN_SIGNAL_SENSOR_CUP		48
#define _PIN_SIGNAL_COIN_OUTPUT		49
#define _PIN_SIGNAL_MIX_MOTOR_1		51
#define _PIN_SIGNAL_MIX_MOTOR_2		53
#define _PIN_SIGNAL_MIX_MOTOR_3		52
#define _PIN_SIGNAL_TEMP_VALUE		1		//Analog pin
#define _PIN_SIGNAL_WATER_LEVEL		50

#define _PIN_BT_ITEM_SELECT_1		31
#define _PIN_BT_ITEM_SELECT_2		33
#define _PIN_BT_ITEM_SELECT_3		35
#define _PIN_BT_ITEM_SELECT_4		37
#define _PIN_BT_ITEM_SELECT_5		39
#define _PIN_BT_ITEM_SELECT_6		41

#define _PIN_LED_ORDER_1	32
#define _PIN_LED_ORDER_2	34
#define _PIN_LED_ORDER_3	36
#define _PIN_LED_ORDER_4	38
#define _PIN_LED_ORDER_5	40
#define _PIN_LED_ORDER_6	42

#define _PIN_LED_STATUS		13

#define _PIN_COIN_ACC_PWR	9
#define _PIN_BILL_ACC_PWR	8

#define _PIN_SEGMENT_CLK	6
#define _PIN_SEGMENT_DATA	5
#define _PIN_SEGMENT_LATCH	4

#define _PIN_PUMP_1_PWR		3
#define _PIN_PUMP_2_PWR		2

#define _PIN_LED_CUP_HANDLE	38
#define _PIN_DOOR_SWITCH	39

#endif // !_HM_MINI_COFFEE_REV1_PIN_DEFINE_h
