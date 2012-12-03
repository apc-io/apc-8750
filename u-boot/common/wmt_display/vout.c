/*++ 
 * linux/drivers/video/wmt/vout.c
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
 * 2010-08-05  Sam Shen
 *     * Add License declaration and ChangeLog
 */

#define VOUT_C
// #define DEBUG
/*----------------------- DEPENDENCE -----------------------------------------*/
#include "vout.h"

/*----------------------- PRIVATE MACRO --------------------------------------*/
/* #define  VO_XXXX  xxxx    *//*Example*/

/*----------------------- PRIVATE CONSTANTS ----------------------------------*/
/* #define VO_XXXX    1     *//*Example*/

/*----------------------- PRIVATE TYPE  --------------------------------------*/
/* typedef  xxxx vo_xxx_t; *//*Example*/

/*----------EXPORTED PRIVATE VARIABLES are defined in vout.h  -------------*/
/*----------------------- INTERNAL PRIVATE VARIABLES - -----------------------*/
/* int  vo_xxx;        *//*Example*/
vout_t *vout_array[VOUT_MODE_MAX];
vout_dev_ops_t *vout_dev_list;

/*--------------------- INTERNAL PRIVATE FUNCTIONS ---------------------------*/
/* void vo_xxx(void); *//*Example*/

/*----------------------- Function Body --------------------------------------*/
void vout_change_status(vout_t *vo,int cmd,int arg,int ret)
{

	DBGMSG(KERN_ALERT "vout_change_status(%d,%d,%d)\n",cmd,arg,ret);

	switch( cmd ){
		case VOCTL_INIT:
			vo->status |= VPP_VOUT_STS_REGISTER + VPP_VOUT_STS_ACTIVE;
			break;
		case VOCTL_UNINIT:
			vo->status = VPP_VOUT_STS_REGISTER;
			break;
		case VOCTL_VISIBLE:
			vo->status = (vo->status & ~VPP_VOUT_STS_BLANK) | ((arg)? 0:VPP_VOUT_STS_BLANK);
			break;
		case VOCTL_SUSPEND:
			vo->status |= VPP_VOUT_STS_POWERDN;
			break;
		case VOCTL_RESUME:
			vo->status &= ~VPP_VOUT_STS_POWERDN;
			break;
		case VOCTL_CHKPLUG:
			vo->status = (vo->status & ~VPP_VOUT_STS_PLUGIN) | ((ret)? VPP_VOUT_STS_PLUGIN:0);
			if( ret == 0 ){
				vo->status &= ~(VPP_VOUT_STS_EDID + VPP_VOUT_STS_CONTENT_PROTECT);
#ifdef CONFIG_VOUT_EDID_ALLOC
				if( vo->edid ){
					kfree(vo->edid);
					vo->edid = 0;
//					DPRINT("[VOUT] free edid buf\n");
				}
#endif
			}
			break;
		default:
			break;
	}
}

/*!*************************************************************************
* vout_op()
* 
* Private Function by Sam Shen, 2008/06/12
*/
/*!
* \brief	
*		
* \retval  0 if success
*/ 
int vout_op(vout_t *vo,int cmd,int arg)
{
	int *addr;
	int (*ioctl_proc)(int arg);
	int ret;
	
	DBGMSG(KERN_ALERT "vout_op(%d,0x%x)\n",cmd,arg);

	addr = (void *) vo->ops;
	addr += cmd;
	ioctl_proc = (void *) *addr;
	if( !ioctl_proc ){
		return 0;
	}
	ret = ioctl_proc(arg);
	vout_change_status(vo,cmd,arg,ret);
	return ret;
} /* End of vout_op */

/*!*************************************************************************
* vout_control()
* 
* Private Function by Sam Shen, 2008/06/12
*/
/*!
* \brief	
*		
* \retval  0 if success
*/ 
int vout_control(vout_mode_t mode,int cmd,int arg)
{
	vout_t *vo;
	int ret = 0;
	int i;

	if( mode > VOUT_MODE_MAX ){
		DPRINT(KERN_ALERT "*E* vout mode invalid %d\n",mode);
		return -1;
	}

	if( mode == VOUT_MODE_ALL ){
		for(i=0;i<VOUT_MODE_MAX;i++){
			if( !(vo = vout_array[i]) )
				continue;
				
			if( vo->status & VPP_VOUT_STS_ACTIVE ){
				vout_op(vo,cmd,arg);
			}
		}
	}
	else {
		if( (vo = vout_array[mode]) ){
			if( vo->status & VPP_VOUT_STS_ACTIVE ){
				ret = vout_op(vo,cmd,arg);
			}
		}
	}
	return ret;
} /* End of vout_control */

/*!*************************************************************************
* vout_enable()
* 
* Private Function by Sam Shen, 2008/06/12
*/
/*!
* \brief	
*		
* \retval  0 if success
*/ 
int vout_set_blank(vout_mode_t mode,int on)
{
	return vout_control(mode,VOCTL_VISIBLE,!on);
} /* End of vout_enable */

/*!*************************************************************************
* vout_set_mode()
* 
* Private Function by Sam Shen, 2008/06/12
*/
/*!
* \brief	
*		
* \retval  0 if success
*/ 
int vout_set_mode(vout_mode_t mode,int on)
{
	vout_t *vo;
	int i;

	DBGMSG(KERN_ALERT "vout_set_mode(%d,%d)\n",mode,on);

	if( mode > VOUT_MODE_MAX ){
		DPRINT(KERN_ALERT "*E* vout mode invalid %d\n",mode);
		return -1;
	}

	if( !(vo = vout_array[mode]) ){
		return -1;
	}

	if( on ){
		if( vo->status & VPP_VOUT_STS_ACTIVE ){
			return 0;
		}

		for(i=0;i<VOUT_MODE_MAX;i++){
			if( i == mode )
				continue;
			if( !(vo = vout_array[i]) )
				continue;
			if( vo->status & VPP_VOUT_STS_ACTIVE ){
				if( vout_op(vo,VOCTL_COMPATIBLE,mode) ){
					vout_op(vo,VOCTL_UNINIT,0);
				}
			}
		}
		vo = vout_array[mode];
		if( !(vo->status & VPP_VOUT_STS_ACTIVE) ){
			vout_op(vo,VOCTL_INIT,0);
		}
	}
	else {
		if( vo->status & VPP_VOUT_STS_ACTIVE ){
			vout_op(vo,VOCTL_UNINIT,0);
		}
	}
	return 0;
} /* End of vout_set_mode */

/*!*************************************************************************
* vout_config()
* 
* Private Function by Sam Shen, 2008/06/12
*/
/*!
* \brief	
*		
* \retval  0 if success
*/ 
int vout_config(vout_mode_t mode,vout_info_t *info)
{
	return vout_control(mode,VOCTL_CONFIG,(int)info); 
} /* End of vout_config */

/*!*************************************************************************
* vout_suspend()
* 
* Private Function by Sam Shen, 2008/06/12
*/
/*!
* \brief	
*		
* \retval  0 if success
*/ 
int vout_suspend(vout_mode_t mode,int level)
{
	return vout_control(mode,VOCTL_SUSPEND,level); 
} /* End of vout_suspend */

/*!*************************************************************************
* vout_resume()
* 
* Private Function by Sam Shen, 2008/06/12
*/
/*!
* \brief	
*		
* \retval  0 if success
*/ 
int vout_resume(vout_mode_t mode,int level)
{
	return vout_control(mode,VOCTL_RESUME,level); 
} /* End of vout_resume */

/*!*************************************************************************
* vout_chkplug()
* 
* Private Function by Sam Shen, 2009/09/24
*/
/*!
* \brief	
*		
* \retval  1 - plug in, 0 - plug out
*/ 
int vout_chkplug(vout_mode_t mode)
{
	vout_t *vo;
	int ret = 0;
	
	if( (vo = vout_array[mode]) ){
		ret = vout_op(vo,VOCTL_CHKPLUG,0);
	}
	return ret; 
} /* End of vout_resume */

/*!*************************************************************************
* vout_get_edid()
* 
* Private Function by Sam Shen, 2010/10/26
*/
/*!
* \brief	
*		
* \retval  edid buffer pointer
*/ 
char *vout_get_edid(vout_mode_t mode)
{
	vout_t *vo;

	if( !(vo = vout_array[mode]) ){
		return 0;
	}

	if( vo->status & VPP_VOUT_STS_EDID ){
		DPRINT("[VOUT] edid exist\n");
		return vo->edid;
	}

	vo->status &= ~VPP_VOUT_STS_EDID;
	if( vo->ops->get_edid ){
#ifdef CONFIG_VOUT_EDID_ALLOC
		if( vo->edid == 0 ){
//			DPRINT("[VOUT] edid buf alloc\n");
			vo->edid = (char *) kmalloc(128*EDID_BLOCK_MAX,GFP_KERNEL);
			if( !vo->edid ){
				DPRINT("[VOUT] *E* edid buf alloc\n");
				return 0;
			}
		}
#endif		
		if( vo->ops->get_edid((int)vo->edid) == 0 ){
			DPRINT("[VOUT] edid read\n");
			vo->status |= VPP_VOUT_STS_EDID;
			return vo->edid;
		}
	}

#ifdef CONFIG_VOUT_EDID_ALLOC
	if( vo->edid ){
		kfree(vo->edid);
		vo->edid = 0;
	}
#endif
	return 0;
} /* End of vout_get_edid */

/*!*************************************************************************
* vout_register()
* 
* Private Function by Sam Shen, 2008/06/12
*/
/*!
* \brief	
*		
* \retval  0 if success
*/ 
int vout_register(vout_mode_t mode,vout_t *vo)
{
	if( mode >= VOUT_MODE_MAX ){
		DPRINT(KERN_ALERT "*E* vout mode invalid %d\n",mode);
		return -1;
	}

	if( vout_array[mode] ){
		DPRINT(KERN_ALERT "*W* vout mode register again %d\n",mode);
	}
	
	vout_array[mode] = vo;
	vo->status = VPP_VOUT_STS_REGISTER;
	return 0;
} /* End of vout_register */

/*!*************************************************************************
* vout_unregister()
* 
* Private Function by Sam Shen, 2008/06/12
*/
/*!
* \brief	
*		
* \retval  0 if success
*/ 
int vout_unregister(vout_mode_t mode)
{
	if( mode >= VOUT_MODE_MAX ){
		printk(KERN_ALERT "*E* vout mode invalid %d\n",mode);
		return -1;
	}
	
	if( vout_array[mode] ){
		vout_array[mode] = 0;
	}
	return 0;
} /* End of vout_unregister */

/*!*************************************************************************
* vout_get_info()
* 
* Private Function by Sam Shen, 2008/06/12
*/
/*!
* \brief	
*		
* \retval  0 if success
*/ 
vout_t *vout_get_info(vout_mode_t mode)
{
	return vout_array[mode];
} /* End of vout_get_info */

/*!*************************************************************************
* vout_device_register()
* 
* Private Function by Sam Shen, 2010/05/27
*/
/*!
* \brief	
*		
* \retval  0 if success
*/ 
int vout_device_register(vout_dev_ops_t *ops)
{
	vout_dev_ops_t *list;

	if( vout_dev_list == 0 ){
		vout_dev_list = ops;
		list = ops;
	}
	else {
		list = vout_dev_list;
		while( list->next != 0 ){
			list = list->next;
		}
		list->next = ops;
	}
	ops->next = 0;
	return 0;
} /* End of vout_device_register */

/*!*************************************************************************
* vout_get_device()
* 
* Private Function by Sam Shen, 2010/05/27
*/
/*!
* \brief	
*		
* \retval  next device link
*/ 
vout_dev_ops_t *vout_get_device(vout_dev_ops_t *ops)
{
	if( ops == 0 ){
		return vout_dev_list;
	}
	return ops->next;
} /* End of vout_get_device */

/*!*************************************************************************
* vout_find_video_mode()
* 
* Private Function by Sam Shen, 2011/07/20
*/
/*!
* \brief	find match video mode
*		
* \retval  0-OK,1-fail
*/ 
int	vout_find_video_mode(vout_info_t *info)
{
	int flag = 1;
	vpp_timing_t *time;

//	DPRINT("[VPP] find video mode(%dx%d@%d,%d)\n",info->resx,info->resy,info->fps,info->timing.pixel_clock);

	if( info->timing.pixel_clock == (info->resx * info->resy * info->fps) ){
		flag = 0;
		time = &info->timing;
	}

	if( flag ){
		time = vpp_get_video_mode(info->resx,info->resy,info->timing.pixel_clock);
		if( time ){
			if( time->pixel_clock == info->timing.pixel_clock ){
				info->fps = VPP_GET_OPT_FPS(time->option);
				flag = 0;
			}
		}
	}

	if( flag ){
		time = vpp_get_video_mode(info->resx,info->resy,info->fps);
		if( time ){
			info->fps = VPP_GET_OPT_FPS(time->option);
			flag = 0;
		}
	}

	if( flag ){
		DPRINT("[VPP] *W* not find video mode\n");
	}
	else {
		info->timing = *time;
	}
//	DPRINT("[VPP] find video mode(%dx%d@%d,%d)\n",info->resx,info->resy,info->fps,info->timing.pixel_clock);
	return 0;
} /* End of vout_find_video_mode */

/*--------------------End of Function Body -----------------------------------*/
#undef VOUT_C

