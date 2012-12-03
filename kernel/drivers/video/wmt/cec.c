/*++ 
 * linux/drivers/video/wmt/cec.c
 * WonderMedia video post processor (VPP) driver
 *
 * Copyright c 2011  WonderMedia  Technologies, Inc.
 *
 * This program is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, either version 2 of the License, or 
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * WonderMedia Technologies, Inc.
 * 4F, 533, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C
--*/

/*
 * ChangeLog
 *
 * 2011-04-11  Sam Shen
 *     * Add License declaration and ChangeLog
 */

#define CEC_C
// #define DEBUG
/*----------------------- DEPENDENCE -----------------------------------------*/
#include "cec.h"

/*----------------------- PRIVATE MACRO --------------------------------------*/

/*----------------------- PRIVATE CONSTANTS ----------------------------------*/
/* #define CEC_XXXX    1     *//*Example*/

/*----------------------- PRIVATE TYPE  --------------------------------------*/
/* typedef  xxxx cec_xxx_t; *//*Example*/

/*----------EXPORTED PRIVATE VARIABLES are defined in cec.h  -------------*/
/*----------------------- INTERNAL PRIVATE VARIABLES - -----------------------*/
/* int  cec_xxx;        *//*Example*/
int cec_logical_addr;
int cec_physical_addr;

/*--------------------- INTERNAL PRIVATE FUNCTIONS ---------------------------*/
/* void cec_xxx(void); *//*Example*/
void wmt_cec_reg_dump(void);

/*----------------------- Function Body --------------------------------------*/
/*---------------------------- CEC COMMON API -------------------------------*/

#ifdef WMT_FTBLK_CEC
/*---------------------------- CEC HAL --------------------------------------*/
void wmt_cec_tx_data(char *buf,int size)
{
	int i;
	unsigned int addr,reg;
	int wait_idle;

#ifdef DEBUG
	DPRINT("[CEC] tx data(%d):",size);
	for(i=0;i<size;i++)
		DPRINT(" 0x%x",buf[i]);
	DPRINT("\n");
#endif

	if( size > 16 ){
		DPRINT("[CEC] *W* size max 16\n");
		return;
	}

	wait_idle = 0;
	while(vppif_reg32_read(CEC_WR_START)){	// wait idle
		if( wait_idle >= 10 ){
			DPRINT("[CEC] wait idle timeout\n");
			wmt_cec_reg_dump();
			return;
		}
		wait_idle++;
		mdelay(10);
		return;
	}
	
	
	addr = REG_CEC_ENCODE_HEADER;
	for(i=0;i<size;i++){
		reg = (buf[i] << 4) + 0x1;
		if( i == (size-1) )
			reg |= BIT1;
		
		vppif_reg32_out(addr,reg);
//		DPRINT("[CEC] wr 0x%x = 0x%x\n",addr,reg);
		addr+=4;
	}
	vppif_reg32_write(CEC_WR_NUM,(size * 10)+0x1);
	vppif_reg32_write(CEC_WR_START,1);
}

int wmt_cec_rx_data(char *buf)
{
	int i,size;
	unsigned int addr,reg;

	addr = REG_CEC_DECODE_HEADER;
	for(i=0;i<16;i++){
		reg = vppif_reg32_in(addr);
		buf[i] = (reg & 0xFF0) >> 4;
		if( reg & BIT1 ){	// EOM
			break;
		}
		addr+=4;
	}
	vppif_reg32_write(CEC_FINISH_RESET,1);
	mdelay(1);
	vppif_reg32_write(CEC_FINISH_RESET,0);
	size = i+1;
#ifdef DEBUG
	DPRINT("[CEC] rx data(%d):\n",size);
	for(i=0;i<size;i++)
		DPRINT(" 0x%x",buf[i]);
	DPRINT("\n");
#endif
	return size;
}

void wmt_cec_set_clock(void)
{
	#define CEC_CLOCK 1000000 / 6984
	
	vppif_reg32_out(REG_CEC_WR_START_SET0,370*CEC_CLOCK); // 3.7 ms
	vppif_reg32_out(REG_CEC_WR_START_SET1,450*CEC_CLOCK); // 4.5 ms
	vppif_reg32_out(REG_CEC_WR_LOGIC0_SET0,150*CEC_CLOCK); // 1.5 ms
	vppif_reg32_out(REG_CEC_WR_LOGIC0_SET1,240*CEC_CLOCK); // 2.4 ms
	vppif_reg32_out(REG_CEC_WR_LOGIC1_SET0,60*CEC_CLOCK); // 0.6 ms
	vppif_reg32_out(REG_CEC_WR_LOGIC1_SET1,240*CEC_CLOCK); // 2.4 ms
	vppif_reg32_out(REG_CEC_RD_START_L_SET0,350*CEC_CLOCK); // 3.5 ms
	vppif_reg32_out(REG_CEC_RD_START_R_SET0,390*CEC_CLOCK); // 3.9 ms
	vppif_reg32_out(REG_CEC_RD_START_L_SET1,430*CEC_CLOCK); // 4.3 ms
	vppif_reg32_out(REG_CEC_RD_START_R_SET1,470*CEC_CLOCK); // 4.7 ms
	vppif_reg32_out(REG_CEC_RD_LOGIC0_L_SET0,130*CEC_CLOCK); // 1.3 ms
	vppif_reg32_out(REG_CEC_RD_LOGIC0_R_SET0,170*CEC_CLOCK); // 1.7 ms
	vppif_reg32_out(REG_CEC_RD_LOGIC0_L_SET1,205*CEC_CLOCK); // 2.05 ms
	vppif_reg32_out(REG_CEC_RD_LOGIC0_R_SET1,275*CEC_CLOCK); // 2.75 ms
	vppif_reg32_out(REG_CEC_RD_LOGIC1_L_SET0,40*CEC_CLOCK); // 0.4 ms
	vppif_reg32_out(REG_CEC_RD_LOGIC1_R_SET0,80*CEC_CLOCK); // 0.8 ms
	vppif_reg32_out(REG_CEC_RD_LOGIC1_L_SET1,205*CEC_CLOCK); // 2.05 ms
	vppif_reg32_out(REG_CEC_RD_LOGIC1_R_SET1,275*CEC_CLOCK); // 2.75 ms
	vppif_reg32_out(REG_CEC_RD_L_SET0_ERROR,182*CEC_CLOCK); // 1.82 ms
	vppif_reg32_out(REG_CEC_RD_R_SET1_ERROR	,238*CEC_CLOCK); // 2.38 ms
	vppif_reg32_out(REG_CEC_RD_L_ERROR,287*CEC_CLOCK); // 2.87 ms
	vppif_reg32_out(REG_CEC_RX_SAMPLE_L_RANGE,85*CEC_CLOCK); // 0.85 ms
	vppif_reg32_out(REG_CEC_RX_SAMPLE_R_RANGE,125*CEC_CLOCK); // 1.25 ms
	vppif_reg32_out(REG_CEC_WR_SET0_ERROR,225*CEC_CLOCK); // 2.25 ms
	
}

void wmt_cec_set_logical_addr(int no,char addr,int enable)
{
	unsigned int mask;

	if( no > 5 ){
		DPRINT("[CEC] *W* max 5\n");
		return;
	}
	mask = 0xF << (4*no);
	vppif_reg32_write(REG_CEC_LOGICAL_ADDR,mask,4*no,addr);
	mask = BIT24 << no;
	vppif_reg32_write(REG_CEC_LOGICAL_ADDR,mask,24+no,enable);
	DBGMSG("[CEC] set logical addr %d,0x%x\n",no,addr);
}

void wmt_cec_rx_enable(int enable)
{
	vppif_reg32_write(CEC_REJECT_NEXT_DECODE,(enable)?0:1);
	vppif_reg32_write(GPIO_BASE_ADDR+0x40,BIT4,4,(enable)?0:1);	// GPIO4 disable GPIO function
}

void wmt_cec_enable_int(int no,int enable)
{
	vppif_reg32_write(REG_CEC_INT_ENABLE,0x1<<no,no,enable);
}

void wmt_cec_clr_int(int sts)
{
	vppif_reg32_out(REG_CEC_STATUS,sts);
}

int wmt_cec_get_int(void)
{
	int reg;

	reg = vppif_reg32_in(REG_CEC_STATUS);
	return reg;
}

void wmt_cec_enable_loopback(int enable)
{
	vppif_reg32_write(CEC_RD_ENCODE_ENABLE,enable);	// 1 : read self write and all dest data	
}

void wmt_cec_init_hw(void)
{
	vppif_reg32_write(GPIO_BASE_ADDR+0x40,BIT4,4,0);	// GPIO4 disable GPIO function
	vppif_reg32_write(GPIO_BASE_ADDR+0x80,BIT4,4,0);	// GPIO4 disable GPIO out
	vppif_reg32_write(GPIO_BASE_ADDR+0x480,BIT4,4,0);	// GPIO4 disable pull ctrl
	vppif_reg32_write(GPIO_BASE_ADDR+0x80,BIT23,23,1);	// Suspend GPIO output enable
	vppif_reg32_write(GPIO_BASE_ADDR+0xC0,BIT23,23,1);	// Suspend GPIO output high
	vppif_reg32_write(GPIO_BASE_ADDR+0x480,BIT19,19,0);	// Wake3 disable pull ctrl

	wmt_cec_set_clock();
	vppif_reg32_write(CEC_WR_RETRY,3);
	vppif_reg32_out(REG_CEC_RX_TRIG_RANGE,2);

	vppif_reg32_write(CEC_FREE_3X,3);
	vppif_reg32_write(CEC_FREE_5X,5);
	vppif_reg32_write(CEC_FREE_7X,7);

	vppif_reg32_write(CEC_COMP_DISABLE,1);
	vppif_reg32_write(CEC_ERR_HANDLE_DISABLE,0);
	vppif_reg32_write(CEC_NO_ACK_DISABLE,0);
	vppif_reg32_write(CEC_DECODE_FULL_DISABLE,0);
	vppif_reg32_write(CEC_STATUS4_START_DISABLE,1);
	vppif_reg32_write(CEC_STATUS4_LOGIC0_DISABLE,1);
	vppif_reg32_write(CEC_STATUS4_LOGIC1_DISABLE,1);
	vppif_reg32_write(CEC_RD_ENCODE_ENABLE,0);			// 1 : read self write and all dest data
}

/*---------------------------- CEC API --------------------------------------*/
void wmt_cec_reg_dump(void)
{
	DPRINT("========== CEC register dump ==========\n");
	vpp_reg_dump(REG_CEC_BEGIN,REG_CEC_END-REG_CEC_BEGIN);
	
	DPRINT("---------- CEC Tx ----------\n");
	DPRINT("wr start %d,wr num %d\n",vppif_reg32_read(CEC_WR_START),vppif_reg32_read(CEC_WR_NUM));
	DPRINT("wr header ack %d,EOM %d,data 0x%x\n",vppif_reg32_read(CEC_WR_HEADER_ACK),vppif_reg32_read(CEC_WR_HEADER_EOM),vppif_reg32_read(CEC_WR_HEADER_DATA));
	DPRINT("wr data ack %d,EOM %d,data 0x%x\n",vppif_reg32_read(CEC_WR_DATA_ACK),vppif_reg32_read(CEC_WR_DATA_EOM),vppif_reg32_read(CEC_WR_DATA));
	DPRINT("finish reset %d,wr retry %d\n",vppif_reg32_read(CEC_FINISH_RESET),vppif_reg32_read(CEC_WR_RETRY));

	DPRINT("---------- CEC Rx ----------\n");
	DPRINT("rd start %d,all ack %d,finish %d\n",vppif_reg32_read(CEC_RD_START),vppif_reg32_read(CEC_RD_ALL_ACK),vppif_reg32_read(CEC_RD_FINISH));
	DPRINT("rd header ack %d,EOM %d,data 0x%x\n",vppif_reg32_read(CEC_RD_HEADER_ACK),vppif_reg32_read(CEC_RD_HEADER_ACK),vppif_reg32_read(CEC_RD_HEADER_DATA));
	DPRINT("rd data ack %d,EOM %d,data 0x%x\n",vppif_reg32_read(CEC_RD_DATA_ACK),vppif_reg32_read(CEC_RD_DATA_EOM),vppif_reg32_read(CEC_RD_DATA));

	DPRINT("---------- Logical addr ----------\n");
	DPRINT("addr1 0x%x,valid %d\n",vppif_reg32_read(CEC_LOGICAL_ADDR1),vppif_reg32_read(CEC_ADDR_VALID1));
	DPRINT("addr2 0x%x,valid %d\n",vppif_reg32_read(CEC_LOGICAL_ADDR2),vppif_reg32_read(CEC_ADDR_VALID2));
	DPRINT("addr3 0x%x,valid %d\n",vppif_reg32_read(CEC_LOGICAL_ADDR3),vppif_reg32_read(CEC_ADDR_VALID3));
	DPRINT("addr4 0x%x,valid %d\n",vppif_reg32_read(CEC_LOGICAL_ADDR4),vppif_reg32_read(CEC_ADDR_VALID4));
	DPRINT("addr5 0x%x,valid %d\n",vppif_reg32_read(CEC_LOGICAL_ADDR5),vppif_reg32_read(CEC_ADDR_VALID5));

	DPRINT("---------- Misc ----------\n");
	DPRINT("free 3x %d,5x %d,7x %d\n",vppif_reg32_read(CEC_FREE_3X),vppif_reg32_read(CEC_FREE_5X),vppif_reg32_read(CEC_FREE_7X));
	DPRINT("reject next decode %d,comp disable %d\n",vppif_reg32_read(CEC_REJECT_NEXT_DECODE),vppif_reg32_read(CEC_COMP_DISABLE));
	DPRINT("err handle disable %d,no ack disable %d\n",vppif_reg32_read(CEC_ERR_HANDLE_DISABLE),vppif_reg32_read(CEC_NO_ACK_DISABLE));
	DPRINT("r1 enc ok %d,r1 dec ok %d,r1 err %d\n",vppif_reg32_read(CEC_R1_ENCODE_OK),vppif_reg32_read(CEC_R1_DECODE_OK),vppif_reg32_read(CEC_R1_ERROR));
	DPRINT("r1 arb fail %d,r1 no ack %d\n",vppif_reg32_read(CEC_R1_ARB_FAIL),vppif_reg32_read(CEC_R1_NO_ACK));
	DPRINT("dec full disable %d,self rd enable %d\n",vppif_reg32_read(CEC_DECODE_FULL_DISABLE),vppif_reg32_read(CEC_RD_ENCODE_ENABLE));
}

static unsigned int *wmt_cec_pm_bk;
void wmt_cec_do_suspend(void)
{
	vppif_reg32_write(GPIO_BASE_ADDR+0xC0,BIT23,23,0);	// Suspend GPIO output high
	wmt_cec_pm_bk = vpp_backup_reg(REG_CEC_BEGIN,(REG_CEC_END-REG_CEC_BEGIN));
}

void wmt_cec_do_resume(void)
{
	vppif_reg32_out(REG_VPP_SWRST2_SEL, 0x1011111);
	vppif_reg32_write(GPIO_BASE_ADDR+0x40,BIT4,4,0);	// GPIO4 disable GPIO function
	vppif_reg32_write(GPIO_BASE_ADDR+0x80,BIT4,4,0);	// GPIO4 disable GPIO out
	vppif_reg32_write(GPIO_BASE_ADDR+0x480,BIT4,4,0);	// GPIO4 disable pull ctrl
	vppif_reg32_write(GPIO_BASE_ADDR+0x80,BIT23,23,1);	// Suspend GPIO output enable
	vppif_reg32_write(GPIO_BASE_ADDR+0xC0,BIT23,23,1);	// Suspend GPIO output high
	vppif_reg32_write(GPIO_BASE_ADDR+0x480,BIT19,19,0);	// Wake3 disable pull ctrl
	vpp_restore_reg(REG_CEC_BEGIN,(REG_CEC_END-REG_CEC_BEGIN),wmt_cec_pm_bk);
	wmt_cec_pm_bk = 0;
}

#endif /* WMT_FTBLK_CEC */

