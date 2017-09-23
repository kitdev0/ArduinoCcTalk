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
#include "ArduinoCcTalk.h"

ARDUINO_CCTALK::ARDUINO_CCTALK()
{
#ifdef _CCTALK_DEBUG
	logDebug = new ARDUINO_DEBUG(F("CCTALK"));
#endif // _CCTALK_DEBUG
}

ARDUINO_CCTALK::~ARDUINO_CCTALK()
{
#ifdef _CCTALK_DEBUG
	delete logDebug;
#endif // _CCTALK_DEBUG
	// delete serial_port;
}

// private:

void ARDUINO_CCTALK::debug(String data)
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

void ARDUINO_CCTALK::timeoutReset(void)
{
	previous_time = millis();
}

bool ARDUINO_CCTALK::timeoutCheck(uint32_t _time)
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
void ARDUINO_CCTALK::init(HardwareSerial *_port)
{
	serial_port = _port;
	serial_baud = _CCTALK_BAUDRATE;
	serial_port->begin(serial_baud, SERIAL_8N1);
	serial_port->flush();
}

uint8_t ARDUINO_CCTALK::addCheckSum(uint8_t raw)//Add Data Checksum
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

bool ARDUINO_CCTALK::sendCcTalkSimplePacket(void)
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
	return 0;
}

//void ARDUINO_CCTALK::cctalkReadDataToBuffer(void)
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

bool ARDUINO_CCTALK::clrRepeatData(uint8_t _numData, bool _db)
{
	String _str = "";
	timeoutReset();
	while (_numData)
	{
		if(serial_port->available())
		{
			uint8_t c = serial_port->read();
			if(_db)
				_str += String(c,DEC) + " ";
			_numData--;
			timeoutReset();
		}
		if (timeoutCheck(_RECEIVE_DATA_TIME_OUT)) {
			debug("!!Repeat Receive Data - Timeout!!");
			return 0;
		}
	}
	if(_db)
		debug("#clrRepeat >> " + _str);
	return 1;
}

bool ARDUINO_CCTALK::checkAck(void)
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

bool ARDUINO_CCTALK::receiveACKData(void)
{
	int i = 0;
	String _str = "";
	// int p = 0;
	// char _str[64];
//	debug("receiveACKData");
	timeoutReset();
	for (i = 0; i < 5;)
	{
		for (; serial_port->available() && i < 5;)
		{
			uint8_t c = serial_port->read();
			_str += String(c,DEC) + " ";
			readACK[i] = c;
			i++;
			// if(i == 5)
			// 	debug("#ACKData >> " + _str);//test
			timeoutReset();
		}
		if (timeoutCheck(_RECEIVE_DATA_TIME_OUT)) {
//			debug("!!Receive Data - Timeout!!\r\n");
			return 0;
		}
	}

	debug("#ACKData >> " + _str);//test

	if (i == 5) {
		if (checkAck())
			return 1;
	}
	return 0;
}

bool ARDUINO_CCTALK::checkFault(void)
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
		//		error_fault = 0;//OK
		//		break;
		//	case 3:
		//		debug("Self-check --> Coin jammed!!\r\n");
		//		debug("Pls, CheckDevice.\r\n");
		//		error_fault = 2;//jam
		//	default :
		//		debug("Self-check --> Not OK!!\r\n");
		//		debug("Pls, CheckDevice.\r\n");
		//		error_fault = 3;//Not OK
		//}
		//debug("Error fault = " + String(readFault[4]));
		error_fault = readFault[4];
		return 1;
	}
	else
	{
		debug("Self check --> Error!!\r\n");
		debug("Pls, CheckDevice.\r\n");
		error_fault = -1;//disconnect
		return 0;
	}
}

uint8_t ARDUINO_CCTALK::receiveFuaultData(void)
{
	int i = 0;
	String _str = "";
	// int p = 0;
	// uint8_t _str[64];
	timeoutReset();
	for (i = 0; i < 6;)
	{
		for (; serial_port->available() && i < 6;)
		{
			uint8_t c = serial_port->read();
			_str += String(c, DEC) + " ";
			readFault[i] = c;
			i++;
			timeoutReset();
		}
		if (timeoutCheck(_RECEIVE_DATA_TIME_OUT)) {
//			debug("!!Receive Data - Timeout!!\r\n");
			return 0;
		}
	}
	debug("receiveFuaultData >> " + _str);
	if (i == 6) {
		return checkFault();
	}
	return 0;
}

int8_t ARDUINO_CCTALK::getFaultCode(void)
{
	return error_fault;
}

bool ARDUINO_CCTALK::receiveEventData(ccTalkAddr_t _slave)
{
	int i = 0;
	String _str = "";
	// int p = 0;
	// uint8_t _str[64];
	timeoutReset();
	for (i = 0; i < 16;)
	{
		for (; serial_port->available() && i < 16;)
		{
			uint8_t c = serial_port->read();
			_str += String(c,DEC) + " ";

			// if(i == 15)
			// 	debug("#coin buff >> " + _str);

			// debug("# >> " + String(c, DEC));
			if (_slave == ccTalk_ADDR_COIN)
				readBuff_Coin[i] = c;
			if (_slave == ccTalk_ADDR_BILL)
				readBuff_Bill[i] = c;
			i++;
			timeoutReset();
		}
		if (timeoutCheck(_RECEIVE_DATA_TIME_OUT)) {
			debug("!!Receive Event Data - Timeout!!\r\n");
			// debug("## i = " + String(i) + "\r\n\r\n");
			return 0;
		}
	}
	return 1;
}

//bool ARDUINO_CCTALK::receiveEventData(ccTalkAddr_t _slave)
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

bool ARDUINO_CCTALK::simplePoll(ccTalkAddr_t _slaveAddr)
{
	ccTalkPacket.SlaveAddr = _slaveAddr;
	ccTalkPacket.NumData = 0;
	ccTalkPacket.MasterAddr = ccTalk_ADDR_MASTER;
	ccTalkPacket.Cmd = ccTalk_CMD_SIMPLE_POLL;
	ccTalkPacket.CheckSum = addCheckSum(ccTalkPacket.SlaveAddr + ccTalkPacket.NumData + ccTalkPacket.MasterAddr + ccTalkPacket.Cmd);
	sendCcTalkSimplePacket();

	//wait to data received
	if (clrRepeatData(5,true))
	{
		if (receiveACKData()) {
			//delay_1ms(50);
			return 1;
		}
	}
	//delay_1ms(50);
	return 0;
}

bool ARDUINO_CCTALK::resetDevice(ccTalkAddr_t _slaveAddr)
{
	debug("Reset Device...");
	ccTalkPacket.SlaveAddr = _slaveAddr;
	ccTalkPacket.NumData = 0;
	ccTalkPacket.MasterAddr = ccTalk_ADDR_MASTER;
	ccTalkPacket.Cmd = ccTalk_CMD_RESET_MACHINE;
	ccTalkPacket.CheckSum = addCheckSum(ccTalkPacket.SlaveAddr + ccTalkPacket.NumData + ccTalkPacket.MasterAddr + ccTalkPacket.Cmd);
	sendCcTalkSimplePacket();

	//wait to data received
	if (clrRepeatData(5,true))
	{
		if (receiveACKData()) {
			//delay_1ms(50);
			return 1;
		}
		else{
			debug("Not ACK respose");
		}
	}
	else{
		debug("Not data Repeat");
	}
	//delay_1ms(50);
	return 0;
}

bool ARDUINO_CCTALK::selfCheck(ccTalkAddr_t _slaveAddr)
{
	ccTalkPacket.SlaveAddr = _slaveAddr;
	ccTalkPacket.NumData = 0;
	ccTalkPacket.MasterAddr = ccTalk_ADDR_MASTER;
	ccTalkPacket.Cmd = ccTalk_CMD_SELF_CHECK;
	ccTalkPacket.CheckSum = addCheckSum(ccTalkPacket.SlaveAddr + ccTalkPacket.NumData + ccTalkPacket.MasterAddr + ccTalkPacket.Cmd);
	sendCcTalkSimplePacket();

	//wait to data received
	if (clrRepeatData(5,true))
	{
		if (receiveFuaultData()) {
			//delay_1ms(50);
			return 1;
		}
	}
	//delay_1ms(50);
	return 0;
}

bool ARDUINO_CCTALK::setUnInhibit(ccTalkAddr_t _slaveAddr)
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
	if (clrRepeatData(7,true))
	{
		if (receiveACKData()) {
			//delay_1ms(50);
			return 1;
		}
	}
	//delay_1ms(50);
	return 0;
}

bool ARDUINO_CCTALK::setMaster(ccTalkAddr_t _slaveAddr, bool _value)
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
	if (clrRepeatData(6,true))
	{
		if (receiveACKData()) {
			//delay_1ms(50);
			return 1;
		}
	}
	//delay_1ms(50);
	return 0;
}

bool ARDUINO_CCTALK::readEvent(ccTalkAddr_t _slaveAddr)
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

	flag_clr_repeat_data = 1;
	if (clrRepeatData(5,false))
	{
		if (receiveEventData(_slaveAddr)) {
			return 1;
		}
	}
	return 0;
}

bool ARDUINO_CCTALK::routeBill(ccTalkAddr_t _slaveAddr, bool _value)
{
	if (_value)
		debug("routeBill - Accept\r\n");
	else
		debug("routeBill - Reject\r\n");
	ccTalkPacket.SlaveAddr = _slaveAddr;
	ccTalkPacket.NumData = 1;
	ccTalkPacket.MasterAddr = ccTalk_ADDR_MASTER;
	ccTalkPacket.Cmd = ccTalk_CMD_ROUTE_BILL;

	if (_value)
		ccTalkPacket.Data[0] = 1;
	else
		ccTalkPacket.Data[0] = 0;

	ccTalkPacket.CheckSum = addCheckSum(ccTalkPacket.SlaveAddr + ccTalkPacket.NumData + ccTalkPacket.MasterAddr + ccTalkPacket.Cmd + ccTalkPacket.Data[0]);
	sendCcTalkSimplePacket();

	//wait to data received
	if (clrRepeatData(6,false))
	{
		if (receiveACKData()) {
			//delay_1ms(50);
			return 1;
		}
	}
	//delay_1ms(50);
	return 0;
}

//----------------------------------------------------//
//################## Coin Acceptor ###################//
//----------------------------------------------------//

int16_t ARDUINO_CCTALK::checkMicroSPCatCoinValue(uint8_t  cat_no)
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

int16_t ARDUINO_CCTALK::checkBuffCoin(void)
{
	int16_t _value = 0;
	//CoinCat = readBuff_Coin[5];
	if (readBuff_Coin[4] - event_coin > 1)
	{
		uint8_t _diff_event = readBuff_Coin[4] - event_coin;
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
		if (event_coin != readBuff_Coin[4])
		{
			_value = checkMicroSPCatCoinValue(readBuff_Coin[5]);
		}
	}
	else if (event_coin == readBuff_Coin[4])
	{
		if (readBuff_Coin[6] == 14)
		{
			error_coin_cnt++;
			if (error_coin_cnt > 20) {
				error_coin_cnt = 0;
				return -1;
			}
		}
		else
			error_coin_cnt = 0;
	}
	event_coin = readBuff_Coin[4];//keep Microcoin event
	return _value;
}

//----------------------------------------------------//
//################## Bill Acceptor ###################//
//----------------------------------------------------//

// uint16_t ARDUINO_CCTALK::iTLBV20BillVerifyValue(uint8_t _ch)
// {
// 	uint16_t _value = 0;
// 	switch (_ch)
// 	{
// 	case 1:
// 		debug("billAcc >> Verify 20BTH.");
// 		_value = 20;
// 		break;
// 	case 2:
// 		debug("billAcc >> Verify 50BTH.");
// 		_value = 50;
// 		break;
// 	case 3:
// 		debug("billAcc >> Verify 100BAHT.");
// 		_value = 100;
// 		break;
// 	case 4:
// 		debug("billAcc >> Verify 500BAHT.");
// 		_value = 500;
// 		break;
// 	case 5:
// 		debug("billAcc >> Verify  500BAHT.");
// 		_value = 500;
// 		break;
// 	case 6:
// 		debug("billAcc >> Verify  1000BAHT.");
// 		_value = 1000;
// 		break;
// 	case 7:
// 		debug("billAcc >> Verify 1000BAHT.");
// 		_value = 1000;
// 		break;
// 	case 8:
// 		debug("billAcc >> Verify 50BAHT.");
// 		_value = 50;
// 		break;
// 	case 9:
// 		debug("billAcc >> Verify 20BAHT.");
// 		_value = 20;
// 		break;
// 	case 10:
// 		debug("billAcc >> Verify 500BAHT.");
// 		_value = 500;
// 		break;
// 	default:
// 		debug("billAcc >> Verify Ch. Not found\r\n");
// 		_value = 0;
// 	}
// 	return _value;
// }

int16_t ARDUINO_CCTALK::checkBV20ChanelValue(uint8_t _ch,uint8_t _sorter)
{
	int16_t _value = 0;
	switch (_ch)
	{
	case 1:
		if(_sorter == 1)
			debug("billAcc >> Verify 20BTH.");
		else if(_sorter == 0)
			debug("billAcc >> Received - 20BAHT.");
		_value = 20;
		break;
	case 2:
		if(_sorter == 1)
			debug("billAcc >> Verify 50BTH.");
		else if(_sorter == 0)
			debug("billAcc >> Received - 50BAHT.");
		_value = 50;
		break;
	case 3:
		if(_sorter == 1)
			debug("billAcc >> Verify 100BTH.");
		else if(_sorter == 0)
			debug("billAcc >> Received - 100BAHT.");
		_value = 100;
		break;
	case 4:
		if(_sorter == 1)
			debug("billAcc >> Verify 500BTH.");
		else if(_sorter == 0)
			debug("billAcc >> Received - 500BAHT.");
		_value = 500;
		break;
	case 5:
		if(_sorter == 1)
			debug("billAcc >> Verify 1000BTH.");
		else if(_sorter == 0)
			debug("billAcc >> Received - 1000BAHT.");
		_value = 1000;
		break;
	case 6:
		if(_sorter == 1)
			debug("billAcc >> Verify 1000BTH.");
		else if(_sorter == 0)
			debug("billAcc >> Received - 1000BAHT.");
		_value = 1000;
		break;
	case 7:
		if(_sorter == 1)
			debug("billAcc >> Verify 1000BTH.");
		else if(_sorter == 0)
			debug("billAcc >> Received - 1000BAHT.");
		_value = 1000;
		break;
	case 8:
		if(_sorter == 1)
			debug("billAcc >> Verify 50BTH.");
		else if(_sorter == 0)
			debug("billAcc >> Received - 50BAHT.");
		_value = 50;
		break;
	case 9:
		if(_sorter == 1)
			debug("billAcc >> Verify 20BTH.");
		else if(_sorter == 0)
			debug("billAcc >> Received - 20BAHT.");
		_value = 20;
		break;
	case 10:
		if(_sorter == 1)
			debug("billAcc >> Verify 500BTH.");
		else if(_sorter == 0)
			debug("billAcc >> Received - 500BAHT.");
		_value = 500;
		break;
	default:
		debug("billAcc >> Channel not found\r\n");
		_value = 0;
	}
	return _value;
}

int16_t ARDUINO_CCTALK::checkBuffBill(void)
{
	int16_t _value = 0;
	if (readBuff_Bill[5] > 0)
	{
		if (event_bill != readBuff_Bill[4])
		{
			// if ((uint8_t)readBuff_Bill[6] == 1) {
			// 	_value = iTLBV20BillVerifyValue((uint8_t)readBuff_Bill[5]);
			// 	if (_value > 0) {
			// 		bill_verify_value = _value;
			// 		// flag_bill_verify = true;
			// 		// flag_bill_accepted = false;
			// 	}
			// 	//logDebug->println("BillVerify"));
			// }
			// else if ((uint8_t)readBuff_Bill[6] == 0) {
			// 	_value = iTLBV20BillAcceptedValue((uint8_t)readBuff_Bill[5]);
			// 	if (_value > 0) {
			// 		bill_accepted_value = _value;
			// 		// flag_bill_accepted = true;
			// 		// flag_bill_verify = false;
			// 	}
			// 	//logDebug->println("BillAccepted"));
			// }
			uint8_t _ch = (uint8_t)readBuff_Bill[5];
			uint8_t _sorter = (uint8_t)readBuff_Bill[6];
			bill_sorter = (int8_t)_sorter;
			_value = checkBV20ChanelValue(_ch, _sorter);
		}
	}
	else if (event_bill == readBuff_Bill[4])
	{
		if (readBuff_Bill[6] == 14)
		{
			error_bill_cnt++;
			if (error_bill_cnt > 20) {
				error_bill_cnt = 0;
				return -1;
			}
		}
		else
			error_bill_cnt = 0;
	}
	event_bill = readBuff_Bill[4];
	return _value;
}

void ARDUINO_CCTALK::clrReadBuffer()
{
	if (serial_port->available()) {
		serial_port->read();
	}
}

// uint16_t ARDUINO_CCTALK::readBillAvailable(void)
// {
// 	uint16_t value = 0;
// 	if (bill_verify_value)
// 		value = bill_verify_value;
// 	bill_verify_value = 0;
// 	flag_bill_verify = false;
// 	return value;
// }

// bool ARDUINO_CCTALK::billAccepted(void)
// {
// 	return flag_bill_accepted;
// }

// uint16_t ARDUINO_CCTALK::readBillAccepted(void)
// {
// 	uint16_t value = 0;
// 	if (bill_accepted_value)
// 		value = bill_accepted_value;
// 	bill_accepted_value = 0;
// 	flag_bill_accepted = false;
// 	return value;
// }
