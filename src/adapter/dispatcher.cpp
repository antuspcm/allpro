/**
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2009-2018 ObdDiag.Net. All rights reserved.
 *
 */

#include <climits>
#include <cstdio>
#include <adaptertypes.h>
#include <obd/j1979.h>
#include "obd/obdprofile.h"
#include <algorithms.h>
#include <CmdUart.h>
#include <AdcDriver.h>
#include <timeoutmgr.h>
#include <obd/isocan.h>

using namespace util;

//
// Reply string constants
//
static const char ErrMessage[] { "?" };
static const char OkMessage [] { "OK" };
static const char Version   [] { "1.15-ant1" };
static const char Interface [] { "ELM327 v2.1-ant1" };
static const char Copyright [] { "Copyright (c) 2009-2018 ObdDiag.Net, PcmHacking.net" };
static const char Copyright2[] { "This is free software; see the source for copying conditions. There is NO" };
static const char Copyright3[] { "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." };


/**
 * Store the boolean true property
 * @param[in] cmd Command line, ignored
 * @param[in[ par The number in dispatch table
 */
static void OnSetValueTrue(const string& cmd, int par)
{
    AdapterConfig::instance()->setBoolProperty(par, true);
    AdptSendReply(OkMessage);
}

/**
 * Store the boolean false property
 * @param[in[ cmd Command line, ignored
 * @param[in] par The number in dispatch table
 */
static void OnSetValueFalse(const string& cmd, int par)
{
    AdapterConfig::instance()->setBoolProperty(par, false);
    AdptSendReply(OkMessage);
}

/**
 * Store the int property
 * @param[in] cmd Command line
 * @param[in] par The number in dispatch table
 */
static void OnSetValueInt(const string& cmd, int par)
{
    uint32_t val = stoul(cmd, 0, 16);
    if (val != ULONG_MAX) {
        AdapterConfig::instance()->setIntProperty(par, val);
        AdptSendReply(OkMessage);
    }
    else {
        AdptSendReply(ErrMessage);
    }    
}

/**
 * Reset the byte sequence property to empty
 * @param[in] cmd Command line, ignored
 * @param[in] par The number in dispatch table
 */
static void OnResetBytes(const string& cmd, int par)
{
    ByteArray bytes;

    AdapterConfig::instance()->setBytesProperty(par, &bytes);
    AdptSendReply(OkMessage);
}

/**
 * Store the byte sequence property
 * @param[in] cmd Command line
 * @param[in] par The number in dispatch table
 */
static void OnSetBytes(const string& cmd, int par)
{
    string cmdData = cmd;
    ByteArray bytes;
    
    if (cmdData.length() == 3) {
        cmdData = "0" + cmdData; //1.5 bytes
    }
    bool sts = to_bytes(cmdData, bytes.data);
    
    if (sts) {
        bytes.length = cmdData.length() / 2;
        AdapterConfig::instance()->setBytesProperty(par, &bytes);
        AdptSendReply(OkMessage);
    }
    else {
        AdptSendReply(ErrMessage);
    }
}

/**
 * Reply with "OK" message
 * @param[in] cmd Command line, ignored
 * @param[in] par The number in dispatch table, ignored
 */
static void OnSetOK(const string& cmd, int par)
{
    AdptSendReply(OkMessage);
}

/**
 * Disable Adaptive Timing control
 * @param[in] cmd Command line, ignored
 * @param[in] par The number in dispatch table, ignored
 */
static void OnSetAT0(const string& cmd, int par)
{
    TimeoutManager::instance()->mode(TimeoutManager::AT0);
    AdptSendReply(OkMessage);
}

/**
 * Set Adaptive Timing control mode 1
 * @param[in] cmd Command line, ignored
 * @param[in] par The number in dispatch table, ignored
 */
static void OnSetAT1(const string& cmd, int par)
{
    TimeoutManager::instance()->mode(TimeoutManager::AT1);
    AdptSendReply(OkMessage);
}

/**
 * Set Adaptive Timing control mode 2
 * @param[in] cmd Command line, ignored
 * @param[in] par The number in dispatch table, ignored
 */
static void OnSetAT2(const string& cmd, int par)
{
    TimeoutManager::instance()->mode(TimeoutManager::AT2);
    AdptSendReply(OkMessage);
}

/**
 * Just a stub
 * @param[in] cmd Command line, ignored
 * @param[in] par The number in dispatch table, ignored
 */
static void OnCanShowStatus(const string& cmd, int par)
{
}

/**
 * Set CAN receive address
 * @param[in] cmd Command line
 * @param[in] par The number in dispatch table, ignored
 */
static void OnCanSetReceiveAddress(const string& cmd, int par)
{
    ByteArray bytes;
    
    AdapterConfig* config = AdapterConfig::instance();
    
    if (cmd.length() == 0) {
        config->setBytesProperty(PAR_CAN_FILTER, &bytes);
        config->setBytesProperty(PAR_CAN_MASK, &bytes);
    }
    else if (cmd.length() == 3 || cmd.length() == 8) {
        IntAggregate mask, filter;
        AutoReceiveParse(cmd, filter.lvalue, mask.lvalue);
        bytes.length = 2;
        memcpy(bytes.data, filter.bvalue, 4);
        config->setBytesProperty(PAR_CAN_FILTER, &bytes);
        memcpy(bytes.data, mask.bvalue, 4);
        config->setBytesProperty(PAR_CAN_MASK, &bytes);
    }
    else {
        AdptSendReply(ErrMessage);
        return;
    }
    
    OBDProfile::instance()->setFilterAndMask();
    AdptSendReply(OkMessage);
}

/**
 * Set CAN flow control mode [0..2]
 * @param[in] cmd Command line
 * @param[in] par The number in dispatch table
 */
static void OnCanSetFlowControlMode(const string& cmd, int par)
{
    AdapterConfig* config = AdapterConfig::instance();
    const ByteArray* hdr = config->getBytesProperty(PAR_CAN_FLOW_CTRL_HDR);
    const ByteArray* bytes = config->getBytesProperty(PAR_CAN_FLOW_CTRL_DAT);

    switch (stoul(cmd, 0, 16)) {
        case 0:
            config->setIntProperty(PAR_CAN_FLOW_CTRL_MD, 0);
            AdptSendReply(OkMessage);
            break;

        case 1:
            if (hdr->length && bytes->length) {
                config->setIntProperty(PAR_CAN_FLOW_CTRL_MD, 1);
                AdptSendReply(OkMessage);
            }
            else
                AdptSendReply(ErrMessage);
            break;

        case 2:
            if (bytes->length) {
                config->setIntProperty(PAR_CAN_FLOW_CTRL_MD, 2);
                AdptSendReply(OkMessage);
            }
            else
                AdptSendReply(ErrMessage);
            break;

        default:
            AdptSendReply(ErrMessage);
    }
}

/**
 * Set CAN filter/mask bytes, and refresh the driver if its CAN currenly
 * @param[in] cmd Command line
 * @param[in] par The number in dispatch table
 */
static void OnCanSetFilterAndMask(const string& cmd, int par)
{
    OnSetBytes(cmd, par);
    OBDProfile::instance()->setFilterAndMask();
}

/**
 * The adapter firmware string
 * @param[in] cmd Command line, ignored
 * @param[in] par The number in dispatch table, ignored
 */
static void OnSendReplyCopyright(const string& cmd, int par)
{
    AdptSendReply(Copyright);
    AdptSendReply(Copyright2);
    AdptSendReply(Copyright3);
}

/**
 * Run the adapter connectivity test
 * @param[in] cmd Command line, ignored
 * @param[in] par The number in dispatch table, ignored
 */
static void OnWiringTest(const string& cmd, int par)
{
    OBDProfile::instance()->wiringCheck();
}

/**
 * The serial #, hardware-related
 * @param[in] cmd Command line, ignored
 * @param[in] par The number in dispatch table, ignored
 */
static void OnGetSerialNum(const string& cmd, int par)
{
    AdptReadSerialNum();
}

/**
 * The firmware version
 * @param[in] cmd Command line, ignored
 * @param[in] par The number in dispatch table, ignored
 */
static void OnSendReplyVersion(const string& cmd, int par)
{
    AdptSendReply(Version);
}

/**
 * Dump the transmit/receive adapter buffer, "ATBD"
 * @param[in] cmd Command line, ignored
 * @param[in] par The number in dispatch table, ignored
 */
static void OnBufferDump(const string& cmd, int par)
{
    OBDProfile::instance()->dumpBuffer();    
}

/**
 * Get the current protocol description, "ATDP"
 * @param[in] cmd Command line, ignored
 * @param[in] par The number in dispatch table, ignored
 */
static void OnProtocolDescribe(const string& cmd, int par)
{    
    OBDProfile::instance()->getProtocolDescription(); 
}

/**
 * Get the current protocol number, "ATDPN"
 * @param[in] cmd Command line, ignored
 * @param[in] par The number in dispatch table, ignored
 */
static void OnProtocolDescribeNum(const string& cmd, int par)
{
    OBDProfile::instance()->getProtocolDescriptionNum(); 
}

/**
 * The adapter interface string, "ATI"
 * @param[in] cmd Command line, ignored
 * @param[in] par The number in dispatch table, ignored
 */
static void OnSendReplyInterface(const string& cmd, int par)
{
    AdptSendReply(Interface);
}

static void OnSetIsoBaudRate(const string& cmd, int par)
{
}

/**
 * ISO 9141/1423O keywords
 * @param[in] cmd Command line, ignored
 * @param[in] par The number in dispatch table, ignored
 */
static void OnKwDisplay(const string& cmd, int par)
{
    OBDProfile::instance()->kwDisplay();
}

/**
 * Execute J1939 DM1 monitor
 * @param[in] cmd Command line, ignored
 * @param[in] par The number in dispatch table, ignored
 */
static void OnJ1939MonitorDM1(const string& cmd, int par)
{
	OBDProfile::instance()->monitor();
}

/**
 * Execute J1939 MP monitor
 * @param[in] cmd Command line
 * @param[in] par The number in dispatch table
 */
static void OnJ1939MonitorMP(const string& cmd, int par)
{
	OBDProfile::instance()->monitor(cmd);
}

/**
 * Set the receive address to XX
 * @param[in] cmd Command line, ignored
 * @param[in] par The number in dispatch table, ignored
 */
static void OnSetRcvAddress(const string& cmd, int par)
{
    AdapterConfig::instance()->setBoolProperty(PAR_AUTO_RECEIVE, false);
    OnSetValueInt(cmd, par);
}

/**
 * Close the protocol, set the disconnected status, "ATPC"
 * @param[in] cmd Command line, ignored
 * @param[in] par The number in dispatch table, ignored
 */
static void OnProtocolClose(const string& cmd, int par)
{
    OBDProfile::instance()->closeProtocol();
    AdptSendReply(OkMessage);
}

/**
 * Read the car voltage level, "ATRV"
 * @param[in] cmd Command line, ignored
 * @param]in] par The number in dispatch table, ignored
 */
static void OnReadVoltage(const string& cmd, int par) 
{
    const uint32_t actualVoltage = 1212;
    const uint32_t adcDivdr = 0x0A54;
    
    uint32_t adcValue = AdcDriver::read();
    
    // Compute the voltage
    uint32_t val = (adcValue * actualVoltage / adcDivdr + 5);
    int valInt = val / 100;
    int valFraction = (val % 100) / 10;
    
    char out[10];
    sprintf(out, "%d.%1dV", valInt, valFraction);
    AdptSendReply(out);
}

/**
 * Set the protocol, "ATSP"
 * @param[in] cmd Command line
 * @param[in] par The number in dispatch table, ignored
 */
static void OnSetProtocol(const string& cmd, int par)
{
    bool useAutoSP = false;
    uint32_t protocol = 0;

    if (cmd[0] == 'A' && cmd.length() == 2 && cmd[1] != 'B') {
        protocol = stoul(cmd.substr(1, 2), 0, 16);
        useAutoSP = true;
    }
    else if (cmd.length() == 1) {
        protocol = stoul(cmd.substr(0, 2), 0, 16);
        useAutoSP = false;
    }
    else if (cmd == "00") {
        protocol = 0;
        useAutoSP = true;
    }
    else {
        AdptSendReply(ErrMessage);
        return;
    }
    
    AdapterConfig::instance()->setBoolProperty(PAR_USE_AUTO_SP, useAutoSP);
    if (OBDProfile::instance()->setProtocol(protocol, true) == REPLY_OK) {
        AdapterConfig::instance()->setIntProperty(PAR_PROTOCOL, protocol);
    AdptSendReply(OkMessage);
}
    else {
        AdapterConfig::instance()->setBoolProperty(PAR_USE_AUTO_SP, false);
        AdptSendReply(ErrMessage);
    }
}

/**
 * Set 4 header bytes for CAN 29, "ATSH ww xx yy zz"
 * @param[in] cmd Command line
 * @param[in] par The number in dispatch table
 */
static void OnSet4HeaderBytes(const string& cmd, int par)
{
    ByteArray cpBytes, hdrBytes;
    
    string cp = cmd.substr(0, 2);
    string hdr = cmd.substr(2);
    
    if (!to_bytes(cp, cpBytes.data) || !to_bytes(hdr, hdrBytes.data)) {
        AdptSendReply(ErrMessage);
        return;
    }
    cpBytes.length = cp.length() / 2;
    AdapterConfig::instance()->setBytesProperty(PAR_CAN_PRIORITY_BITS, &cpBytes);
    hdrBytes.length = hdr.length() / 2;
    AdapterConfig::instance()->setBytesProperty(PAR_HEADER_BYTES, &hdrBytes);
    AdptSendReply(OkMessage);
}

/**
 * Set CAN timeout multiplier, ATCTM1 or ATCTM5
 * @param[in] cmd Command line
 * @param[in] par The number in dispatch table
 */
static void OnCanSetTimeoutMult(const string& cmd, int par)
{
    bool val = false;
    if (cmd == "1") {
        val = false;
    }
    else if(cmd == "5") {
        val = true;
    }
    else {
        AdptSendReply(ErrMessage);
        return;
    }
    AdapterConfig::instance()->setBoolProperty(PAR_CAN_TIMEOUT_MULT, val);
    AdptSendReply(OkMessage);
}

/**
 * Set adapter default parameters
 */
static void SetDefault()
{
    AdapterConfig* config = AdapterConfig::instance();
    OBDProfile::instance()->setProtocol(PROT_AUTO, true);
    config->clear();
    config->setBoolProperty(PAR_HEADER_SHOW, false);
    config->setBoolProperty(PAR_LINEFEED, true);
    config->setBoolProperty(PAR_ECHO, true);
    config->setBoolProperty(PAR_SPACES, true);
    config->setBoolProperty(PAR_USE_AUTO_SP, true);
    config->setBoolProperty(PAR_KW_CHECK, true);
    config->setBoolProperty(PAR_CAN_DLC, false);
    config->setBoolProperty(PAR_CAN_FLOW_CONTROL, true);
    config->setBoolProperty(PAR_AUTO_RECEIVE, true);
    config->setBoolProperty(PAR_CAN_TIMEOUT_MULT, false);
    config->setBoolProperty(PAR_CAN_SILENT_MODE, true);
    config->setBoolProperty(PAR_J1939_HEADER, true);
    config->setIntProperty(PAR_ISO_INIT_ADDRESS, 0x33);
    config->setIntProperty(PAR_WAKEUP_VAL, (DEFAULT_WAKEUP_TIME / 20));
    config->setIntProperty(PAR_CAN_TSTR_ADDRESS, TESTER_ADDRESS);
    config->setIntProperty(PAR_CAN_TSTR_ADDRESS, 0xF1);
    config->setIntProperty(PAR_VPW_SPEED, 1);
}

/**
 * Reset to defaults, "ATD"
 * @param[in] cmd Command line, ignored
 * @param[in] par The number in dispatch table, ignored
 */
static void OnSetDefault(const string& cmd, int par)
{
    SetDefault();
    AdptSendReply(OkMessage);
}

/**
 * Set adapter parameters on reset, "ATZ"
 * @param[in] cmd Command line, ignored
 * @param[in] par The number in dispatch table, ignored
 */
static void OnReset(const string& cmd, int par) 
{
    SetDefault();
    ProtocolAdapter::clearHistory();
    AdptSendReply(Interface);
}

typedef void (*ParCallbackT)(const string& cmd, int par);

struct DispatchType {
    const char* name;
    int id;
    uint8_t minParNum;
    uint8_t maxParNum;
    ParCallbackT callback;
};

static const DispatchType dispatchTbl[] = {
    { "#1",     PAR_CHIP_COPYRIGHT,    0,  0, OnSendReplyCopyright   },
    { "#3",     PAR_WIRING_TEST,       0,  0, OnWiringTest           },
    { "#RSN",   PAR_GET_SERIAL,        0,  0, OnGetSerialNum         },
    { "@1",     PAR_VERSION,           0,  0, OnSendReplyVersion     },
    { "AL",     PAR_ALLOW_LONG,        0,  0, OnSetValueTrue         },
    { "AR",     PAR_AUTO_RECEIVE,      0,  0, OnSetValueTrue         },
    { "AT0",    PAR_ADPTV_TIM0,        0,  0, OnSetAT0               },
    { "AT1",    PAR_ADPTV_TIM1,        0,  0, OnSetAT1               },
    { "AT2",    PAR_ADPTV_TIM2,        0,  0, OnSetAT2               },
    { "BD",     PAR_BUFFER_DUMP,       0,  0, OnBufferDump           },
    { "AL",     PAR_ALLOW_LONG,        0,  0, OnSetOK                },
    { "BI",     PAR_BYPASS_INIT,       0,  0, OnSetValueTrue         },
    { "BRD",    PAR_TRY_BRD,           2,  2, OnSetValueInt          },
    { "BRT",    PAR_SET_BRD,           2,  2, OnSetValueInt          },
    { "CAF0",   PAR_CAN_CAF,           0,  0, OnSetValueFalse        },
    { "CAF1",   PAR_CAN_CAF,           0,  0, OnSetValueTrue         },
    { "CEA",    PAR_CAN_EXT,           0,  0, OnResetBytes           },
    { "CEA",    PAR_CAN_EXT,           2,  2, OnSetBytes             },
    { "CER",    PAR_CAN_TSTR_ADDRESS,  2,  2, OnSetValueInt          },
    { "CF",     PAR_CAN_FILTER,        3,  3, OnCanSetFilterAndMask  },
    { "CF",     PAR_CAN_FILTER,        8,  8, OnCanSetFilterAndMask  },
    { "CFC0",   PAR_CAN_FLOW_CONTROL,  0,  0, OnSetValueFalse        },
    { "CFC1",   PAR_CAN_FLOW_CONTROL,  0,  0, OnSetValueTrue         },
    { "CM",     PAR_CAN_MASK,          3,  3, OnCanSetFilterAndMask  },
    { "CM",     PAR_CAN_MASK,          8,  8, OnCanSetFilterAndMask  },
    { "CP",     PAR_CAN_PRIORITY_BITS, 2,  2, OnSetBytes             },
    { "CRA",    PAR_CAN_SET_ADDRESS,   0,  0, OnCanSetReceiveAddress },
    { "CRA",    PAR_CAN_SET_ADDRESS,   3,  3, OnCanSetReceiveAddress },
    { "CRA",    PAR_CAN_SET_ADDRESS,   8,  8, OnCanSetReceiveAddress },
    { "CS",     PAR_CAN_SHOW_STATUS,   0,  0, OnCanShowStatus        },
    { "CSM0",   PAR_CAN_SILENT_MODE,   0,  0, OnSetValueFalse        },
    { "CSM1",   PAR_CAN_SILENT_MODE,   0,  0, OnSetValueTrue         },
    { "CTM",    PAR_CAN_TIMEOUT_MULT,  1,  1, OnCanSetTimeoutMult    },
    { "CV",     PAR_CALIBRATE_VOLT,    4,  4, OnSetOK                },
    { "D",      PAR_SET_DEFAULT,       0,  0, OnSetDefault           },
    { "D0",     PAR_CAN_DLC,           0,  0, OnSetValueFalse        },
    { "D1",     PAR_CAN_DLC,           0,  0, OnSetValueTrue         },
    { "DM1",    PAR_J1939_DM1_MONITOR, 0,  0, OnJ1939MonitorDM1      },
    { "DP",     PAR_DESCRIBE_PROTOCOL, 0,  0, OnProtocolDescribe     },
    { "DPN",    PAR_DESCRIBE_PROTCL_N, 0,  0, OnProtocolDescribeNum  },
    { "E0",     PAR_ECHO,              0,  0, OnSetValueFalse        },
    { "E1",     PAR_ECHO,              0,  0, OnSetValueTrue         },
    { "FCSD",   PAR_CAN_FLOW_CTRL_DAT, 2, 10, OnSetBytes             },
    { "FCSH",   PAR_CAN_FLOW_CTRL_HDR, 3,  3, OnSetBytes             },
    { "FCSH",   PAR_CAN_FLOW_CTRL_HDR, 8,  8, OnSetBytes             },
    { "FCSM",   PAR_CAN_FLOW_CTRL_MD,  1,  1, OnCanSetFlowControlMode},
    { "FE",     PAR_FORGET_EVENTS,     0,  0, OnSetOK                },
    { "FI",     PAR_FAST_INIT,         0,  0, OnSetOK                },
    { "H0",     PAR_HEADER_SHOW,       0,  0, OnSetValueFalse        },
    { "H1",     PAR_HEADER_SHOW,       0,  0, OnSetValueTrue         },
    { "I",      PAR_INFO,              0,  0, OnSendReplyInterface   },
    { "IB",     PAR_ISO_BAUDRATE,      2,  2, OnSetIsoBaudRate       },
    { "IFR",    PAR_INFRAME_RESPONSE,  1,  1, OnSetOK                },
    { "IIA",    PAR_ISO_INIT_ADDRESS,  2,  2, OnSetValueInt          },
    { "JE",     PAR_J1939_FMT,         0,  0, OnSetValueFalse        },
    { "JHF0",   PAR_J1939_HEADER,      0,  0, OnSetValueFalse        },
    { "JHF1",   PAR_J1939_HEADER,      0,  0, OnSetValueTrue         },
    { "JS",     PAR_J1939_FMT,         0,  0, OnSetValueTrue         },
    { "JTM",    PAR_CAN_TIMEOUT_MULT,  1,  1, OnCanSetTimeoutMult    },
    { "KW",     PAR_KW_DISPLAY,        0,  0, OnKwDisplay            },
    { "KW0",    PAR_KW_CHECK,          0,  0, OnSetValueFalse        },
    { "KW1",    PAR_KW_CHECK,          0,  0, OnSetValueTrue         },
    { "L0",     PAR_LINEFEED,          0,  0, OnSetValueFalse        },
    { "L1",     PAR_LINEFEED,          0,  0, OnSetValueTrue         },
    { "LP",     PAR_LOW_POWER_MODE,    0,  0, OnSetOK                },
    { "M0",     PAR_MEMORY,            0,  0, OnSetValueFalse        },
    { "M1",     PAR_MEMORY,            0,  0, OnSetValueTrue         },
    { "MP",     PAR_J1939_MONITOR,     4,  7, OnJ1939MonitorMP       },
    { "NL",     PAR_ALLOW_LONG,        0,  0, OnSetValueTrue         },
    { "PB",     PAR_USER_B,            4,  4, OnSetBytes             },
    { "PC",     PAR_PROTOCOL_CLOSE,    0,  0, OnProtocolClose        },
    { "PPFFON", PAR_DUMMY,             0,  0, OnSetOK                },
    { "PPFFOFF",PAR_DUMMY,             0,  0, OnSetOK                },
    { "R0",     PAR_RESPONSES,         0,  0, OnSetValueFalse        },
    { "R1",     PAR_RESPONSES,         0,  0, OnSetValueTrue         },
    { "RA",     PAR_RECEIVE_ADDRESS,   2,  2, OnSetValueInt          },
    { "RTR",    PAR_CAN_SEND_RTR,      0,  0, OnSetOK                },
    { "RV",     PAR_READ_VOLT,         0,  0, OnReadVoltage          },
    { "S0",     PAR_SPACES,            0,  0, OnSetValueFalse        },
    { "S1",     PAR_SPACES,            0,  0, OnSetValueTrue         },
    { "SH",     PAR_HEADER_BYTES,      3,  3, OnSetBytes             },
    { "SH",     PAR_HEADER_BYTES,      6,  6, OnSetBytes             },
    { "SH",     PAR_HEADER_BYTES,      8,  8, OnSet4HeaderBytes      },
    { "SI",     PAR_SLOW_INIT,         0,  0, OnSetOK                },
    { "SP",     PAR_PROTOCOL,          1,  2, OnSetProtocol          },
    { "SR",     PAR_RECEIVE_FILTER,    2,  2, OnSetRcvAddress        },
    { "SS",     PAR_STD_SEARCH_MODE,   0,  0, OnSetValueTrue         },
    { "ST",     PAR_TIMEOUT,           2,  2, OnSetValueInt          },
    { "SW",     PAR_WAKEUP_VAL,        2,  2, OnSetValueInt          },
    { "TA",     PAR_TESTER_ADDRESS,    2,  2, OnSetValueInt          },
    { "TP",     PAR_TRY_PROTOCOL,      1,  1, OnSetProtocol          },
    { "TP",     PAR_TRY_PROTOCOL,      2,  2, OnSetProtocol          },
    { "V0",     PAR_CAN_VAIDATE_DLC,   0,  0, OnSetValueFalse        },
    { "V1",     PAR_CAN_VAIDATE_DLC,   0,  0, OnSetValueTrue         },
    { "VPW",    PAR_VPW_SPEED,         1,  1, OnSetValueInt          },
    { "WM",     PAR_WM_HEADER,         2, 12, OnSetBytes             },
    { "WS",     PAR_WARMSTART,         0,  0, OnReset                },
    { "Z",      PAR_RESET_CPU,         0,  0, OnReset                }
};

static bool ValidateArgLength(const DispatchType& entry, const string& arg)
{
    int len = arg.length();
    return len >= entry.minParNum && len <= entry.maxParNum;
}

/**
 * Dispatch the AT command to the proper handler with particular type
 * @param[in] cmdString Command line
 * @return true if command was dispatched, false otherwise
 */
static bool DispatchATCmd(const string& cmdString, int numOfChar, bool extraPar)
{
     // Ignore first two "AT" chars
    string atcmd = extraPar ? cmdString.substr(2, numOfChar) : cmdString.substr(2);

    for (const DispatchType& dt : dispatchTbl) {
        bool cmdType = dt.minParNum > 0;
        
        if ((cmdType == extraPar) && (atcmd == dt.name)) {
            string arg = extraPar ? cmdString.substr(numOfChar + 2) : "";

            // Argument length validation if we have an argument
            if (extraPar && !ValidateArgLength(dt, arg)) {
                continue;
            }
            
            // Have callback?
            if (!dt.callback)
                return false;
            
            dt.callback(arg, dt.id);
            return true;
        }
    }
    return false;
}

/**
 * Parse and dispatch AT sequence
 * @param[in] cmdString The user command 
 * @return true if command was parsed, false otherwise
 */
static bool ParseGenericATCmd(const string& cmdString)
{
    bool dispatched = DispatchATCmd(cmdString, 4, false); // Do exact string match, like "AT#DP"
    if (dispatched)
        return true;
    dispatched = DispatchATCmd(cmdString, 4, true); // Four char sequence prefixes
    if (dispatched)
        return true;
    dispatched = DispatchATCmd(cmdString, 3, true); // Three char sequence prefixes
    if (dispatched)
        return true;
    return DispatchATCmd(cmdString, 2, true); // Two char sequence prefixes
}

/**
 * Get the new command, do the processing. The "entry point" is here!
 * @param[in] cmdString The user command
 */
void AdptOnCmd(string& cmdString)
{
    static string PreviousCmd(RX_BUFFER_LEN);
    bool succeeded = false;
    
    // Compress and convert to uppercase 
    to_upper(cmdString);
    remove_space(cmdString);

    // Repeat the previous ?
    if (cmdString.empty()) {
        cmdString = PreviousCmd;
    }
    else {
        PreviousCmd = cmdString;
    }

    // Do we have AT sequence here?
    if (cmdString.substr(0,2) != "AT") { // Not AT sequence
        if (is_xdigits(cmdString)) {     // Should be only digits
            OBDProfile::instance()->onRequest(cmdString);
            succeeded = true;
        }
    }
    else { // AT sequence
        succeeded = ParseGenericATCmd(cmdString); // String cmd->numeric
    }
    
    if (!succeeded) {
        AdptSendReply(ErrMessage);
    }
    AdptSendString(">");
}

/**
 * Initialize buffers, flags and etc.
 */
void AdptDispatcherInit() 
{
    OnReset("", PAR_RESET_CPU);
    AdptSendString(">");
}

/**
 * Do heartbeat if required
 */
void AdptCheckHeartBeat()
{
    OBDProfile::instance()->sendHeartBeat();
}

/**
 * Send out string with <CR><LF>
 * @param[in] str String to send
 */
void AdptSendReply(const char* str)
{
	string s(strlen(str) + 2); // avoid unnecessary re-allocation, allocate the exact number of bytes
    s = str;
    AdptSendReply(s);
}

/**
 * Send out string with <CR><LF>
 * @param[in] str String to send
 */
void AdptSendReply(const string& str)
{
	string s(str.length() + 2); // avoid unnecessary re-allocation, allocate the exact number of bytes
    s = str;
    AdptSendReply(s); // use the next one
}

/**
 * Send out string with <CR><LF>, do not a allocate additional string
 * @param[in] str String to send
 */
void AdptSendReply(string& str)
{
    if (AdapterConfig::instance()->getBoolProperty(PAR_LINEFEED)) {
        str += "\r\n";
        AdptSendString(str);
    }
    else {
        str += "\r";
        AdptSendString(str);
    }
}
