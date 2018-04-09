/**
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2009-2018 ObdDiag.Net. All rights reserved.
 *
 */

#ifndef __ADAPTER_DEFS_H__ 
#define __ADAPTER_DEFS_H__

#ifdef  __CC_ARM
  #define unique_ptr auto_ptr
#endif

#define TX_LED_ON_P012 // antus@pcmhacking.net has the older hardware

const int TX_LED_PORT =  0;
const int RX_LED_PORT =  0;
#ifdef TX_LED_ON_P012
  const int TX_LED_NUM  = 12;
#else
  const int TX_LED_NUM  = 13;
#endif
const int RX_LED_NUM  = 11;

#endif //__ADAPTER_DEFS_H__
