//############################################################################//
//----------------Standard Message Packets, Simple checksum-------------------//
//
//  For a payload of N data bytes�
//      [ Destination Address ]
//      [ No. of Data Bytes ]
//      [ Source Address ]
//      [ Header ]
//      [ Data 1 ]
//      ...
//      [ Data N ]
//      [ Checksum ]
//
//  For a simple command or request with no data bytes�
//      [ Destination Address ]
//      [ 0 ]
//      [ Source Address ]
//      [ Header ]
//      [ Checksum ]
//
//  Simple Checksum (modulo 256)...
//          For example, the message [ 1 ] [ 0 ] [ 2 ] [ 0 ] would be followed
//      by the checksum [ 253 ] because 1 + 0 + 2 + 0 + 253 = 256 = 0.
//############################################################################//
#include "hm_cctalk_v201.h"

HM_CCTALK::HM_CCTALK()
{
#ifdef _CCTALK_DEBUG
	logDebug = new SM_DEBUG(F("CCTALK"));
#endif // _CCTALK_DEBUG
}

HM_CCTALK::~HM_CCTALK()
{
#ifdef _CCTALK_DEBUG
	delete logDebug;
#endif // _CCTALK_DEBUG
	delete serial_port;
}

// private:

void HM_CCTALK::debug(String data)
{
#ifdef _CCTALK_DEBUG
#if _CCTALK_DEBUG == _DEBUG_SAY_ONLY
	logDebug->sayln(data);
#elif _CCTALK_DEBUG == _DEBUG_WRITE_ONLY
	logDebug->writeLog(data);
#elif _CCTALK_DEBUG == _DEBUG_SAY_AND_WRITE
	logDebug->sayAndWriteLog(data);
#endif
#endif
}

void HM_CCTALK::timeoutReset(void)
{
	previous_time = millis();
}

bool HM_CCTALK::timeoutCheck(uint32_t _time)
{
	if (millis() < previous_time)
		timeoutReset();
	if (millis() - previous_time >= _time)
	{
		return 1;
	}
	return 0;
}

//  public:
void HM_CCTALK::init(HardwareSerial *_port)
{
	serial_port = _port;
	serial_baud = _CCTALK_BAUDRATE;
	serial_port->begin(serial_baud, SERIAL_8N1);
	serial_port->flush();
}

uint8_t HM_CCTALK::addCheckSum(uint8_t raw)//Add Data Checksum
{
	raw = 256 * ((raw / 256) + 1) - raw;
	return raw;
}

//bool sendCcTalkSimplePacket(void)
//{
//	int i=0;
//	serialWrite(_CCTALK_PORT,ccTalkPacket.SlaveAddr);
//	serialWrite(_CCTALK_PORT,ccTalkPacket.NumData);
//	serialWrite(_CCTALK_PORT,ccTalkPacket.MasterAddr);
//	serialWrite(_CCTALK_PORT,ccTalkPacket.Cmd);
//	for(i=0; i < ccTalkPacket.NumData ;i++)
//	{
//		serialWrite(_CCTALK_PORT,ccTalkPacket.Data[i]);
//	}
//	serialWrite(_CCTALK_PORT,ccTalkPacket.CheckSum);
//	return 1;
//}

bool HM_CCTALK::sendCcTalkSimplePacket(void)
{
	int i = 0;
	int _len = 0;
	uint8_t _buf[128];

	_buf[_len++] = ccTalkPacket.SlaveAddr;
	_buf[_len++] = ccTalkPacket.NumData;
	_buf[_len++] = ccTalkPacket.MasterAddr;
	_buf[_len++] = ccTalkPacket.Cmd;
	for (i = 0; i < ccTalkPacket.NumData; i++)
	{
		_buf[_len++] = ccTalkPacket.Data[i];
	}
	_buf[_len++] = ccTalkPacket.CheckSum;
	for (uint8_t p = 0; p < _len; p++) {
		//debug("#W >> " + String(_buf[p],DEC));
		serial_port->write(_buf[p]);
	}
}

//void HM_CCTALK::cctalkReadDataToBuffer(void)
//{
//	if(!SerialIsRxEmpty(_CCTALK_PORT))
//	{
//		for(;!SerialIsRxEmpty(_CCTALK_PORT);)
//		{
//				uint8_t c = SerialGetRx(_CCTALK_PORT);
//				if(g_u32comRbytes < RXBUFSIZE)
//				{
//						g_u8RecData[g_u32comRbytes] = u8InChar;
//						g_u32comRbytes++;
//				}
//		}
//	}
//}

bool HM_CCTALK::clrRepeatData(uint8_t _numData)
{
	timeoutReset();
	while (_numData)
	{
		if(serial_port->available())
		{
			uint8_t c = serial_port->read();
			//debug("#A >> " + String(c,DEC));
			_numData--;
			timeoutReset();
		}
		if (timeoutCheck(_RECEIVE_DATA_TIME_OUT)) {
			debug("!!Repeat Receive Data - Timeout!!");
			return 0;
		}
	}
	return 1;
}

bool HM_CCTALK::checkAck(void)
{
	unsigned char _checksum_zero = 0;
	int i;
	for (i = 0; i< 5; i++)
	{
		_checksum_zero = _checksum_zero + readACK[i];
	}
	if (((readACK[2] == ccTalk_ADDR_BILL) || (readACK[2] == ccTalk_ADDR_COIN)) && (readACK[3] == 0) && (_checksum_zero == 0))
	{
//		debug("Receive ACKED\r\n");
		return 1;
	}
	else
	{
		debug("!!Receive Data - Incorrect!!\r\n");
		return 0;
	}
}

bool HM_CCTALK::receiveACKData(void)
{
	int i = 0;
	int p = 0;
	char _str[64];
//	debug("receiveACKData");
	timeoutReset();
	for (i = 0; i < 5;)
	{
		for (; serial_port->available() && i < 5;)
		{
			uint8_t c = serial_port->read();
			//debug("#B >> " + String(c, DEC));
			readACK[i] = c;
			i++;
			timeoutReset();
		}
		if (timeoutCheck(_RECEIVE_DATA_TIME_OUT)) {
//			debug("!!Receive Data - Timeout!!\r\n");
			return 0;
		}
	}

	if (i == 5) {
		if (checkAck())
			return 1;
	}
	return 0;
}

bool HM_CCTALK::checkFault(void)
{
	unsigned char _checksum_zero = 0;
	int i;
	for (i = 0; i<6; i++)
	{
		_checksum_zero = _checksum_zero + readFault[i];
	}
	if (((readACK[2] == ccTalk_ADDR_BILL) || (readACK[2] == ccTalk_ADDR_COIN)) && _checksum_zero == 0)
	{
		//switch(readFault[4])//map error for sp115
		//{
		//	case 0:
		//		debug("Self-check --> OK!\r\n");
		//		errorFault = 0;//OK
		//		break;
		//	case 3:
		//		debug("Self-check --> Coin jammed!!\r\n");
		//		debug("Pls, CheckDevice.\r\n");
		//		errorFault = 2;//jam
		//	default :
		//		debug("Self-check --> Not OK!!\r\n");
		//		debug("Pls, CheckDevice.\r\n");
		//		errorFault = 3;//Not OK
		//}
		//debug("Error fault = " + String(readFault[4]));
		errorFault = readFault[4];
		return 1;
	}
	else
	{
		debug("Self check --> Error!!\r\n");
		debug("Pls, CheckDevice.\r\n");
		errorFault = -1;//disconnect
		return 0;
	}
}

uint8_t HM_CCTALK::receiveFuaultData(void)
{
	int i = 0;
	int p = 0;
	uint8_t _str[64];
	timeoutReset();
	for (i = 0; i < 6;)
	{
		for (; serial_port->available() && i < 6;)
		{
			uint8_t c = serial_port->read();
			//debug("# >> " + String(c, DEC));
			readFault[i] = c;
			i++;
			timeoutReset();
		}
		if (timeoutCheck(_RECEIVE_DATA_TIME_OUT)) {
//			debug("!!Receive Data - Timeout!!\r\n");
			return 0;
		}
	}
	if (i == 6) {
		return checkFault();
	}
	return 0;
}

int8_t HM_CCTALK::getFaultCode(void)
{
	return errorFault;
}

bool HM_CCTALK::receiveEventData(ccTalkAddr_t _slave)
{
	int i = 0;
	int p = 0;
	uint8_t _str[64];
	timeoutReset();
	for (i = 0; i < 16;)
	{
		for (; serial_port->available() && i < 16;)
		{
			uint8_t c = serial_port->read();
			//debug("# >> " + String(c, DEC));
			if (_slave == ccTalk_ADDR_COIN)
				readBuff_Coin[i] = c;
			if (_slave == ccTalk_ADDR_COIN)
				readBuff_Bill[i] = c;
			i++;
			timeoutReset();
		}
		if (timeoutCheck(_RECEIVE_DATA_TIME_OUT)) {
			debug("!!Receive Event Data - Timeout!!\r\n");
			return 0;
		}
	}
	return 1;
}

//bool HM_CCTALK::receiveEventData(ccTalkAddr_t _slave)
//{
//	int i=0;
//	int p=0;
//	const uint16_t RXBUFSIZE = 1024;
//	uint8_t g_u8RecData[RXBUFSIZE]  = {0};
//	volatile uint32_t g_u32comRbytes = 0;
//	volatile uint32_t g_u32comRtail  = 0;
//	
//	uint8_t _str[64];
//	receiveTimeOut = _RECEIVE_DATA_TIME_OUT;
////	printf("event : ");
////	for(i=0; i < 16;)
////	{
//		if(!((_CCTALK_PORT)->FSR & UART_FSR_RX_EMPTY_Msk))
//		{
//			while(!((_CCTALK_PORT)->FSR & UART_FSR_RX_EMPTY_Msk))
//			{
//				uint8_t u8InChar = UART_READ(_CCTALK_PORT);
//				if(g_u32comRbytes < RXBUFSIZE)
//				{
//						g_u8RecData[g_u32comRbytes] = u8InChar;
//						g_u32comRbytes++;
//				}
//			//printf("#\r\n");
//			}
//			for(i=0;i<g_u32comRbytes;i++)
//			{
//				printf("%d ",g_u8RecData[i]);
////				if(g_u8RecData[i] == 242){
////					delay_1ms(300);
////					readEvent(ccTalk_ccTalk_ADDR_COIN);
////				}
//				g_u8RecData[i] = ' ';
//			}
//			g_u32comRbytes = 0;
//		}
////		for(;!SerialIsRxEmpty(_CCTALK_PORT) && i < 16;)
////		{
////			char c = SerialGetRx(_CCTALK_PORT);
////			if(!SerialIsRxError(_CCTALK_PORT)){
////				if(_slave == ccTalk_ccTalk_ADDR_COIN)
////					readBuff_Coin[i] = c;
////				if(_slave == ccTalk_ccTalk_ADDR_COIN)
////					readBuff_Bill[i] = c;
////				i++;
////			}
//			
////			//debug
////			sprintf(_str, "data = %d", c);
////			debug(_str);
////			//
//			//printf("%d " ,c);
//				
////			receiveTimeOut = _RECEIVE_DATA_TIME_OUT;
////			if(!displayState())
////				delay_1ms(_DELAY_TIME);
////			else{
////				for(p=0;p < 3;p++)
////					segmentDisplay();
////			}
//////			if(UART_IS_RX_FULL(_CCTALK_PORT)){
//////				debug("UART_IS_RX_FULL");
//////			}
////		}
////		if(!displayState())
////			delay_1ms(_DELAY_TIME);
////		else{
////			for(p=0;p < 3;p++)
////				segmentDisplay();
////		}
////		receiveTimeOut--;
////		if(receiveTimeOut == 0){
////			debug("!!Receive Event Data - Timeout!!\r\n");
////			break;
////		}
////	}
////	printf("\r\n");
////	if(receiveTimeOut == 0)
////		return 0;
//	return 1;
//}

bool HM_CCTALK::simplePoll(ccTalkAddr_t _slaveAddr)
{
	ccTalkPacket.SlaveAddr = _slaveAddr;
	ccTalkPacket.NumData = 0;
	ccTalkPacket.MasterAddr = ccTalk_ADDR_MASTER;
	ccTalkPacket.Cmd = ccTalk_CMD_SIMPLE_POLL;
	ccTalkPacket.CheckSum = addCheckSum(ccTalkPacket.SlaveAddr + ccTalkPacket.NumData + ccTalkPacket.MasterAddr + ccTalkPacket.Cmd);
	sendCcTalkSimplePacket();

	//wait to data received
	if (clrRepeatData(5))
	{
		if (receiveACKData()) {
			//delay_1ms(50);
			return 1;
		}
	}
	//delay_1ms(50);
	return 0;
}

bool HM_CCTALK::resetDevice(ccTalkAddr_t _slaveAddr)
{
	ccTalkPacket.SlaveAddr = _slaveAddr;
	ccTalkPacket.NumData = 0;
	ccTalkPacket.MasterAddr = ccTalk_ADDR_MASTER;
	ccTalkPacket.Cmd = ccTalk_CMD_RESET_MACHINE;
	ccTalkPacket.CheckSum = addCheckSum(ccTalkPacket.SlaveAddr + ccTalkPacket.NumData + ccTalkPacket.MasterAddr + ccTalkPacket.Cmd);
	sendCcTalkSimplePacket();

	//wait to data received
	if (clrRepeatData(5))
	{
		if (receiveACKData()) {
			//delay_1ms(50);
			return 1;
		}
	}
	//delay_1ms(50);
	return 0;
}

bool HM_CCTALK::selfCheck(ccTalkAddr_t _slaveAddr)
{
	ccTalkPacket.SlaveAddr = _slaveAddr;
	ccTalkPacket.NumData = 0;
	ccTalkPacket.MasterAddr = ccTalk_ADDR_MASTER;
	ccTalkPacket.Cmd = ccTalk_CMD_SELF_CHECK;
	ccTalkPacket.CheckSum = addCheckSum(ccTalkPacket.SlaveAddr + ccTalkPacket.NumData + ccTalkPacket.MasterAddr + ccTalkPacket.Cmd);
	sendCcTalkSimplePacket();

	//wait to data received
	if (clrRepeatData(5))
	{
		if (receiveFuaultData()) {
			//delay_1ms(50);
			return 1;
		}
	}
	//delay_1ms(50);
	return 0;
}

bool HM_CCTALK::setUnInhibit(ccTalkAddr_t _slaveAddr)
{
	debug("SetUnInhibit\r\n");
	ccTalkPacket.SlaveAddr = _slaveAddr;
	ccTalkPacket.NumData = 2;
	ccTalkPacket.MasterAddr = ccTalk_ADDR_MASTER;
	ccTalkPacket.Cmd = ccTalk_CMD_SET_UNINHIBIT;

	if (_slaveAddr == ccTalk_ADDR_COIN) {
		ccTalkPacket.Data[0] = 128;
		ccTalkPacket.Data[1] = 255;
	}
	else if (_slaveAddr == ccTalk_ADDR_BILL) {
		ccTalkPacket.Data[0] = 255;
		ccTalkPacket.Data[1] = 3;
	}

	ccTalkPacket.CheckSum = addCheckSum(ccTalkPacket.SlaveAddr + ccTalkPacket.NumData + ccTalkPacket.MasterAddr + ccTalkPacket.Cmd + ccTalkPacket.Data[0] + ccTalkPacket.Data[1]);
	sendCcTalkSimplePacket();

	//wait to data received
	if (clrRepeatData(7))
	{
		if (receiveACKData()) {
			//delay_1ms(50);
			return 1;
		}
	}
	//delay_1ms(50);
	return 0;
}

bool HM_CCTALK::setMaster(ccTalkAddr_t _slaveAddr, bool _value)
{
	if (_value)
		debug("SetMaster - ON\r\n");
	else
		debug("SetMaster - OFF\r\n");
	ccTalkPacket.SlaveAddr = _slaveAddr;
	ccTalkPacket.NumData = 1;
	ccTalkPacket.MasterAddr = ccTalk_ADDR_MASTER;
	ccTalkPacket.Cmd = ccTalk_CMD_SET_MASTER;

	if (_value)
		ccTalkPacket.Data[0] = 1;
	else
		ccTalkPacket.Data[0] = 0;

	ccTalkPacket.CheckSum = addCheckSum(ccTalkPacket.SlaveAddr + ccTalkPacket.NumData + ccTalkPacket.MasterAddr + ccTalkPacket.Cmd + ccTalkPacket.Data[0]);
	sendCcTalkSimplePacket();

	//wait to data received
	if (clrRepeatData(6))
	{
		if (receiveACKData()) {
			//delay_1ms(50);
			return 1;
		}
	}
	//delay_1ms(50);
	return 0;
}

bool HM_CCTALK::readEvent(ccTalkAddr_t _slaveAddr)
{
	ccTalkPacket.SlaveAddr = _slaveAddr;
	ccTalkPacket.NumData = 0;
	ccTalkPacket.MasterAddr = ccTalk_ADDR_MASTER;

	if (_slaveAddr == ccTalk_ADDR_COIN)
		ccTalkPacket.Cmd = ccTalk_CMD_READ_COIN_EVENT;
	else if (_slaveAddr == ccTalk_ADDR_BILL)
		ccTalkPacket.Cmd = ccTalk_CMD_READ_BILL_EVENT;

	ccTalkPacket.CheckSum = addCheckSum(ccTalkPacket.SlaveAddr + ccTalkPacket.NumData + ccTalkPacket.MasterAddr + ccTalkPacket.Cmd);
	sendCcTalkSimplePacket();

	flagClrRepeatData = 1;
	if (clrRepeatData(5))
	{
		if (receiveEventData(_slaveAddr)) {
			return 1;
		}
	}
	return 0;
}

//----------------------------------------------------//
//################## Coin Acceptor ###################//
//----------------------------------------------------//

int16_t HM_CCTALK::checkMicroSPCatCoinValue(uint8_t  cat_no)
{
	int16_t _coin_value = 0;
	switch (cat_no)
	{
	case 4:
		//logDebug->Print("coinAcc >> Received - 0.25BTH.");
		break;
	case 5:
		//logDebug->Print("coinAcc >> Received - 0.25BTH.");
		break;
	case 6:
		//logDebug->Print("coinAcc >> Received - 0.50BTH.");
		break;
	case 7:
		//logDebug->Print("coinAcc >> Received - 0.50BTH.");
		break;
	case 8:
		debug("Received - 1BTH.\r\n");
		_coin_value = 1;
		break;
	case 9:
		debug("Received - 1BTH.\r\n");
		_coin_value = 1;
		break;
	case 10:
		debug("Received - 2BTH.\r\n");
		_coin_value = 2;
		break;
	case 11:
		debug("Received - 2BTH.\r\n");
		_coin_value = 2;
		break;
	case 12:
		debug("Received - 2BTH.\r\n");
		_coin_value = 2;
		break;
	case 13:
		debug("Received - 5BTH.\r\n");
		_coin_value = 5;
		break;
	case 14:
		debug("Received - 5BTH.\r\n");
		_coin_value = 5;
		break;
	case 15:
		debug("Received - 10BTH.\r\n");
		_coin_value = 10;
		break;
	default:
		_coin_value = 0;
		//Serial1_print("coinAcc >> Channel not found\r\n");
	}
	return _coin_value;
}

int16_t HM_CCTALK::checkBuffCoin(void)
{
	int16_t _value = 0;
	//CoinCat = readBuff_Coin[5];
	if (readBuff_Coin[4] - eventCoin > 1)
	{
		uint8_t _diff_event = readBuff_Coin[4] - eventCoin;
		for (; _diff_event > 0; _diff_event--)
		{
			uint8_t _array = (_diff_event * 2) - 1;
			_array += 4;
			//CheckCoinValueBuff(readBuff_Coin1[_array]);
			_value += checkMicroSPCatCoinValue(readBuff_Coin[_array]);
		}
	}
	else if (readBuff_Coin[5] > 0)
	{
		//CoinCount = readBuff_Coin[4];
		if (eventCoin != readBuff_Coin[4])
		{
			_value = checkMicroSPCatCoinValue(readBuff_Coin[5]);
		}
	}
	else if (eventCoin == readBuff_Coin[4])
	{
		if (readBuff_Coin[6] == 14)
		{
			errorCoinCount++;
			if (errorCoinCount > 20) {
				errorCoinCount = 0;
				return -1;
			}
		}
		else
			errorCoinCount = 0;
	}
	eventCoin = readBuff_Coin[4];//keep Microcoin event
	return _value;
}

//----------------------------------------------------//
//################## Bill Acceptor ###################//
//----------------------------------------------------//

uint16_t HM_CCTALK::iTLBV20BillValueVerify(uint8_t _ch)
{
	uint16_t _value = 0;
	switch (_ch)
	{
	case 1:
		debug("billAcc >> Verify 20BTH.");
		_value = 20;
		break;
	case 2:
		debug("billAcc >> Verify 50BTH.");
		_value = 50;
		break;
	case 3:
		debug("billAcc >> Verify 100BAHT.");
		_value = 100;
		break;
	case 4:
		debug("billAcc >> Verify 500BAHT.");
		_value = 500;
		break;
	case 5:
		debug("billAcc >> Verify  500BAHT.");
		_value = 500;
		break;
	case 6:
		debug("billAcc >> Verify  1000BAHT.");
		_value = 1000;
		break;
	case 7:
		debug("billAcc >> Verify 1000BAHT.");
		_value = 1000;
		break;
	case 8:
		debug("billAcc >> Verify 50BAHT.");
		_value = 50;
		break;
	case 9:
		debug("billAcc >> Verify 20BAHT.");
		_value = 20;
		break;
	case 10:
		debug("billAcc >> Verify 500BAHT.");
		_value = 500;
		break;
	default:
		debug("billAcc >> Verify Ch. Not found\r\n");
		_value = 0;
	}
	return _value;
}

uint16_t HM_CCTALK::iTLBV20BillValueReceived(uint8_t _ch)
{
	uint16_t _value = 0;
	switch (_ch)
	{
	case 1:
		debug("billAcc >> Received - 20BAHT.");
		_value = 20;
		break;
	case 2:
		debug("billAcc >> Received - 50BAHT.");
		_value = 50;
		break;
	case 3:
		debug("billAcc >> Received - 100BAHT.");
		_value = 100;
		break;
	case 4:
		debug("billAcc >> Received - 500BAHT.");
		_value = 500;
		break;
	case 5:
		debug("billAcc >> Received - 500BAHT.");
		_value = 500;
		break;
	case 6:
		debug("billAcc >> Received - 1000BAHT.");
		_value = 1000;
		break;
	case 7:
		debug("billAcc >> Received - 1000BAHT.");
		_value = 1000;
		break;
	case 8:
		debug("billAcc >> Received - 50BAHT.");
		_value = 50;
		break;
	case 9:
		debug("billAcc >> Received - 20BAHT.");
		_value = 20;
		break;
	case 10:
		debug("billAcc >> Received - 500BAHT.");
		_value = 500;
		break;
	default:
		debug("billAcc >> Channel not found\r\n");
		_value = 0;
	}
	return _value;
}

uint8_t HM_CCTALK::checkBuffBill(void)
{
	int16_t _value = 0;
	if (readBuff_Bill[5] > 0)
	{
		if (eventBill != readBuff_Bill[4])
		{
			if ((uint8_t)readBuff_Bill[6] == 1) {
				_value = iTLBV20BillValueVerify((uint8_t)readBuff_Bill[5]);
				if (_value > 0) {
					bill_value = _value;
					flag_bill_verify = true;
					flag_bill_received = false;
				}
				//logDebug->println("BillVerify"));
			}
			else if ((uint8_t)readBuff_Bill[6] == 0) {
				_value = iTLBV20BillValueReceived((uint8_t)readBuff_Bill[5]);
				if (_value > 0) {
					bill_value = _value;
					flag_bill_received = true;
					flag_bill_verify = false;
				}
				//logDebug->println("BillAccepted"));
			}
		}
	}
	else if (eventBill == readBuff_Bill[4])
	{
		if (readBuff_Bill[6] == 14)
		{
			errorBillCount++;
			if (errorBillCount > 20) {
				errorBillCount = 0;
				return -1;
			}
		}
		else
			errorBillCount = 0;
	}
	eventBill = readBuff_Bill[4];
	return _value;
}

void HM_CCTALK::clrReadBuffer()
{
	if (serial_port->available()) {
		uint8_t c = serial_port->read();
	}
}

bool HM_CCTALK::billAvailable(void)
{
	return bill_verify_flag;
}

uint16_t HM_CCTALK::readBillAvailable(void)
{
	uint16_t value = 0;
	if (bill_verify_value)
		value = bill_verify_value;
	bill_verify_value = 0;
	bill_verify_flag = false;
	return value;
}

bool HM_CCTALK::billAccepted(void)
{
	return bill_accepted_flag;
}

uint16_t HM_CCTALK::readBillAccepted(void)
{
	uint16_t value = 0;
	if (bill_accepted_value)
		value = bill_accepted_value;
	bill_accepted_value = 0;
	bill_accepted_flag = false;
	return value;
}
