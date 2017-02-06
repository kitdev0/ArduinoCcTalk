// hm_cctalk_device_v104.h

#ifndef _HM_CCTALK_DEVICE_V104_h
#define _HM_CCTALK_DEVICE_V104_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "hm_mini_coffee_rev1_pin_define.h"
#include "sm_debug_v102.h"
#include "sm_timeout.h"

#ifndef _DEBUG_SAY_ONLY
#define _DEBUG_SAY_ONLY 0
#endif //_DEBUG_SAY_ONLY

#ifndef _DEBUG_WRITE_ONLY //Set write to se card
#define _DEBUG_WRITE_ONLY 1
#endif //_DEBUG_WRITE_ONLY

#ifndef _DEBUG_SAY_AND_WRITE
#define _DEBUG_SAY_AND_WRITE 2
#endif //_DEBUG_PRINT_AND_WRITE

#define _CCTALK_DEVICE_DEBUG _DEBUG_SAY_ONLY
//#define _CCTALK_DEVICE_DEBUG _DEBUG_WRITE_ONLY
//#define _CCTALK_DEVICE_DEBUG _DEBUG_SAY_AND_WRITE

#ifndef _PIN_COIN_ACC_PWR
#define _PIN_COIN_ACC_PWR 9
#endif // !_COIN_ACC_PWR_PIN

#ifndef _PIN_BILL_ACC_PWR
#define _PIN_BILL_ACC_PWR 10
#endif // !_BILL_ACC_PWR_PIN

class HM_CCTALK_DEVICE
{
 private:
	 SM_DEBUG	*logDebug;
	 SM_TIMEOUT	coinTimeout;
	 SM_TIMEOUT	billTimeout;

	 bool coin_acc_pwr_state = LOW;
	 bool bill_acc_pwr_state = LOW;

	 volatile int16_t coin_value = 0;
	 volatile int16_t bill_value = 0;

	 int8_t bill_error_cnt = 0;
	 int8_t coin_state = 0;
	 int8_t last_coin_state = -2;

	 int8_t bill_state = 0;
	 int8_t last_bill_state = -2;

	 uint8_t coin_acc_pwr_pin = _PIN_COIN_ACC_PWR;
	 uint8_t bill_acc_pwr_pin = _PIN_BILL_ACC_PWR;
	 uint8_t read_coin_event_interval = 0;
	 uint8_t read_bill_event_interval = 0;

	 bool flag_coin_enable = false;
	 bool flag_bill_enable = false;
	 void debug(String data);

 public:
	 HM_CCTALK_DEVICE();
	 ~HM_CCTALK_DEVICE();
	 void portInit(HardwareSerial *_serial);
	 bool coinInit(void);
	 bool coinCheckReady(void);
	 void coinDisable(void);
	 void coinEnable(void);
	 void coinPwrSet(bool _state);
	 void coinPwrPinDefine(uint8_t _pin);
	 void coinReadEvent(void);
	 bool coinRealTimeStateCheck(void);
	 int16_t coinGetValue(void);
	 int8_t coinState(void);
	 bool coinIsEnable(void);


	 bool billInit(void);
	 bool billCheckReady(void);
	 void billDisable(void);
	 void billEnable(void);
	 void billPwrSet(bool _value);
	 void billPwrPinDefine(uint8_t _pin);
	 void billReadEvent(void);
	 bool billRealTimeStateCheck(void);
	 int16_t billGetValue(void);
	 int8_t billState(void);
	 bool billIsEnable(void);

};

#endif

