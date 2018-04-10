/**
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2009-2018 ObdDiag.Net. All rights reserved.
 *
 */

#ifndef __ADAPTER_TYPES_H__ 
#define __ADAPTER_TYPES_H__

#include <cstdint>
#include <cstring>
#include <lstring.h>
#include <adapterdefs.h>

using namespace std;

// Config settings
const int KWP_HDR_LEN      = 5; // 4 header + 1 chksum
const int OBD_IN_MSG_DLEN  = 255;
const int OBD_OUT_MSG_DLEN = 255;                            // Binary len
const int OBD_OUT_MSG_LEN  = OBD_OUT_MSG_DLEN + KWP_HDR_LEN; // Binary buffer size
const int TX_BUFFER_LEN    = OBD_OUT_MSG_LEN * 3;            // Char buffer size

const int OBD_IN_MSG_LEN   = OBD_IN_MSG_DLEN + KWP_HDR_LEN;
const int RX_BUFFER_LEN    = OBD_IN_MSG_LEN * 3; 

//
// Command dispatch values
//
const int BYTE_PROPS_START  = 0;
const int INT_PROPS_START   = 100;
const int BYTES_PROPS_START = 1000;

enum AT_Requests {
    // bool properties
    PAR_ADPTV_TIM0 = BYTE_PROPS_START,
    PAR_ADPTV_TIM1,
    PAR_ADPTV_TIM2,
    PAR_ALLOW_LONG,
    PAR_AUTO_RECEIVE,
    PAR_BUFFER_DUMP,
    PAR_BYPASS_INIT,
    PAR_CALIBRATE_VOLT,
    PAR_CAN_CAF,
    PAR_CAN_DLC,
    PAR_CAN_FLOW_CONTROL,
    PAR_CAN_SEND_RTR,
    PAR_CAN_SHOW_STATUS,
    PAR_CAN_SILENT_MODE,
    PAR_CAN_TIMEOUT_MULT,
    PAR_CAN_VAIDATE_DLC,
    PAR_CHIP_COPYRIGHT,
    PAR_DESCRIBE_PROTCL_N,
    PAR_DESCRIBE_PROTOCOL,
    PAR_DUMMY,
    PAR_ECHO,
    PAR_FAST_INIT,
    PAR_FORGET_EVENTS,
    PAR_GET_SERIAL,
    PAR_HEADER_SHOW,
    PAR_INFO,
    PAR_INFRAME_RESPONSE,
    PAR_ISO_BAUDRATE,
    PAR_J1939_DM1_MONITOR,
    PAR_J1939_FMT,
    PAR_J1939_HEADER,
    PAR_J1939_MONITOR,
    PAR_KW_CHECK,
    PAR_KW_DISPLAY,
    PAR_LINEFEED,
    PAR_LOW_POWER_MODE,
    PAR_MEMORY,
    PAR_PROTOCOL_CLOSE,
    PAR_READ_VOLT,
    PAR_RESET_CPU,
    PAR_RESPONSES,
    PAR_SET_DEFAULT,
    PAR_SLOW_INIT,
    PAR_SPACES,
    PAR_STD_SEARCH_MODE,
    PAR_TRY_PROTOCOL,
    PAR_USE_AUTO_SP,
    PAR_VERSION,
    PAR_WARMSTART,
    PAR_WIRING_TEST,
    BYTE_PROPS_END,
    // int properties
    PAR_CAN_CFCPA = INT_PROPS_START,
    PAR_CAN_FLOW_CTRL_MD,
    PAR_CAN_SET_ADDRESS,
    PAR_CAN_TSTR_ADDRESS,
    PAR_ISO_INIT_ADDRESS,
    PAR_PROTOCOL,
    PAR_RECEIVE_ADDRESS,
    PAR_RECEIVE_FILTER,
    PAR_SET_BRD,
    PAR_TIMEOUT,
    PAR_TRY_BRD,
    PAR_VPW_SPEED,
    PAR_WAKEUP_VAL,
    INT_PROPS_END,
    // bytes properties
    PAR_CAN_EXT = BYTES_PROPS_START,
    PAR_CAN_FILTER,
	PAR_CAN_FLOW_CTRL_DAT,
    PAR_CAN_FLOW_CTRL_HDR,
    PAR_CAN_MASK,
    PAR_CAN_PRIORITY_BITS,
    PAR_HEADER_BYTES,
    PAR_TESTER_ADDRESS,
    PAR_USER_B,
    PAR_WM_HEADER,
    BYTES_PROPS_END
};

//
// union IntAggregate
//
union IntAggregate
{
    uint32_t lvalue;  
    uint8_t  bvalue[4];  
    IntAggregate() { lvalue = 0; }
    IntAggregate(uint32_t val) { lvalue = val; }
    IntAggregate(uint8_t b0, uint8_t b1, uint8_t b2 = 0, uint8_t b3 = 0) {
        bvalue[0] = b0; 
        bvalue[1] = b1; 
        bvalue[2] = b2; 
        bvalue[3] = b3; 
    }
};

//
// Byte array container for storing byte properties
//
struct ByteArray {
    const static int ARRAY_SIZE = 7;
    ByteArray()  {
        clear();
    }
    uint32_t asCanId() const {
        if (length == 4) {
            IntAggregate val(data[3], data[2], data[2], data[0]);
            return val.lvalue;
        }
        else if (length == 2) {
            IntAggregate val(data[1], data[0]);
            return val.lvalue;
        }
        return 0;
    }
    void clear() {
    	length = 0;
    	memset(data, 0, sizeof(data));
    }
    uint8_t data[ARRAY_SIZE];
    uint8_t length;
};

// Configuration settings
//
class AdapterConfig {
public:
    static AdapterConfig* instance();
    void     setBoolProperty(int parameter, bool val);
    bool     getBoolProperty(int parameter) const;
    void     setIntProperty(int parameter,  uint32_t val);
    uint32_t getIntProperty(int parameter) const;
    void     setBytesProperty(int parameter, const ByteArray* bytes);
    const    ByteArray* getBytesProperty(int parameter) const;
    void clear();
private:
    const static int INT_PROP_LEN   = INT_PROPS_END - INT_PROPS_START;
    const static int BYTES_PROP_LEN = BYTES_PROPS_END - BYTES_PROPS_START;

    AdapterConfig();
    uint64_t   values_; // 64 max
    uint32_t   intProps_  [INT_PROP_LEN];
    ByteArray  bytesProps_[BYTES_PROP_LEN];
};

//
// Helper class for inserting spaces, depends on ATS0/ATS1
//
class Spacer {
public:
    Spacer(util::string& str) : str_(str) {
        spaces_ = AdapterConfig::instance()->getBoolProperty(PAR_SPACES);
    }
    Spacer(util::string& str, bool spaces) : str_(str), spaces_(spaces) {}
    void space() { if (spaces_) str_ += ' '; }
    bool isSpaces() const { return spaces_; }
private:
    util::string& str_;
    bool spaces_;
};


void AdptSendString(const util::string& str);
void AdptSendReply(const char* str);
void AdptSendReply(const util::string& str);
void AdptSendReply(util::string& str);
void AdptDispatcherInit();
void AdptOnCmd(util::string& cmdString);
void AdptCheckHeartBeat();
void AdptReadSerialNum();
void AdptPowerModeConfigure();

// Utilities
void Delay1ms(uint32_t value);
void Delay1us(uint32_t value);
void KWordsToString(const uint8_t* kw, util::string& str);
void CanIDToString(uint32_t num, util::string& str, bool extended);
void CanIDToString(uint32_t num, util::string& str, bool extended, bool spaces);
void AutoReceiveParse(const util::string& str, uint32_t& filter, uint32_t& mask);
void ReverseBytes(uint8_t* bytes, uint32_t length);

uint32_t to_bytes(const util::string& str, uint8_t* bytes);
void to_ascii(const uint8_t* bytes, uint32_t length, util::string& str);

// LEDs
#define TX_LED(val) GPIOPinWrite(TX_LED_PORT, TX_LED_NUM, (~val) & 0x1)
#define RX_LED(val) GPIOPinWrite(RX_LED_PORT, RX_LED_NUM, (~val) & 0x1)

#endif //__ADAPTER_TYPES_H__
