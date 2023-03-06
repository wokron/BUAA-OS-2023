#ifndef TESTMACHINE_FB_H
#define TESTMACHINE_FB_H

/*
 *  Definitions used by the framebuffer device in GXemul.
 *
 *  This file is in the public domain.
 */

/*  Physical base address for linear framebuffer memory:  */
#define DEV_FB_ADDRESS 0x12000000

/*  Physical base address for the framebuffer controller:  */
#define DEV_FBCTRL_ADDRESS 0x12f00000
#define DEV_FBCTRL_LENGTH 0x20

/*
 *  First choose the port by writing the port index to DEV_FBCTRL_PORT,
 *  then read or write DEV_FBCTRL_DATA.
 */

#define DEV_FBCTRL_PORT 0x00
#define DEV_FBCTRL_DATA 0x10

#define DEV_FBCTRL_PORT_COMMAND 0
#define DEV_FBCTRL_PORT_X1 1
#define DEV_FBCTRL_PORT_Y1 2
#define DEV_FBCTRL_PORT_X2 3
#define DEV_FBCTRL_PORT_Y2 4
#define DEV_FBCTRL_PORT_COLOR_R 5
#define DEV_FBCTRL_PORT_COLOR_G 6
#define DEV_FBCTRL_PORT_COLOR_B 7
#define DEV_FBCTRL_NPORTS 8

/*
 *  Controller commands:
 */

/*  Do nothing.  */
#define DEV_FBCTRL_COMMAND_NOP 0

/*  Set resolution to X1 x Y1.  */
#define DEV_FBCTRL_COMMAND_SET_RESOLUTION 1

/*  Get current resolution into X1, Y1.  */
#define DEV_FBCTRL_COMMAND_GET_RESOLUTION 2

/*  TODO:  */
#define DEV_FBCTRL_COMMAND_FILL 3
#define DEV_FBCTRL_COMMAND_COPY 4

#define DEV_FBCTRL_MAXY(x) (((DEV_FBCTRL_ADDRESS - DEV_FB_ADDRESS) / 3) / x)

#endif /*  TESTMACHINE_FB_H  */
