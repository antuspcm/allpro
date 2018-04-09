/**
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2009-2016 ObdDiag.Net. All rights reserved.
 *
 */

#ifndef __VPW_H__ 
#define __VPW_H__

#include "padapter.h"

class PwmDriver;
class Timer;

class VpwAdapter : public ProtocolAdapter {
public:
    friend class ProtocolAdapter;
    virtual int onRequest(const uint8_t* data, uint32_t len, uint32_t numOfResp);
    virtual void getDescription();
    virtual void getDescriptionNum();
    virtual void open();
    virtual void wiringCheck();
    virtual int onConnectEcu(bool sendReply);
    virtual int getProtocol() const { return PROT_J1850_VPW; }
private:
    VpwAdapter();
    bool waitForSof();
    int sendToEcu(const Ecumsg* msg);
    int receiveFromEcu(Ecumsg* msg, int maxLen);
    uint32_t getP2MaxTimeout() const;
    int requestImpl(const uint8_t* data, uint32_t len, uint32_t numOfResp, bool sendReply);
    Timer*     timer_;
    PwmDriver* driver_;

};

#endif //__VPW_H__
