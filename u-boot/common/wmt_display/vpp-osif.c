/*++ 
 * linux/drivers/video/wmt/osif.c
 * WonderMedia video post processor (VPP) driver
 *
 * Copyright c 2010  WonderMedia  Technologies, Inc.
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
 * 2010-10-18  Sam Shen
 *     * Add License declaration and ChangeLog
 */

#define VPP_OSIF_C
// #define DEBUG
/*----------------------- DEPENDENCE -----------------------------------------*/
#include "vpp-osif.h"
#include "./hw/wmt-vpp-hw.h"

/*----------------------- PRIVATE MACRO --------------------------------------*/

/*----------------------- PRIVATE CONSTANTS ----------------------------------*/
/* #define LVDS_XXXX    1     *//*Example*/

/*----------------------- PRIVATE TYPE  --------------------------------------*/
/* typedef  xxxx lvds_xxx_t; *//*Example*/

/*----------EXPORTED PRIVATE VARIABLES are defined in lvds.h  -------------*/
/*----------------------- INTERNAL PRIVATE VARIABLES - -----------------------*/
/* int  lvds_xxx;        *//*Example*/
#ifdef __KERNEL__
spinlock_t vpp_irqlock = SPIN_LOCK_UNLOCKED;
static unsigned long vpp_lock_flags;
#endif

/*--------------------- INTERNAL PRIVATE FUNCTIONS ---------------------------*/
/* void lvds_xxx(void); *//*Example*/

/*----------------------- Function Body --------------------------------------*/
#ifdef __KERNEL__
#include <asm/io.h>
#include <linux/proc_fs.h>
#else
__inline__ U32 inl(U32 offset)
{
	return REG32_VAL(offset);
}

__inline__ void outl(U32 val,U32 offset)
{
	REG32_VAL(offset) = val;
}

static __inline__ U16 inw(U32 offset)
{
	return REG16_VAL(offset);
}

static __inline__ void outw(U16 val,U32 offset)
{
	REG16_VAL(offset) = val;
}

static __inline__ U8 inb(U32 offset)
{
	return REG8_VAL(offset);
}

static __inline__ void outb(U8 val,U32 offset)
{
	REG8_VAL(offset) = val;
}
#ifndef CFG_LOADER
int get_key(void) 
{
	int key;

	extern int get_num(unsigned int min,unsigned int max,char *message,unsigned int retry);
	key = get_num(0, 256, "Input:", 5);
	DPRINT("\n");
	return key;
}

void udelay(int us)
{
	extern void vpp_post_delay(U32 tmr);

	vpp_post_delay(us);
}

void mdelay(int ms)
{
	udelay(ms*1000);
}
#endif
#endif

//Internal functions
U8 vppif_reg8_in(U32 offset)
{
	return (inb(offset));
}

U8 vppif_reg8_out(U32 offset, U8 val)
{
	outb(val, offset);
	return (val);
}

U16 vppif_reg16_in(U32 offset)
{
	return (inw(offset));
}

U16 vppif_reg16_out(U32 offset, U16 val)
{
	outw(val, offset);
	return (val);
}

U32 vppif_reg32_in(U32 offset)
{
	return (inl(offset));
}

U32 vppif_reg32_out(U32 offset, U32 val)
{
	outl(val, offset);
	return (val);
}

U32 vppif_reg32_write(U32 offset, U32 mask, U32 shift, U32 val)
{
	U32 new_val;

#ifdef VPPIF_DEBUG
	if( val > (mask >> shift) ){
		VPPIFMSG("*E* check the parameter 0x%x 0x%x 0x%x 0x%x\n",offset,mask,shift,val);
	}
#endif	

	new_val = (inl(offset) & ~(mask)) | (((val) << (shift)) & mask);
	outl(new_val, offset);
	return (new_val);
}

U32 vppif_reg32_read(U32 offset, U32 mask, U32 shift)
{
	return ((inl(offset) & mask) >> shift);
}

U32 vppif_reg32_mask(U32 offset, U32 mask, U32 shift)
{
	return (mask);
}

int vpp_request_irq(unsigned int irq_no,void *routine,unsigned int flags,char *name,void *arg)
{
#ifdef __KERNEL__
	if ( request_irq(irq_no,routine,flags,name,arg) ) {
		DPRINT("[VPP] *E* request irq %s fail\n",name);
		return -1;
	}
#endif
	return 0;
}

void vpp_free_irq(unsigned int irq_no,void *arg)
{
#ifdef __KERNEL__
	free_irq(irq_no,arg);
#endif
}

#ifndef __KERNEL__
int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen)
{
    int i = 0;
	unsigned char *p;

    p = getenv(varname);
    if (!p) {
        printf("## Warning: %s not defined\n",varname);
        return -1;
    } 
	while( p[i] != '\0' ){
		varval[i] = p[i];
		i++;
	}
	varval[i] = '\0';
	*varlen = i;
//    printf("getsyspara: %s,len %d\n", p, *varlen);
	return 0;
}
#endif

int vpp_parse_param(char *buf,unsigned int *param,int cnt)
{
	char *p;
    char * endp;
    int i = 0;

	if( *buf == '\0' )
		return 0;

	for(i=0;i<cnt;i++)
		param[i] = 0;

	p = buf;
	for(i=0;i<cnt;i++){
        param[i] = simple_strtoul(p, &endp, 10);
        if (*endp == '\0')
            break;
        p = endp + 1;

        if (*p == '\0')
            break;
    }
	return i+1;
}

void vpp_lock(void)
{
#ifdef __KERNEL__
	spin_lock_irqsave(&vpp_irqlock, vpp_lock_flags);
#else
#endif
}

void vpp_unlock(void)
{
#ifdef __KERNEL__
	spin_unlock_irqrestore(&vpp_irqlock, vpp_lock_flags);
#else
#endif
}

int vpp_i2c_lock;

int vpp_i2c_write(unsigned int addr,unsigned int index, unsigned char *pdata,int len)
{
#ifdef __KERNEL__
    struct i2c_msg msg[1];
	unsigned char buf[len+1];
	int ret;

//	vpp_lock();
	
	if( vpp_i2c_lock )
		DPRINT("*W* %s\n",__FUNCTION__);

	vpp_i2c_lock = 1;

	addr = (addr >> 1);
    buf[0] = index;
	memcpy(&buf[1],pdata,len);
    msg[0].addr = addr;
    msg[0].flags = 0 ;
    msg[0].flags &= ~(I2C_M_RD);
    msg[0].len = len+1;
    msg[0].buf = buf;
#ifdef CONFIG_I2C_WMT
    ret = wmt_i2c_xfer_continue_if_4(msg,1,VPP_I2C_ID);
#endif

#ifdef DEBUG
{
	int i;

	DBGMSG("vpp_i2c_write(addr 0x%x,index 0x%x,len %d\n",addr,index,len);	
	for(i=0;i<len;i+=8){
		DBGMSG("%d : 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",i,
			pdata[i],pdata[i+1],pdata[i+2],pdata[i+3],pdata[i+4],pdata[i+5],pdata[i+6],pdata[i+7]);
	}
}	
#endif
	vpp_i2c_lock = 0;
#else
	struct i2c_msg_s msgs[1] ;
	unsigned char p_data[8] ; 
	int ret;

	addr = addr >> 1;
	p_data[0] = (unsigned char)index ;
	memcpy(&p_data[1],pdata,len);
	msgs[0].addr  = addr ; 	/*slave address*/
	msgs[0].flags = I2C_M_WR ;
	msgs[0].len = len+1;
	msgs[0].buf   = p_data ;
	ret = i2c1_transfer(msgs, 1);
#endif
//	vpp_unlock();
	return ret;
} /* End of vpp_i2c_write */

int vpp_i2c_read(unsigned int addr,unsigned int index, unsigned char *pdata,int len) 
{
#ifdef __KERNEL__
	struct i2c_msg msg[2];
	unsigned char buf[len+1];
	int ret;

//	vpp_lock();
	if( vpp_i2c_lock )
		DPRINT("*W* %s\n",__FUNCTION__);

	vpp_i2c_lock = 1;

	addr = (addr >> 1);	
	memset(buf,0x55,len+1);
    buf[0] = index;
	buf[1] = 0x0;

    msg[0].addr = addr;
    msg[0].flags = 0 ;
	msg[0].flags &= ~(I2C_M_RD);
	msg[0].len = 1;
    msg[0].buf = buf;

	msg[1].addr = addr;
	msg[1].flags = 0 ;
	msg[1].flags |= (I2C_M_RD);
	msg[1].len = len;
	msg[1].buf = buf;

#ifdef CONFIG_I2C_WMT
	ret = wmt_i2c_xfer_continue_if_4(msg,2,VPP_I2C_ID);
#endif
	memcpy(pdata,buf,len);
#ifdef DEBUG
{
	int i;
	
	DBGMSG("vpp_i2c_read(addr 0x%x,index 0x%x,len %d\n",addr,index,len);
	for(i=0;i<len;i+=8){
		DBGMSG("%d : 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",i,
			pdata[i],pdata[i+1],pdata[i+2],pdata[i+3],pdata[i+4],pdata[i+5],pdata[i+6],pdata[i+7]);
	}
}
#endif
#else
	struct i2c_msg_s msgs[2] ;
	unsigned char p_data[2] ; 
	int ret;

	addr = addr >> 1;
	
	p_data[0] = (unsigned char)index ;
	msgs[0].addr  = addr ; /*slave address*/
	msgs[0].flags = I2C_M_WR ;
	msgs[0].len   = 1;/*how many bytes do you want to transfer*/
	msgs[0].buf   = p_data ;

	msgs[1].addr  = addr ;
	msgs[1].flags = I2C_M_RD ;
	msgs[1].len   = len ;
	msgs[1].buf   = pdata  ;
	ret = i2c1_transfer(msgs, 2);
#endif
	vpp_i2c_lock = 0;
//	vpp_unlock();
    return ret;
}

int vpp_i2c_enhanced_ddc_read(unsigned int addr,unsigned int index,char *pdata,int len) 
{
#ifdef __KERNEL__
	struct i2c_msg msg[3];
	unsigned char buf[len+1];
	unsigned char buf2[2];	

//	vpp_lock();

	if( vpp_i2c_lock )
		DPRINT("*W* %s\n",__FUNCTION__);

	vpp_i2c_lock = 1;

	buf2[0] = 0x1;
	buf2[1] = 0x0;
    msg[0].addr = (0x60 >> 1);
    msg[0].flags = 0 ;
	msg[0].flags &= ~(I2C_M_RD);
	msg[0].len = 1;
    msg[0].buf = buf2;

	addr = (addr >> 1);
	memset(buf,0x55,len+1);
    buf[0] = index;
	buf[1] = 0x0;

    msg[1].addr = addr;
    msg[1].flags = 0 ;
	msg[1].flags &= ~(I2C_M_RD);
	msg[1].len = 1;
    msg[1].buf = buf;

	msg[2].addr = addr;
	msg[2].flags = 0 ;
	msg[2].flags |= (I2C_M_RD);
	msg[2].len = len;
	msg[2].buf = buf;

#ifdef CONFIG_I2C_WMT
	wmt_i2c_xfer_continue_if(msg, 3);
#endif
	memcpy(pdata,buf,len);
	vpp_i2c_lock = 0;
//	vpp_unlock();
	return 0;
#else
	struct i2c_msg_s msg[3];
	unsigned char buf[len+1];
	unsigned char buf2[2];
	int ret;

	buf2[0] = 0x1;
	buf2[1] = 0x0;
    msg[0].addr = (0x60 >> 1);
    msg[0].flags = I2C_M_WR;
	msg[0].len = 1;
    msg[0].buf = buf2;

	addr = (addr >> 1);
	memset(buf,0x55,len+1);
    buf[0] = index;
	buf[1] = 0x0;

    msg[1].addr = addr;
    msg[1].flags = I2C_M_WR;
	msg[1].len = 1;
    msg[1].buf = buf;

	msg[2].addr = addr;
	msg[2].flags = I2C_M_RD;
	msg[2].len = len;
	msg[2].buf = buf;

	ret = i2c1_transfer(msg,3);
	memcpy(pdata,buf,len);
    return ret;
#endif
} /* End of vpp_i2c_enhanced_ddc_read */

void DelayMS(int ms)
{
	mdelay(ms);
}

#ifdef __KERNEL__
EXPORT_SYMBOL(vpp_i2c_write);
EXPORT_SYMBOL(vpp_i2c_read);
EXPORT_SYMBOL(vpp_i2c_enhanced_ddc_read);
EXPORT_SYMBOL(DelayMS);
#endif

