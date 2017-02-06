#include "ArduinoCcTalkDevice.h"
#include "ArduinoCcTalk.h"

ARDUINO_CCTALK	ccTalk;

ARDUINO_CCTALK_DEVICE::ARDUINO_CCTALK_DEVICE()
{
#ifdef _CCTALK_DEVICE_DEBUG
	logDebug = new ARDUINO_DEBUG(F("CCTALK-DEVICE"));
#endif // _CCTALK_DEVICE_DEBUG
}

ARDUINO_CCTALK_DEVICE::~ARDUINO_CCTALK_DEVICE()
{

}

void ARDUINO_CCTALK_DEVICE::debug(String data)
{
#ifdef _CCTALK_DEVICE_DEBUG
#if _CCTALK_DEVICE_DEBUG == _DEBUG_SAY_ONLY
	logDebug->sayln(data);
#elif _CCTALK_DEVICE_DEBUG == _DEBUG_WRITE_ONLY
	logDebug->writeLog(data);
#elif _CCTALK_DEVICE_DEBUG == _DEBUG_SAY_AND_WRITE
	logDebug->sayAndWriteLog(data);
#endif
#endif
}

void ARDUINO_CCTALK_DEVICE::portInit(HardwareSerial *_serial)
{
	debug("portInit");
	ccTalk.init(_serial);
}

//-------------------------------------------------//
//------------- Coin Acceptor Device --------------//
//-------------------------------------------------//
bool ARDUINO_CCTALK_DEVICE::coinInit(void)
{
	// uint16_t _delay = 1000;

	debug("CoinAcc. Initial");
	//coinPwrSet(LOW);
	//delay(_delay);

	//coinPwrSet(HIGH);
	//delay(_delay + 1000);

	coin_state = -1;
	if (!coinCheckReady())
		return 0;
	else
		flag_coin_enable = true;
	return 1;

}

bool ARDUINO_CCTALK_DEVICE::coinCheckReady(void)
{
//	debug("Check Coin Ready?");
	if (ccTalk.resetDevice(ccTalk_ADDR_COIN))
	{
		// int8_t _last_coin_state = coin_state;
		if(coin_state == -1)
			debug("CoinAcc. Connection >> OK");
		delay(200);
		ccTalk.selfCheck(ccTalk_ADDR_COIN);
		delay(200);
		coin_state = ccTalk.getFaultCode();
		if (coin_state == 0)
		{
//			if (_last_coin_state == -1) {
				ccTalk.setUnInhibit(ccTalk_ADDR_COIN);
				coinEnable();
//			}
		}
		else if (coin_state > 0) {
			if (coin_state < 10) {
				if (coin_state != last_coin_state) {
					debug("CoinAcc. Status >> E0" + String(coin_state));
					last_coin_state = coin_state;
				}
			}
			else {
				if (coin_state != last_coin_state) {
					debug("CoinAcc. Status >> E" + String(coin_state));
					last_coin_state = coin_state;
				}
			}
		}
		else {
			if (coin_state != last_coin_state) {
				debug("CoinAcc. Connection >> Error");
				debug("CoinAcc. Status >> E00");
				last_coin_state = coin_state;
			}
		}
	}
	else {
		coin_state = -1;
		if (coin_state != last_coin_state) {
			debug("CoinAcc. Connection >> Error");
			debug("CoinAcc. Status >> E00");
			last_coin_state = coin_state;
		}
	}

	if (coin_state == 0)
	{
		if (coin_state != last_coin_state) {
			debug("CoinAcc. Status >> Ready");
			last_coin_state = coin_state;
		}
		coinTimeout.reset();
		return 1;
	}
	return 0;
}

void ARDUINO_CCTALK_DEVICE::coinDisable(void)
{
	if (coin_state != 0) //coin not ready
		return;
	ccTalk.setMaster(ccTalk_ADDR_COIN, 0);
	flag_coin_enable = false;
}

void ARDUINO_CCTALK_DEVICE::coinEnable(void)
{
	if (coin_state != 0) //coin not ready
		return;
	if (ccTalk.setMaster(ccTalk_ADDR_COIN, 1))
		flag_coin_enable = true;
	else
		flag_coin_enable = false;
}

void ARDUINO_CCTALK_DEVICE::coinPwrSet(bool _value)
{
	if (_value != coin_acc_pwr_state)
	{
		if (_value)
			debug("Set Coin Acc. Power >> HIGH");
		else
			debug("Set Coin Acc. Power >> LOW");
		digitalWrite(coin_acc_pwr_pin, _value);
		coin_acc_pwr_state = _value;
	}

	coinTimeout.reset();
	while (!coinTimeout.check(500))
	{
		ccTalk.clrReadBuffer();
	}

}

void ARDUINO_CCTALK_DEVICE::coinPwrPinDefine(uint8_t _pin)
{
	coin_acc_pwr_pin = _pin;
	pinMode(coin_acc_pwr_pin, OUTPUT);
	coin_acc_pwr_state = HIGH;

	coinPwrSet(LOW);
}

void ARDUINO_CCTALK_DEVICE::coinReadEvent(void)
{
	int8_t _value = 0;
	if (coin_state == 0 && flag_coin_enable)
	{
		read_coin_event_interval++;
		if (read_coin_event_interval >= 2)
		{
			if (ccTalk.readEvent(ccTalk_ADDR_COIN)) {
				_value = ccTalk.checkBuffCoin();
				if (_value != -1)
					coin_value += _value;
				else
					coinCheckReady();
				read_coin_event_interval = 0;
			}
			else {
				coinCheckReady();
			}
		}
	}
}

bool ARDUINO_CCTALK_DEVICE::coinRealTimeStateCheck(void)
{
	if (coin_state != 0)
	{
		if (coinTimeout.check(500)) {
			coinCheckReady();
			coinTimeout.reset();
		}
		return 0;
	}
	return 1;
}

int16_t ARDUINO_CCTALK_DEVICE::coinGetValue(void)
{
	int16_t _value = coin_value;
	coin_value = 0;
	return _value;
}

int8_t ARDUINO_CCTALK_DEVICE::coinState(void)
{
	return coin_state;
}

bool ARDUINO_CCTALK_DEVICE::coinIsEnable(void)
{
	return flag_coin_enable;
}


//-------------------------------------------------//
//------------- Bill Acceptor Device --------------//
//-------------------------------------------------//
bool ARDUINO_CCTALK_DEVICE::billInit(void)
{
	// uint16_t _delay = 1000;
	debug("BillAcc. Initial");

	bill_state = -1;
	if (!billCheckReady())
		return 0;
	else
		flag_bill_enable = true;
	return 1;
}

bool ARDUINO_CCTALK_DEVICE::billCheckReady(void)
{
	if (ccTalk.resetDevice(ccTalk_ADDR_BILL))
	{
		// int8_t _last_bill_state = bill_state;
		if (bill_state == -1)
			debug("BillAcc. Connection >> OK");
		delay(200);
		ccTalk.selfCheck(ccTalk_ADDR_BILL);
		delay(200);
		bill_state = ccTalk.getFaultCode();
		if (bill_state == 0)
		{
			//			if (_last_coin_state == -1) {
			ccTalk.setUnInhibit(ccTalk_ADDR_BILL);
			billEnable();
			//			}
		}
		else if (bill_state > 0) {
			if (bill_state < 10) {
				if (bill_state != last_bill_state) {
					debug("BillAcc. Status >> E0" + String(bill_state));
					last_bill_state = bill_state;
				}
			}
			else {
				if (bill_state != last_bill_state) {
					debug("BillAcc. Status >> E" + String(bill_state));
					last_bill_state = bill_state;
				}
			}
		}
		else {
			if (bill_state != last_bill_state) {
				debug("BillAcc. Connection >> Error");
				debug("BillAcc. Status >> E00");
				last_bill_state = bill_state;
			}
		}
	}
	else {
		bill_state = -1;
		if (bill_state != last_bill_state) {
			debug("BillAcc. Connection >> Error");
			debug("BillAcc. Status >> E00");
			last_bill_state = bill_state;
		}
	}

	if (bill_state == 0)
	{
		if (bill_state != last_bill_state) {
			debug("BillAcc. Status >> Ready");
			bill_error_cnt = 0;
			last_bill_state = bill_state;
		}
		billTimeout.reset();
		return 1;
	}
	return 0;
}

void ARDUINO_CCTALK_DEVICE::billDisable(void)
{
	if (bill_state != 0) //coin not ready
		return;
	ccTalk.setMaster(ccTalk_ADDR_BILL, 0);
	flag_bill_enable = false;
}

void ARDUINO_CCTALK_DEVICE::billEnable(void)
{
	if (bill_state != 0) //coin not ready
		return;
	if (ccTalk.setMaster(ccTalk_ADDR_BILL, 1))
		flag_bill_enable = true;
	else
		flag_bill_enable = false;
}

void ARDUINO_CCTALK_DEVICE::billPwrSet(bool _value)
{
	if (_value != bill_acc_pwr_state)
	{
		if (_value)
			debug("Set Bill Acc. Power >> HIGH");
		else
			debug("Set Bill Acc. Power >> LOW");
		digitalWrite(bill_acc_pwr_pin, _value);
		bill_acc_pwr_state = _value;
	}

	billTimeout.reset();
	while (!billTimeout.check(500))
	{
		ccTalk.clrReadBuffer();
	}
}

void ARDUINO_CCTALK_DEVICE::billPwrPinDefine(uint8_t _pin)
{
	bill_acc_pwr_pin = _pin;
	pinMode(bill_acc_pwr_pin, OUTPUT);
	bill_acc_pwr_state = HIGH;

	billPwrSet(LOW);
}

void ARDUINO_CCTALK_DEVICE::billReadEvent(void)
{
	// int8_t _value = 0;
	if (bill_state == 0 && flag_bill_enable)
	{
		read_bill_event_interval++;
		if (read_bill_event_interval >= 4)
		{
			if (ccTalk.readEvent(ccTalk_ADDR_BILL)) {
				ccTalk.checkBuffBill();

				if (ccTalk.checkBuffBill() == -1) {
					bill_error_cnt++;
					if (bill_error_cnt >= 5) {
						billDisable();
						return;
					}
					billCheckReady();
				}

				read_bill_event_interval = 0;
			}
			else {
				billCheckReady();
			}
		}
	}
}

bool ARDUINO_CCTALK_DEVICE::billRealTimeStateCheck(void)
{
	if (bill_state != 0)
	{
		if (billTimeout.check(500)) {
			billCheckReady();
			billTimeout.reset();
		}
		return 0;
	}
	return 1;
}

bool ARDUINO_CCTALK_DEVICE::billIsVerfied(void)
{
	return ccTalk.billAvailable();
}

int8_t ARDUINO_CCTALK_DEVICE::billState(void)
{
	return bill_state;
}

bool ARDUINO_CCTALK_DEVICE::billIsEnable(void)
{
	return flag_bill_enable;
}
