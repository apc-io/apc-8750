/*
 * (C) Copyright 2004
 * Jian Zhang, Texas Instruments, jzhang@ti.com.

 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>

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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* #define DEBUG */

#include <common.h>

#if defined(CFG_ENV_IS_IN_NAND) /* Environment is in Nand Flash */

#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <linux/mtd/nand.h>

#if ((CONFIG_COMMANDS&(CFG_CMD_ENV|CFG_CMD_NAND)) == (CFG_CMD_ENV|CFG_CMD_NAND))
#define CMD_SAVEENV
#endif

#if defined(CFG_ENV_SIZE_REDUND)
#error CFG_ENV_SIZE_REDUND  not supported yet
#endif

#if defined(CFG_ENV_ADDR_REDUND)
#error CFG_ENV_ADDR_REDUND and CFG_ENV_IS_IN_NAND not supported yet
#endif


#ifdef CONFIG_INFERNO
#error CONFIG_INFERNO not supported yet
#endif

/* references to names in cmd_nand.c */
#define NANDRW_READ		0x01
#define NANDRW_WRITE	0x00
#define NANDRW_JFFS2	0x02
#define NANDRW_JFFS2_SKIP	0x04
unsigned int env_blk_pos = 0; /*searched env block position*/
extern struct nand_chip nand_dev_desc[];
int nand_rw (struct nand_chip* nand, int cmd,
	    size_t start, size_t len,
	    size_t * retlen, u_char * buf);
int nand_erase(struct nand_chip* nand, size_t ofs,
				size_t len, int clean);
int check_block_table(struct nand_chip *nand, unsigned int scan);
/* references to names in env_common.c */
extern uchar default_environment[];
extern int default_environment_size;

char * env_name_spec = "NAND";


#ifdef ENV_IS_EMBEDDED
extern uchar environment[];
env_t *env_ptr = (env_t *)(&environment[0]);
#else /* ! ENV_IS_EMBEDDED */
env_t *env_ptr = 0;
#endif /* ENV_IS_EMBEDDED */


/* local functions */
static void use_default(void);

DECLARE_GLOBAL_DATA_PTR;

uchar env_get_char_spec (int index)
{
	return ( *((uchar *)(gd->env_addr + index)) );
}


/* this is called before nand_init()
 * so we can't read Nand to validate env data.
 * Mark it OK for now. env_relocate() in env_common.c
 * will call our relocate function which will does
 * the real validation.
 */
int env_init(void)
{
	gd->env_addr  = (ulong)&default_environment[0];
	gd->env_valid = 1;

	return (0);
}

#ifdef CMD_SAVEENV
int saveenv(void)
{
	int ret = 0;
	unsigned int total, i, index;
	
	NAND_ENABLE_CE(nand_dev_desc);
	ret = check_block_table(nand_dev_desc, 0);
	NAND_DISABLE_CE(nand_dev_desc);
	if (ret) {
		printf("bbt not found! Write ENV is rejected\n");
		return 1;
	}

	total = CFG_ENV_SIZE;
 	index = (env_blk_pos+1)%CFG_ENV_BLOCK_LENGTH;
 	printf ("Erasing next Nand block 0x%x\n", index);
 	for (i = 0; i < CFG_ENV_BLOCK_LENGTH; i++) {
		if (!nand_dev_desc[0].isbadblock((nand_dev_desc + 0),
		nand_dev_desc[0].erasesize * (CFG_ENV_BLOCK_OFFSET + index%CFG_ENV_BLOCK_LENGTH))) {
		 	if (nand_erase(nand_dev_desc + 0,
		 	nand_dev_desc[0].erasesize * (CFG_ENV_BLOCK_OFFSET + index%CFG_ENV_BLOCK_LENGTH),
		 	(CFG_ENV_SIZE/nand_dev_desc[0].erasesize) + 
		 	((CFG_ENV_SIZE%nand_dev_desc[0].erasesize) ? nand_dev_desc[0].erasesize : 0), 0)) {
		 		/* fixme add bad block into bbt */
		 		puts ("Erasing Nand next block fail..");
		 		return 1;
			} else {
				ret = 1;
				break;
		 	}
	 	}
	 	index++;
	}
	if (!ret)
		return 1;

	/*printf ("write nand env_ptr addr 0x%x\n", (unsigned int)env_ptr);*/
	ret = nand_rw(nand_dev_desc + 0,
				NANDRW_WRITE | NANDRW_JFFS2, nand_dev_desc[0].erasesize *
				(CFG_ENV_BLOCK_OFFSET + index%CFG_ENV_BLOCK_LENGTH), CFG_ENV_SIZE,
			      &total, (u_char *)env_ptr);
	//ret = WMTSaveImageToNAND(nand_dev_desc[0].erasesize * CFG_ENV_BLOCK_OFFSET,
	//(u_char*)env_ptr,	CFG_ENV_SIZE, 1);
  if (ret || total != CFG_ENV_SIZE) {
  	printf ("ret =0x%x total= 0x%x\n", (int) ret, total);
		return 1;
	}
	printf ("Erase original Nand env_blk_pos=0x%x", env_blk_pos);
	if (!nand_dev_desc[0].isbadblock((nand_dev_desc + 0),
		nand_dev_desc[0].erasesize * (CFG_ENV_BLOCK_OFFSET + env_blk_pos))) {
		 	if (nand_erase(nand_dev_desc + 0,
		 	nand_dev_desc[0].erasesize * (CFG_ENV_BLOCK_OFFSET + env_blk_pos),
		 	(CFG_ENV_SIZE/nand_dev_desc[0].erasesize) + 
		 	((CFG_ENV_SIZE%nand_dev_desc[0].erasesize) ? nand_dev_desc[0].erasesize : 0), 0)) {
		 		/* fixme add bad block into bbt */
		 		puts ("Erasing Nand original block fail..");
		 		return 1;
			} else {
				env_blk_pos++;
				puts (" ...done\n");
				return 0;
		 	}
	 	}

 	puts ("original nand env block is bad block\n");
  return 1;
}
#endif /* CMD_SAVEENV */


void env_relocate_spec (void)
{
#if !defined(ENV_IS_EMBEDDED)
	int ret = 0;
	unsigned int total, i = 0;
	total = 0;
	/*env_blk_pos = 0;*/
	/*WMTLoadImageFormNAND(nand_dev_desc + 0, 
	nand_dev_desc[0].erasesize * CFG_ENV_BLOCK_OFFSET,
	(u_char*)env_ptr, CFG_ENV_SIZE);*/
	NAND_ENABLE_CE(nand_dev_desc);
	ret = check_block_table(nand_dev_desc, 0);
	NAND_DISABLE_CE(nand_dev_desc);
	if (ret) {
		printf("bbt not found! Search ENV is rejected\n");
		return use_default();
	}
	for (i = 0; i < (CFG_ENV_BLOCK_LENGTH*2); i++) {
		/*printf("i = %d\n", i);*/
		if (nand_dev_desc[0].isbadblock((nand_dev_desc + 0),
		nand_dev_desc[0].erasesize * (CFG_ENV_BLOCK_OFFSET + i%CFG_ENV_BLOCK_LENGTH))) {
			printf(" search env block = 0x%x+1 is bad\n", env_blk_pos);
			continue;
	 	}
		/*printf("i1 = %d\n", i);*/
		ret = nand_rw(nand_dev_desc + 0, NANDRW_READ | NANDRW_JFFS2 | NANDRW_JFFS2_SKIP,
					nand_dev_desc[0].erasesize * (CFG_ENV_BLOCK_OFFSET + i%CFG_ENV_BLOCK_LENGTH),
					CFG_ENV_SIZE, &total, (u_char*)env_ptr);
		if (ret || (total != CFG_ENV_SIZE)) {
	  	printf ("ret =0x%x total= 0x%x\n", (int) ret, total);
	  	env_blk_pos = 0;
			return use_default();
		}
		if ((env_ptr->crc == 0xFFFFFFFF) && (i < CFG_ENV_BLOCK_LENGTH))
			continue;

		if (crc32(0, env_ptr->data, ENV_SIZE) != env_ptr->crc) {
			if (i < (CFG_ENV_BLOCK_LENGTH*2 - 1))
				continue;
			printf("could not search env nand block\n");
			env_blk_pos = 0;
			return use_default();
		} else {
			env_blk_pos = i%CFG_ENV_BLOCK_LENGTH;
			printf("env is read from nand block 0x%x+1 \n", env_blk_pos);
			return;
		}
	}
	return use_default();
#endif /* ! ENV_IS_EMBEDDED */

}

static void use_default()
{
	puts ("*** Warning - bad CRC or NAND, using default environment\n\n");

  	if (default_environment_size > CFG_ENV_SIZE){
		puts ("*** Error - default environment is too large\n\n");
		return;
	}

	memset (env_ptr, 0, sizeof(env_t));
	memcpy (env_ptr->data,
			default_environment,
			default_environment_size);
	env_ptr->crc = crc32(0, env_ptr->data, ENV_SIZE);
 	gd->env_valid = 1;

}

#endif /* CFG_ENV_IS_IN_NAND */
