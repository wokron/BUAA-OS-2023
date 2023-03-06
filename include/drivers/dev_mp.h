#ifndef TESTMACHINE_MP_H
#define TESTMACHINE_MP_H

/*
 *  Definitions used by the "mp" device in GXemul.
 *
 *  This file is in the public domain.
 */

/*
 *  Architecture-specific interrupt definitions:
 */

#define MIPS_IPI_INT 6

/*
 *  Default (physical) base address and length:
 */

#define DEV_MP_ADDRESS 0x11000000ULL
#define DEV_MP_LENGTH 0x00000100ULL

/*
 *  Offsets from the base address to reach the MP device' registers:
 */

#define DEV_MP_WHOAMI 0x0000
#define DEV_MP_NCPUS 0x0010
#define DEV_MP_STARTUPCPU 0x0020
#define DEV_MP_STARTUPADDR 0x0030
#define DEV_MP_PAUSE_ADDR 0x0040
#define DEV_MP_PAUSE_CPU 0x0050
#define DEV_MP_UNPAUSE_CPU 0x0060
#define DEV_MP_STARTUPSTACK 0x0070
#define DEV_MP_HARDWARE_RANDOM 0x0080
#define DEV_MP_MEMORY 0x0090
#define DEV_MP_IPI_ONE 0x00a0
#define DEV_MP_IPI_MANY 0x00b0
#define DEV_MP_IPI_READ 0x00c0
#define DEV_MP_NCYCLES 0x00d0

#endif /*  TESTMACHINE_MP_H  */
