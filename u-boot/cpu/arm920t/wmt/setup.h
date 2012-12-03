/*++ 
Copyright (c) 2010 WonderMedia Technologies, Inc.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software 
Foundation, either version 2 of the License, or (at your option) any later 
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details. You
should have received a copy of the GNU General Public License along with this
program. If not, see http://www.gnu.org/licenses/>.

WonderMedia Technologies, Inc.
10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

#ifndef __SETUP_H__
#define __SETUP_H__

#if !defined(__TTYPE_H__)
#include "ttype.h"
#endif
#if !defined(__ADAPTER_H__)
#include "adapter.h"
#endif

/*---------------------  Export Definitions -------------------------*/
#define DIAG_AUTO_NONE          0
#define DIAG_AUTO_DIAG          1
#define DIAG_AUTO_2CTEST        2
#define DIAG_AUTO_PINGPONG      3
#define DIAG_AUTO_PHYTEST       4

typedef struct CmdParaSet {

      /* command-line auto-function number, default = 0 (no auto-function), diagnositic test = 1, */
      /* network test = 2 */
      UINT uCmdFuncNum;
      /* function execution frequency, default = 0 (forever) */
      ULONG ulTestFreq;
      /* specify auto-test NIC number */
      UINT uCardNum;
      /* init NIC with forced mode, default = 0 (Auto mode), 1=10H, 2=10F, 3=100H, 4=100F */
      UINT uForceMode;
      /* shared IRQ support, default = TRUE */
      BOOL bSharedIRQ;

      /* 2-card network test with specified transmit packet size */
      /* 0  = increased packet size, 1 = Random packet size */
      /* 60 - 1514 = packet size */
      UINT uPktSize;

      /* test cable link status in diagnostic, default = TRUE */
      BOOL bTstCable;

      /* Connect to Altima-PHY slave RxNoReply exampt count, default = 0 */
      UINT uAltimaSlaveRxNoReply;

      /* Internal/external loopback test, default = FALSE */
      BOOL bExtLoopback;

      /* [1.34], Gigabit mode external loopback test, default = FALSE */
      BOOL bExtLoopbackGiga;

      /* RAMBIST test, default = TRUE */
      BOOL bRAMBISTTest;

      /* FIFO test, default = FALSE */
      BOOL bFIFOTest;

      /* IOL test flag, default = FALSE */
      BOOL bIOLTest;

      /* EEPROM checksum checking flag, default = FALSE */
      BOOL bEEPROMCheck;

      /* ForceMode loop flag, default = FALSE */
      BOOL bForceModeLoop;

      /* [1.33], Random data of packets for 2-card test, default = FALSE (normal content filling) */
      BOOL bRandomPktData;

      /* [1.33], Control transmission traffic for 2-card test, default = FALSE (normal load) */
      BOOL bHeavyLoad;

} SCmdParaSet, DEF * PSCmdParaSet;

/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/

#ifdef __cplusplus
extern "C" {                            /* Assume C declarations for C++ */
#endif /* __cplusplus */

void SETUPvSetupMenu(PSAdapterInfo pAdapter, SCmdParaSet sCmdPara);
ULONG SETUPuSetupSilence(PSAdapterInfo pAdapter[], SCmdParaSet sCmdPara);
void SETUPvGetStringForRealTimeMedia(PTSTR pszString, PSAdapterInfo pAdapter);

#ifdef __cplusplus
}                                       /* End of extern "C" { */
#endif /* __cplusplus */

#endif /* __SETUP_H__ */
