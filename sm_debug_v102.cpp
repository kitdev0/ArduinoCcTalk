#include "sm_debug_v102.h"

SM_DEBUG::SM_DEBUG(String _head)
{
    serial_port = &_LOG_DEBUG_SERIAL_PORT;
    serial_baud = _LOG_DEBUG_SERIAL_BAUD;
    log_dir = (char *)_LOG_DIR;
    header = _head;
}

SM_DEBUG::~SM_DEBUG()
{
    delete serial_port;
}

//  private:
String SM_DEBUG::setFormat(String _data)
{
    String str;
    str = currentTime() + " " + header + " : " + _data;
    return str;
}

String SM_DEBUG::currentTime(void)
{
	time_t t = now();
	String str = to2Digits(hour(t));
	str += F(":");
	str += to2Digits(minute(t));
	str += F(":");
	str += to2Digits(second(t));
	return str;
}

String SM_DEBUG::currentDay(void)
{
	time_t t = now();
	String _str = F("");
	String _buf = F("");
	_str += to2Digits(day(t));
	_str += F("-");
	_str += to2Digits(month(t));
	_str += F("-");
	_str += year(t);
	//_str += _buf.substring(4);
	return _str;
}

String SM_DEBUG::to2Digits(int _digits)
{
	String str = F("");
	if (_digits < 10)
		str += F("0");
	str += String(_digits);
	return str;
}

String SM_DEBUG::to4Digits(int _digits)
{
	String str = F("");
	if (_digits < 10)
	{
		str += F("000");
	}
	else if (_digits < 100)
	{
		str += F("00");
	}
	else if (_digits < 1000)
	{
		str += F("000");
	}
	str += String(_digits);
	return str;
}

String SM_DEBUG::dirName(void)
{
	String _str = F("");
	_str += log_dir;
	_str += F("/");
	_str += currentDay();
	return _str;
}

String SM_DEBUG::fileName(void)
{
	String _str = F("");
	_str += dirName();
	_str += F("/");
	_str += lastFileName();
	return _str;
}

String SM_DEBUG::lastFileName(void)
{
#ifdef _SD_CARD_CONNECT

	File dir = SD.open(dirName());
	File entry = dir.openNextFile();
	String _file_name = F("");
	int _file_name_num = 1;
	if (entry)
	{
		while (entry)
		{
			if (!entry.isDirectory())
			{
				String _str = F("");
				uint16_t _int = 0;
				_str += entry.name();
				//sayln("#A = " + _str.substring(4, 8));
				_int = _str.substring(4, 8).toInt();
				if (_int > _file_name_num) {
					_file_name_num = _int;
				}
			}
			entry.close();
			entry = dir.openNextFile();
		}
	}
	entry.close();
	dir.close();
	_file_name = "file" + to4Digits(_file_name_num) + ".log";
	if (checkFileOverSize(_file_name)) {
		if (_file_name_num > _LOG_FILE_NUM_MAX)
			_file_name_num = 0;
		_file_name = "file" + to4Digits(_file_name_num + 1) + ".log";
	}
	//sayln("#B = " + _file_name);
	return _file_name;
#endif // _SD_CARD_CONNECT
	return "";
}

bool SM_DEBUG::checkFileOverSize(String _file_name)
{
#ifdef _SD_CARD_CONNECT
	char _file[32];
	char *_char_file_name;
	String _str_file_name = F("");
	_str_file_name += dirName();
	_str_file_name += F("/");
	_str_file_name += _file_name;

	_str_file_name.toCharArray(_file, _str_file_name.length() + 1);
	_char_file_name = _file;
	File _file_buf = SD.open(_char_file_name);
	if (_file_buf.size() > _LOG_SIZE_MAX) {
		return 1;
	}
#endif
	return 0;
}

bool SM_DEBUG::checkExistDir(void)
{
#ifdef _SD_CARD_CONNECT
	char _dir[16];
	char *_dir_name;
	String _str = dirName();
	_str.toCharArray(_dir, _str.length() + 1);
	_dir_name = _dir;

	if (sdCard.checkExistsFile(_dir_name)){
		//sayln("# nameDir exitsted"));
		return 1;
	}
	else{
		//sayln("# nameDir don't exitst"));
		sdCard.mkDir(_dir_name);
		return 0;
	}
#endif
	return 0;
}

//  public:
bool SM_DEBUG::init(void)
{
    serial_port->begin(serial_baud,SERIAL_8N1);
#ifdef _SD_CARD_CONNECT
#ifdef ROOT_PATH
	if (!sdCard.presentFlag())
		sdCard.init();

	if (sdCard.readyFlag())
		if(!sdCard.checkExistsFile(log_dir))
			sdCard.mkDir(log_dir);
#endif // ROOT_PATH
    sayln(F("# initial debug and log"));
#endif // _SD_CARD_CONNECT
    return 0;
}

void SM_DEBUG::setHeader(String _header)
{
    header = _header;
}

void SM_DEBUG::setLogPath(String _path)
{
	String str = F("Set path log file to : ");
	str += _path;
    sayln(str);

    log_path = _path;
}

void SM_DEBUG::setLogDir(char *_dir)
{
	String str = F("Set path log file to : ");
	str += String(_dir);
	sayln(str);

	log_dir = _dir;
}

void SM_DEBUG::say(String _data)
{
    String str;
    str = setFormat(_data);
    serial_port->print(str);
	serial_port->flush();
}

void SM_DEBUG::sayln(String _data)
{
    String str;
    str = setFormat(_data);
    serial_port->println(str);
	serial_port->flush();
}

void SM_DEBUG::writeLog(String _data)
{
#ifdef _SD_CARD_CONNECT
	checkExistDir();
	if (sdCard.readyFlag()) 
	{
		char _file[32];
		char *_file_name;
		String _str = "";
		_str += fileName();
		_str.toCharArray(_file, _str.length() + 1);
		_file_name = _file;
		sdCard.writeFile(_file_name, _data, 0);
		//if(sdCard.writeFile(_file_name,_data, 0))
			//sayln("write file - finished"));
		//else
			//sayln("can't write file"));
	}
	//char *_dir = "LOG/31_12_15";
	//sdCard.mkDir(_dir);
	//char *_file = "log/file1.log";
	//if (!sdCard.checkExistsFile(_file)) {
	//	sayln("file don't exists"));
	//	if (sdCard.mkFile(_file, log_dir)) {
	//		sayln("make file"));
	//		if(sdCard.writeFile(_file, _data))
	//			sayln("write file - finished"));
	//		else
	//			sayln("can't write file"));
	//	}
	//	else {
	//		sayln("can't make file"));
	//	}
	//}
	//else {
	//	if(sdCard.writeFile(_file, _data))
	//		sayln("write file - finished"));
	//}
#endif
}

void SM_DEBUG::sayAndWriteLog(String _data)
{
    String str;
    str = setFormat(_data);
	writeLog(str + "\n");
    serial_port->println(str);
	serial_port->flush();
}

void SM_DEBUG::check2DelLogOver(int _size_byte, String _file_name)
{

}
