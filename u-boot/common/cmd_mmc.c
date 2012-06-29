/*
 * (C) Copyright 2003
 * Kyle Harris, kharris@nexus-tech.net
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>


#if (CONFIG_COMMANDS & CFG_CMD_MMC)

#include <mmc.h>

static int sd_check_ctrlc(void)
{
    extern int ctrlc (void);
    if( ctrlc()){
        printf("Abort\n");
        return 1;
    }
    return 0;
}

int do_mmc_wait_insert(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    SD_Controller_Powerup();
	int insert = simple_strtoul(argv[1], NULL, 10);
	if(insert){
		printf("waiting insert SD card\n");
		while(SD_card_inserted()!=1){
			if( sd_check_ctrlc())
                    return -1;
		}
		return 0;

	}else{
		printf("wainting remove SD card\n");
		while(SD_card_inserted()!=0){
			if( sd_check_ctrlc())
                    return -1;
            udelay(500000);//delay 500ms
		}
		
		return 0;
	}
}

int do_mmc (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int device_num = 1;
	ulong dev_id = 0;
	
	if (argc <= 1) {
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
	
	if (argc == 2)
		dev_id = simple_strtoul(argv[1], NULL, 16);
	if (dev_id == 0 || dev_id == 1 || dev_id == 2)
		device_num = dev_id;

	if (mmc_init (1, (int)device_num) != 0) {
		printf ("No MMC card found\n");
		return 1;
	}
	return 0;
}

int do_mmc_read (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int device_num = 1;
	ulong dev_id = 0;
	ulong addr,block_num,bytes,blk_cnt;
	ulong ret;
		
	if (argc < 4) {
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
	
	/*get device id*/
	dev_id = simple_strtoul(argv[1], NULL, 16);
	if (dev_id == 0 || dev_id == 1 || dev_id == 2)
		device_num = dev_id;
	else {
		printf("dev_id Invalid\n");
		return 1;
	}

	/*get memory address*/
	addr = simple_strtoul (argv[2], NULL, 16);
	if (addr < 0) {
		printf("addr Invalid\n");
		return 1;
	}

	/*get card block address*/
	block_num = simple_strtoul (argv[3], NULL, 16);
	if (block_num < 0) {
		printf("block_num Invalid\n");
		return 1;
	}

	/*get transfer size is bytes*/
	bytes = simple_strtoul (argv[4], NULL, 16);
	if (bytes < 0) {
		printf("bytes Invalid\n");
		return 1;
	}

	if (bytes == 0)
		return 0;

	/*calculate transfer block count*/
	blk_cnt = (bytes / 512);
	if (bytes % 512)
		blk_cnt++;
	
	//printf("device_num = %x block_num = %x addr =  %x bytes = %x blk_cnt =%x\n",device_num,block_num,addr,bytes,blk_cnt);

	ret = mmc_bread(device_num,block_num,blk_cnt,(ulong *)addr);

	if (ret != blk_cnt) {
		printf("Read Data Fail\n");
		return 1;
	} else {
		printf("Read Data Success\n");
	}
		

	return 0;
}

int do_mmc_write (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int device_num = 1;
	ulong dev_id = 0;
	ulong addr,block_num,bytes,blk_cnt;
	ulong ret;
		
	if (argc < 4) {
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
	
	/*get device id*/
	dev_id = simple_strtoul(argv[1], NULL, 16);
	if (dev_id == 0 || dev_id == 1 || dev_id == 2)
		device_num = dev_id;
	else {
		printf("dev_id Invalid\n");
		return 1;
	}

	/*get memory address*/
	addr = simple_strtoul (argv[2], NULL, 16);
	if (addr < 0) {
		printf("addr Invalid\n");
		return 1;
	}

	/*get card block address*/
	block_num = simple_strtoul (argv[3], NULL, 16);
	if (block_num < 0) {
		printf("block_num Invalid\n");
		return 1;
	}

	/*get transfer size is bytes*/
	bytes = simple_strtoul (argv[4], NULL, 16);
	if (bytes < 0) {
		printf("bytes Invalid\n");
		return 1;
	}
	
	if (bytes == 0)
		return 0;

	/*calculate transfer block count*/
	blk_cnt = (bytes / 512);
	if (bytes % 512)
		blk_cnt++;
	
	//printf("device_num = %x block_num = %x addr =  %x bytes = %x blk_cnt =%x\n",device_num,block_num,addr,bytes,blk_cnt);
	ret = mmc_bwrite(device_num,block_num,blk_cnt,(ulong *)addr);

	if (ret != blk_cnt) {
		printf("Write Data Fail\n");
		return 1;
	} else {
		printf("Write Data Success\n");
	}
		

	return 0;
}

U_BOOT_CMD(
	mmcinit,	2,	1,	do_mmc,
	"mmcinit - init mmc card\n"
	"  mmcinit 0 -- init mmc device 0 \n"
	"  mmcinit 1 -- init mmc device 1 \n"
	"  mmcinit 2 -- init mmc device 2 \n",
	"mmcinit - init mmc card\n"
	"  mmcinit 0 -- init mmc device 0 \n"
	"  mmcinit 1 -- init mmc device 1 \n"
	"  mmcinit 2 -- init mmc device 2 \n"
);

U_BOOT_CMD(
	mmcread,	5,	1,	do_mmc_read,
	"mmcread - read data from SD/MMC card\n"
	"  <dev_id> <addr> <block_num> <bytes>\n"
	"   -read data from SD/MMC card block address 'block_num' on 'dev_id'\n"
	"    to memory address 'addr' size is 'bytes'\n",
	"mmcread - read data from SD/MMC card\n"
	"  <dev_id> <addr> <block_num> <bytes>\n"
	"   -read data from SD/MMC card block address 'block_num' on 'dev_id'\n"
	"    to memory address 'addr' size is 'bytes'\n"
);

U_BOOT_CMD(
	mmcwrite,	5,	1,	do_mmc_write,
	"mmcwrite - write data to SD/MMC card\n"
	"  <dev_id> <addr> <block_num> <bytes>\n"
	"   -write data to SD/MMC card block address 'block_num' on 'dev_id'\n"
	"    from memory address 'addr' size is 'bytes'\n",
	"mmcwrite - write data to SD/MMC card\n"
	"  <dev_id> <addr> <block_num> <bytes>\n"
	"   -write data to SD/MMC card block address 'block_num' on 'dev_id'\n"
	"    from memory address 'addr' size is 'bytes'\n"
);


U_BOOT_CMD(
	sdwaitins,	2,	1,	do_mmc_wait_insert,
	"sdwaitins - wait sd card inserted or removed\n"
	"sdwaitins 0 -- waiting removed\n"
	"sdwaitins 1 -- waiting inserted\n",
	"sdwaitins - wait sd card inserted or removed\n"
	"sdwaitins 0 -- waiting removed\n"
	"sdwaitins 1 -- waiting inserted\n"
);

#endif	/* CFG_CMD_MMC */
