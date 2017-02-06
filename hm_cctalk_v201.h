// hm_cctalk_v104.h

#ifndef _HM_CCTALK_V104_h
#define _HM_CCTALK_V104_h

#include "sm_debug_v102.h"

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#define _DATA_MAX_SIZE 				8
#define _RECEIVE_DATA_TIME_OUT		200 // ms

#ifndef _DEBUG_SAY_ONLY
#define _DEBUG_SAY_ONLY 0
#endif //_DEBUG_SAY_ONLY

#ifndef _DEBUG_WRITE_ONLY //Set write to se card
#define _DEBUG_WRITE_ONLY 1
#endif //_DEBUG_WRITE_ONLY

#ifndef _DEBUG_SAY_AND_WRITE
#define _DEBUG_SAY_AND_WRITE 2
#endif //_DEBUG_PRINT_AND_WRITE

#define _CCTALK_DEBUG _DEBUG_SAY_ONLY
//#define _CCTALK_DEBUG _DEBUG_WRITE_ONLY
//#define _CCTALK_DEBUG _DEBUG_SAY_AND_WRITE

#ifndef _CCTALK_PORT
#define	_CCTALK_PORT		Serial1
#endif
#ifndef _CCTALK_BAUDRATE
#define	_CCTALK_BAUDRATE	9600
#endif

#define _DELAY_TIME			5 //Don't change

typedef enum
{
	ccTalk_ADDR_MASTER = 1,
	ccTalk_ADDR_COIN = 2,
	ccTalk_ADDR_BILL = 40
}ccTalkAddr_t;

typedef enum
{
	ccTalk_CMD_SIMPLE_POLL = 254,
	ccTalk_CMD_SELF_CHECK = 232,
	ccTalk_CMD_SET_UNINHIBIT = 231,
	ccTalk_CMD_READ_COIN_EVENT = 229,
	ccTalk_CMD_SET_MASTER = 228,
	ccTalk_CMD_READ_BILL_EVENT = 159,
	ccTalk_CMD_NAK_MESSAGE = 5,
	ccTalk_CMD_RESET_MACHINE = 1,
	ccTalk_CMD_RETURN_MESSAGE = 0
}ccTalkCMD_t;

typedef struct
{
	ccTalkAddr_t 		SlaveAddr;
	uint8_t    			NumData;
	ccTalkAddr_t		MasterAddr;
	ccTalkCMD_t			Cmd;
	uint8_t				Data[_DATA_MAX_SIZE];
	uint8_t				CheckSum;
}ccTalk_simple_packet_t;

class HM_CCTALK
{

private:

	SM_DEBUG *logDebug;
	HardwareSerial *serial_port;
	uint32_t serial_baud;
	uint32_t previous_time = 0;

	//######## Error Fault Table #########//
	// 0 = OK
	// 1 = Disconnect
	// 2 = jame
	// 3 = NOt OK
	//------------------------------------//

	ccTalk_simple_packet_t ccTalkPacket;

	/*
	const unsigned char simplePoll[5]		= {0x02,0x00,0x01,0xFE,0xFF};
	const unsigned char resetCoin[5]		= {0x02,0x00,0x01,0x01,0xFC};
	const unsigned char selfCheck[5]		= {0x02,0x00,0x01,0xE8,0x15};
	const unsigned char masterDisable[6]	= {0x02,0x01,0x01,0xE4,0x00,0x18};
	const unsigned char masterEnable[6]		= {0x02,0x01,0x01,0xE4,0x01,0x17};
	const unsigned char readBuffer[5]		= {0x02,0x00,0x01,0xE5,0x18};
	const unsigned char uninhibit8_16[7]	= {0x02,0x02,0x01,0xE7,0x80,0xFF,0x95};
	*/
	volatile unsigned char readACK[5];
	volatile unsigned char readFault[6];
	volatile unsigned char readBuff_Coin[16];
	volatile unsigned char readBuff_Bill[16];

	bool flag_bill_verify = false;
	bool flag_bill_received = false;
	bool flag_clr_repeat_data = 1;

	uint8_t event_coin = 0;
	uint8_t event_bill = 0;
	uint8_t error_coin_cnt = 0;
	uint8_t error_bill_cnt = 0;

	int8_t error_fault = 0;

	uint16_t bill_value = 0;

	void debug(String data);
	void timeoutReset(void);
	bool timeoutCheck(uint32_t _time);

public:
	HM_CCTALK();
	~HM_CCTALK();
	void init(HardwareSerial *_port);
	bool sendCcTalkSimplePacket(void);
	bool clrRepeatData(uint8_t _numData);
	bool checkAck(void);
	bool receiveACKData(void);
	bool checkFault(void);
	bool receiveEventData(ccTalkAddr_t _slaveAddr);

	bool simplePoll(ccTalkAddr_t _slaveAddr);
	bool resetDevice(ccTalkAddr_t _slaveAddr);
	bool selfCheck(ccTalkAddr_t _slaveAddr);
	bool setUnInhibit(ccTalkAddr_t _slaveAddr);
	bool setMaster(ccTalkAddr_t _slaveAddr, bool _value);
	bool readEvent(ccTalkAddr_t _slaveAddr);
	
	int8_t getFaultCode(void);
	int16_t checkBuffCoin(void);

	uint8_t addCheckSum(uint8_t raw);
	uint8_t receiveFuaultData(void);

	int16_t checkMicroSPCatCoinValue(uint8_t  cat_no);
	uint8_t checkBuffBill(void);

	void clrReadBuffer();

	uint16_t iTLBV20BillValueVerify(uint8_t  ch);
	uint16_t iTLBV20BillValueReceived(uint8_t _ch);
};

#endif //

