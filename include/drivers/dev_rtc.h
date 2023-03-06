#ifndef TESTMACHINE_RTC_H
#define TESTMACHINE_RTC_H

/*
 *  Definitions used by the "rtc" device in GXemul.
 *
 *  This file is in the public domain.
 */

#define DEV_RTC_ADDRESS 0x15000000
#define DEV_RTC_LENGTH 0x00000200

#define DEV_RTC_TRIGGER_READ 0x0000
#define DEV_RTC_SEC 0x0010
#define DEV_RTC_USEC 0x0020

#define DEV_RTC_HZ 0x0100
#define DEV_RTC_INTERRUPT_ACK 0x0110

#endif /*  TESTMACHINE_RTC_H  */
