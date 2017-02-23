#include <TimerOne.h>
#include "ArduinoDebug.h"

//Define Validator Power Pin
#define _PIN_COIN_ACC_PWR	42//9//42
#define _PIN_BILL_ACC_PWR	40//8//40

#include "ArduinoCcTalkDevice.h"

#define _CONSOLE_BAUDRATE_DEFINE	115200

#define _MAIN_DEBUG _DEBUG_SAY_ONLY
//#define _MAIN_DEBUG _DEBUG_WRITE_ONLY
//#define _MAIN_DEBUG _DEBUG_SAY_AND_WRITE

ARDUINO_DEBUG				logDebug("MAIN");
ARDUINO_CCTALK_DEVICE		ccTalkDevice;

///
uint32_t current_millis = 0;
uint32_t previous_millis = 0;

uint8_t time_1ms_cnt;
uint8_t time_10ms_cnt;
uint8_t time_100ms_cnt;
uint8_t time_500ms_cnt;
bool time_1ms_flag;
bool time_10ms_flag;
bool time_100ms_flag;
bool time_500ms_flag;
bool time_1000ms_flag;
///

//coin_state code
// code -1 = can't connect to coin acceptor.

int8_t last_coin_acc_error_code = 0;
int8_t last_bill_acc_error_code = 0;

uint16_t total_coin_value = 0;
uint16_t total_bill_value = 0;

void setup()
{
	logDebug.init();

	debug(F("------------------------------------------"));
	debug(F("##  ArduinoCcTalkLib - Example >> Start..."));

	ccTalkDevice.portInit(&Serial3); //Init. Serial port for ccTalk Port.
	ccTalkDevice.billPwrPinDefine(_PIN_BILL_ACC_PWR); delay(500);
	ccTalkDevice.coinPwrPinDefine(_PIN_COIN_ACC_PWR); delay(500);
	ccTalkDevice.billPwrSet(HIGH); delay(2000);
	ccTalkDevice.coinPwrSet(HIGH); delay(500);

	ccTalkDevice.coinInit();
	ccTalkDevice.billInit();

}

void loop()
{
	timerTask();

	//Coin Acceptor Segment
	readAcceptCoinValue();
	ccTalkDevice.coinRealTimeStateCheck();
	if (ccTalkDevice.coinState() != 0)//if coin acceptor is not raedy
	{
		int8_t _error_code = ccTalkDevice.coinState();
		if(_error_code != last_coin_acc_error_code){
			if (_error_code == -1) {
				debug("Can't connect to Coin acc.");
			}
			else{
				debug("Coin acc. Error Code = " + String(_error_code));
			}
			last_coin_acc_error_code = _error_code;
		}
	}
	else
	{
		last_coin_acc_error_code = 0;
	}


	//Bill Acceptor Segment
	readAcceptBillValue();
	ccTalkDevice.billRealTimeStateCheck();
	if (ccTalkDevice.billState() != 0)//if bill acceptor is not ready
	{
		int8_t _error_code = ccTalkDevice.billState();
		if (_error_code != last_bill_acc_error_code){
			if (_error_code == -1) {
				debug("Can't connect to Bill acc.");
			}
			else {
				debug("Bill acc. Error Code = " + String(_error_code));
			}
			last_bill_acc_error_code = _error_code;
		}
	}
	else
	{
		last_bill_acc_error_code = 0;
	}
}

void readAcceptCoinValue()
{
	int16_t _value = ccTalkDevice.coinGetValue();
	if (_value > 0) {
		total_coin_value += _value;
		debug("get Coin value = " + String(_value));
		debug("Total Coin value = " + String(total_coin_value));
	}
}

void readAcceptBillValue()
{
	if (ccTalkDevice.billIsAvailable()) {
		int16_t _value = ccTalkDevice.billValue();
		//total_bill_value += _value;
		debug("Bill verify value = " + String(_value));
		ccTalkDevice.billAccept();
		// ccTalkDevice.
		//debug("Total Bill value = " + String(total_bill_value));
	}

	if (ccTalkDevice.billIsAccepted()) {
		int16_t _value = ccTalkDevice.billValue();
		total_bill_value += _value;
		debug("Bill received value = " + String(_value));
		debug("Total Bill value = " + String(total_bill_value));
	}
}

//////////////////////////////////////////////////

void updateTimerCnt(void)
{
	time_1ms_cnt += uint8_t(current_millis - previous_millis);
	previous_millis = current_millis;

	if (time_1ms_cnt >= 10) {
		time_1ms_cnt = 0;
		time_10ms_cnt++;
		time_10ms_flag = 1;
		if (time_10ms_cnt >= 10) {
			time_10ms_cnt = 0;
			time_100ms_cnt++;
			time_100ms_flag = 1;
			if (time_100ms_cnt >= 5) {
				time_100ms_cnt = 0;
				time_500ms_cnt++;
				time_500ms_flag = 1;
				if (time_500ms_cnt >= 2) {
					time_500ms_cnt = 0;
					time_1000ms_flag = 1;
				}
			}
		}
	}
}

void timerTask(void)
{
	if (time_10ms_flag)
	{
		appTask10ms();
		time_10ms_flag = 0;
	}

	if (time_100ms_flag)
	{
		appTask100ms();
		time_100ms_flag = 0;
	}

	if (time_500ms_flag)
	{
		appTask500ms();
		time_500ms_flag = 0;
	}

	if (time_1000ms_flag)
	{
		appTask1000ms();
		time_1000ms_flag = 0;
	}

	current_millis = millis();
	if (current_millis - previous_millis > 1)
		updateTimerCnt();
	else if (current_millis < previous_millis)
		previous_millis = current_millis;
}

void appTask10ms(void)
{

}

void appTask100ms(void)
{
	ccTalkDevice.coinReadEvent();
	ccTalkDevice.billReadEvent();
}

void appTask500ms(void)
{

}

void appTask1000ms(void)
{

}

void wait(uint16_t time)
{
	unsigned long _last_time = millis();
	while (millis() - _last_time < time || millis() - _last_time < 0)
	{
		timerTask();
	}
}

void debug(String data)
{
#ifdef _MAIN_DEBUG
#if _MAIN_DEBUG == _DEBUG_SAY_ONLY
	logDebug.sayln(data);
#elif _MAIN_DEBUG == _DEBUG_WRITE_ONLY
	logDebug.writeLog(data);
#elif _MAIN_DEBUG == _DEBUG_SAY_AND_WRITE
	logDebug.sayAndWriteLog(data);
#endif
#endif
}
