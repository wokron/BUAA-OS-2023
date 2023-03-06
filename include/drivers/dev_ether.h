#ifndef TESTMACHINE_ETHER_H
#define TESTMACHINE_ETHER_H

/*
 *  Definitions used by the "ether" device in GXemul.
 *
 *  This file is in the public domain.
 */

#define DEV_ETHER_ADDRESS 0x14000000
#define DEV_ETHER_LENGTH 0x8000

#define DEV_ETHER_BUFFER 0x0000
#define DEV_ETHER_BUFFER_SIZE 0x4000
#define DEV_ETHER_STATUS 0x4000
#define DEV_ETHER_PACKETLENGTH 0x4010
#define DEV_ETHER_COMMAND 0x4020
#define DEV_ETHER_MAC 0x4040

/*  Status bits:  */
#define DEV_ETHER_STATUS_PACKET_RECEIVED 1
#define DEV_ETHER_STATUS_MORE_PACKETS_AVAILABLE 2

/*  Commands:  */
#define DEV_ETHER_COMMAND_RX 0
#define DEV_ETHER_COMMAND_TX 1

#endif /*  TESTMACHINE_ETHER_H  */
