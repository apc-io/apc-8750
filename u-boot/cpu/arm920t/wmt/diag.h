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


#ifndef __DIAG_H__
#define __DIAG_H__

#if !defined(__TTYPE_H__)
#include "ttype.h"
#endif
#if !defined(__ADAPTER_H__)
#include "adapter.h"
#endif


/*---------------------  Export Definitions -------------------------*/
#define CMD_RBTST_TXRAM       0x01
#define CMD_RBTST_RXRAM       0x02
#define CMD_RBTST_SCANMODE    0x04
#define CMD_RBTST_SEQMODE     0x08
#define CMD_RBTST_PATMASK     0x70

typedef enum tagDIAG_TEST_ITEM {
    DIAG_TEST_MAC_REG_RW     = 1,
    DIAG_TEST_EEP_CONT_RW    = 2,
    DIAG_TEST_MII_REG_RW     = 3,
    DIAG_TEST_MII_FLOWREG_RW = 4,
    DIAG_TEST_IRQ_CONN       = 5,
    DIAG_TEST_INT_EXT_LBK    = 6,
    DIAG_TEST_RD_TD_HANDLE   = 7,
    DIAG_TEST_ADDR_MATCH_LOG = 8,
    DIAG_TEST_CABLE_LINK     = 9,
    DIAG_TEST_ITEM_MAX
} DIAG_TEST_ITEM;
/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/

#ifdef __cplusplus
extern "C" {                            /* Assume C declarations for C++ */
#endif /* __cplusplus */

BOOL GDIAGbLoopbackOnceByPoll(PSAdapterInfo pAdapter);

BOOL GDIAGbTestMacRegReadWrite(PSAdapterInfo pAdapter);
BOOL GDIAGbTestSromReadWrite(PSAdapterInfo pAdapter);
BOOL GDIAGbTestMiiRegReadWrite(PSAdapterInfo pAdapter);

BOOL GDIAGbTestMiiRegRWPAUSE(PSAdapterInfo pAdapter);

BOOL GDIAGbTestIrqConnect(PSAdapterInfo pAdapter);
BOOL GDIAGbTestRdTdHandling(PSAdapterInfo pAdapter);
BOOL GDIAGbTestAddrMatchLogic(PSAdapterInfo pAdapter);
BOOL GDIAGbTestCableLink(PSAdapterInfo pAdapter);

BOOL GDIAGbTestLoopbackMac(PSAdapterInfo pAdapter);
BOOL GDIAGbTestLoopbackExt(PSAdapterInfo pAdapter);
BOOL GDIAGbTestLoopbackExtGiga(PSAdapterInfo pAdapter);

BOOL GDIAGbTestPromiscuousMode(PSAdapterInfo pAdapter);
BOOL GDIAGbTestBroadcastMode(PSAdapterInfo pAdapter);
BOOL GDIAGbTestMulticastMode(PSAdapterInfo pAdapter);
BOOL GDIAGbTestDirectedMode(PSAdapterInfo pAdapter);
BOOL GDIAGbTestRuntPktMode(PSAdapterInfo pAdapter);
BOOL GDIAGbTestLongPktMode(PSAdapterInfo pAdapter);

BOOL GDIAGbRAMBISTTest(PSAdapterInfo pAdapter);

BOOL GDIAGbFIFOTest(PSAdapterInfo pAdapter);
VOID GDIAGvShowHWMibCounter(
    PSAdapterInfo pAdapter,
    PSHWMibCounter psHWMibCounter
    );

ULONG GDIAGuNetworkTest(DWORD dwTimes, UINT uPktSize);

#ifdef __cplusplus
}                                       /* End of extern "C" { */
#endif /* __cplusplus */

#endif /* __DIAG_H__ */
