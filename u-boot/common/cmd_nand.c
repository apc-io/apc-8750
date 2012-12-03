/*
 * Driver for NAND support, Rick Bronson
 * borrowed heavily from:
 * (c) 1999 Machine Vision Holdings, Inc.
 * (c) 1999, 2000 David Woodhouse <dwmw2@infradead.org>
 *
 * Added 16-bit nand support
 * (C) 2004 Texas Instruments
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <asm/io.h>
#include <watchdog.h>
#include <nand.h>

#ifdef CONFIG_SHOW_BOOT_PROGRESS
# include <status_led.h>
# define SHOW_BOOT_PROGRESS(arg)	show_boot_progress(arg)
#else
# define SHOW_BOOT_PROGRESS(arg)
#endif

//#if (CONFIG_COMMANDS & CFG_CMD_NAND)

#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ids.h>
#include <jffs2/jffs2.h>

#ifdef CONFIG_OMAP1510
void archflashwp(void *archdata, int wp);
#endif

#define ROUND_DOWN(value,boundary)      ((value) & (~((boundary)-1)))

/*
 * Definition of the out of band configuration structure
 */
struct nand_oob_config {
	int ecc_pos[6];		/* position of ECC bytes inside oob */
	int badblock_pos;	/* position of bad block flag inside oob -1 = inactive */
	int eccvalid_pos;	/* position of ECC valid flag inside oob -1 = inactive */
} oob_config = { {0}, 0, 0};

#undef	NAND_DEBUG
#undef	PSYCHO_DEBUG

/* ****************** WARNING *********************
 * When ALLOW_ERASE_BAD_DEBUG is non-zero the erase command will
 * erase (or at least attempt to erase) blocks that are marked
 * bad. This can be very handy if you are _sure_ that the block
 * is OK, say because you marked a good block bad to test bad
 * block handling and you are done testing, or if you have
 * accidentally marked blocks bad.
 *
 * Erasing factory marked bad blocks is a _bad_ idea. If the
 * erase succeeds there is no reliable way to find them again,
 * and attempting to program or erase bad blocks can affect
 * the data in _other_ (good) blocks.
 */
#define	 ALLOW_ERASE_BAD_DEBUG 0

#define CONFIG_MTD_NAND_ECC  /* enable ECC */
#define CONFIG_MTD_NAND_ECC_JFFS2

/* bits for nand_rw() `cmd'; or together as needed */
#define NANDRW_READ	0x01
#define NANDRW_WRITE	0x00
#define NANDRW_JFFS2	0x02
#define NANDRW_JFFS2_SKIP	0x04

/*------move from cmd_nandrw.c---start--*/
#define USE_BBT
#define NFC_TIMEING_CAL
#define PMPMB_ADDR 0xd8130204
#define PMNAND_ADDR 0xd8130330
#define PMCS_ADDR 0xd8130000
#define WMT_PLL_CLOCK 25
#define NFC_MAX_CLOCK 60
#define NFC_SETUPTIME 15

WMT_NFC_CFG *pNFCRegs;
struct _NAND_PDMA_REG_ *pNand_PDma_Reg;
unsigned long *ReadDesc, *WriteDesc;
unsigned int g_WMTNFCBASE;

#define WRITE_NAND16(d, addr) (*(volatile unsigned short *)(addr) = (unsigned short)d)
#define RAED_NAND16(d, addr) (d = *(volatile unsigned short *)(addr))
#define RAED_NAND8(d, addr) (d = *(volatile unsigned char *)(addr))
#define WRITE_NAND8(d, addr) (*(volatile unsigned char *)(addr) = (unsigned char)d)
#define READ_NAND(adr) ((volatile unsigned char)(*(volatile __u8 *)(unsigned long)adr))

static unsigned char bbt_pattern[] = {'B', 'b', 't', '0' };
static unsigned char mirror_pattern[] = {'1', 't', 'b', 'B' };
static unsigned int *bbt;
static unsigned int bbt_version;
#define BBT_MAX_BLOCK 4
#define BBT_BITMAP 2
unsigned int bad_block_pos[10];
/*------move from cmd_nandrw.c---end--*/
/*
 * Function Prototypes
 */
static void nand_print(struct nand_chip *nand);
int nand_rw (struct nand_chip* nand, int cmd,
	    size_t start, size_t len,
	    size_t * retlen, u_char * buf);
int wmt_nand_erase(struct nand_chip *nand, unsigned int start);
int get_pattern_small(struct nand_chip *nand, unsigned int block, unsigned int *tag, unsigned int *version);
int get_pattern_large(struct nand_chip *nand, unsigned int block, unsigned int *tag, unsigned int *version);
int nand_page_program(struct nand_chip *nand, unsigned int start, unsigned int maddr, unsigned int len);
int nand_page_program_random(struct nand_chip *nand, unsigned int start, unsigned int maddr, unsigned int len);

int nand_read_page(struct nand_chip *nand, unsigned int start, unsigned int maddr, unsigned int len);
void nfc_ecc_set(unsigned int type, unsigned int ecc, struct nand_chip *nand);
int WMTLoadImageFormNAND(struct nand_chip *nand, unsigned int naddr, unsigned int maddr, unsigned int size);
int WMTSaveImageToNAND(struct nand_chip *nand, unsigned int naddr, unsigned int dwImageStart,
		unsigned int dwImageLength);
int nfc_BCH_read_page(struct nand_chip *nand, unsigned int start, unsigned int maddr, unsigned int len);
int nfc_BCH_read_8kpage(struct nand_chip *nand, unsigned int start, unsigned int maddr, unsigned int len);
int NFC_CHECK_ECC(void);
int nfc_1bit_ecc_correct(struct nand_chip *nand, unsigned int start, unsigned int maddr, unsigned int len);
int nfc_1bit_read_page(struct nand_chip *nand, unsigned int start, unsigned int maddr, unsigned int len);
int creat_bbt(struct nand_chip *nand);
int find_bbt(struct nand_chip *nand);
int update_bbt_inram(struct nand_chip *nand, unsigned int addr);
int update_bbt_inflash(struct nand_chip *nand, unsigned int last);
int isbbtbadblock(struct nand_chip *nand, unsigned int addr);
int tellme_badblock(struct nand_chip *nand);
int WMTEraseNAND(struct nand_chip *nand, unsigned int start, unsigned int nums, unsigned int all);
int tellme_whereistable(struct nand_chip *nand);
int nand_write_block(struct nand_chip *nand, unsigned int start, unsigned int naddr, unsigned int size, unsigned char *oob);
int tellme_nandinfo(struct nand_chip *nand);
int nand_readID(unsigned char *id);
int WMTEraseNANDALL(struct nand_chip *nand, unsigned int all);
int check_block_table(struct nand_chip *nand, unsigned int scan);
int nfc_BCH_ecc_check(unsigned int maddr);
void nfc_BCH_ecc_correct(unsigned int bitcnt, unsigned int maddr);
int wmt_calc_clock(struct nand_chip *nand, unsigned int spec_clk,
unsigned int *T, unsigned int *divisor, unsigned int *Thold);
int wmt_get_timing(struct nand_chip *nand, unsigned int nand_timing);

int nand_erase(struct nand_chip* nand, size_t ofs, size_t len, int clean);
static int nand_read_ecc(struct nand_chip *nand, size_t start, size_t len,
		 size_t * retlen, u_char *buf, u_char *ecc_code);
static int nand_write_ecc (struct nand_chip* nand, size_t to, size_t len,
			   size_t * retlen, const u_char * buf, u_char * ecc_code);
static void nand_print_bad(struct nand_chip *nand);
static int nand_read_oob(struct nand_chip* nand, size_t ofs, size_t len,
		 size_t * retlen, u_char * buf);
/*static int nand_write_oob(struct nand_chip* nand, size_t ofs, size_t len,
		 size_t * retlen, const u_char * buf);
static int NanD_WaitReady(struct nand_chip *nand, int ale_wait);*/
#ifdef CONFIG_MTD_NAND_ECC
/*static int nand_correct_data (u_char *dat, u_char *read_ecc, u_char *calc_ecc);
static void nand_calculate_ecc (const u_char *dat, u_char *ecc_code);*/
#endif

struct nand_chip nand_dev_desc[CFG_MAX_NAND_DEVICE] = {{0}};

/* Current NAND Device	*/
static int curr_device = -1;

/* ------------------------------------------------------------------------- */

int do_nand (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    int rcode = 0;

    switch (argc) {
    case 0:
    case 1:
	printf ("Usage:\n%s\n", cmdtp->usage);
	return 1;
    case 2:
	if (strcmp(argv[1],"info") == 0) {
		int i;

		putc ('\n');

		for (i=0; i<CFG_MAX_NAND_DEVICE; ++i) {
			if(nand_dev_desc[i].ChipID == NAND_ChipID_UNKNOWN)
				continue; /* list only known devices */
			printf ("Device %d: ", i);
			nand_print(&nand_dev_desc[i]);
		}
		return 0;

	} else if (strcmp(argv[1],"device") == 0) {
		if ((curr_device < 0) || (curr_device >= CFG_MAX_NAND_DEVICE)) {
			puts ("\nno devices available\n");
			return 1;
		}
		printf ("\nDevice %d: ", curr_device);
		nand_print(&nand_dev_desc[curr_device]);
		return 0;

	} else if (strcmp(argv[1],"bad") == 0) {
		if ((curr_device < 0) || (curr_device >= CFG_MAX_NAND_DEVICE)) {
			puts ("\nno devices available\n");
			return 1;
		}
		printf ("\nDevice %d bad blocks:\n", curr_device);
		nand_print_bad(&nand_dev_desc[curr_device]);
		return 0;

	}
	printf ("Usage:\n%s\n", cmdtp->usage);
	return 1;
    case 3:
	if (strcmp(argv[1],"device") == 0) {
		int dev = (int)simple_strtoul(argv[2], NULL, 10);

		printf ("\nDevice %d: ", dev);
		if (dev >= CFG_MAX_NAND_DEVICE) {
			puts ("unknown device\n");
			return 1;
		}
		nand_print(&nand_dev_desc[dev]);
		/*nand_print (dev);*/

		if (nand_dev_desc[dev].ChipID == NAND_ChipID_UNKNOWN) {
			return 1;
		}

		curr_device = dev;

		puts ("... is now current device\n");

		return 0;
	}	else if (strncmp(argv[1], "erase", 4) == 0 || strncmp(argv[1], "erase", 3) == 0) {
		struct nand_chip* nand = &nand_dev_desc[curr_device];
		if (strcmp(argv[2], "all") == 0) {
			printf("Erase all\n");
			WMTEraseNAND(nand, 0, 0, 1);
			return 0;
		} else if (strcmp(argv[2], "force") == 0) {
			printf("Erase all\n");
			WMTEraseNANDALL(nand, 1);

			return 0;
		} else if (strcmp(argv[2], "table") == 0) {
			printf("Erase Table\n");
			WMTEraseNANDALL(nand, 0);

			return 0;
		}
		return 0;
	}	else if (strncmp(argv[1], "tellme", 3) == 0) {
		struct nand_chip* nand = &nand_dev_desc[curr_device];
		if (strcmp(argv[2], "bad") == 0)
			tellme_badblock(nand);
		if (strcmp(argv[2], "table") == 0)
			tellme_whereistable(nand);
		if (strcmp(argv[2], "nand") == 0)
			tellme_nandinfo(nand);
		return 0;
	}

	printf ("Usage:\n%s\n", cmdtp->usage);
	return 1;
    default:
	/* at least 4 args */

	if (strncmp(argv[1], "read", 4) == 0 ||
	    strncmp(argv[1], "write", 5) == 0) {
		ulong addr = simple_strtoul(argv[2], NULL, 16);
		ulong off  = simple_strtoul(argv[3], NULL, 16);
		ulong size = simple_strtoul(argv[4], NULL, 16);
		int cmd    = (strncmp(argv[1], "read", 4) == 0) ?
				NANDRW_READ : NANDRW_WRITE;
		int ret, total;
		char* cmdtail = strchr(argv[1], '.');

		if (cmdtail && !strncmp(cmdtail, ".oob", 2)) {
			/* read out-of-band data */
			if (cmd & NANDRW_READ) {
				ret = nand_read_oob(nand_dev_desc + curr_device,
						    off, size, (size_t *)&total,
						    (u_char*)addr);
			}
			else {
				ret = 0;
				/*ret = nand_write_oob(nand_dev_desc + curr_device,
						     off, size, (size_t *)&total,
						     (u_char*)addr);*/
			}
			return ret;
		}
		else if (cmdtail && !strncmp(cmdtail, ".jffs2s", 7)) {
			cmd |= NANDRW_JFFS2;	/* skip bad blocks (on read too) */
			if (cmd & NANDRW_READ)
				cmd |= NANDRW_JFFS2_SKIP;	/* skip bad blocks (on read too) */
		}
		else if (cmdtail && !strncmp(cmdtail, ".jffs2", 2))
			cmd |= NANDRW_JFFS2;	/* skip bad blocks (fill with 0xFF)*/
#ifdef SXNI855T
		/* need ".e" same as ".j" for compatibility with older units */
		else if (cmdtail && !strcmp(cmdtail, ".e"))
			cmd |= NANDRW_JFFS2;	/* skip bad blocks */
#endif
#ifdef CFG_NAND_SKIP_BAD_DOT_I
		/* need ".i" same as ".jffs2s" for compatibility with older units (esd) */
		/* ".i" for image -> read skips bad block (no 0xff) */
		else if (cmdtail && !strcmp(cmdtail, ".i")) {
			cmd |= NANDRW_JFFS2;	/* skip bad blocks (on read too) */
			if (cmd & NANDRW_READ)
				cmd |= NANDRW_JFFS2_SKIP;	/* skip bad blocks (on read too) */
		}
#endif /* CFG_NAND_SKIP_BAD_DOT_I */
		else if (cmdtail) {
			printf ("Usage:\n%s\n", cmdtp->usage);
			return 1;
		}

		printf ("\nNAND %s: device %d offset %ld, size %ld ... ",
			(cmd & NANDRW_READ) ? "read" : "write",
			curr_device, off, size);

		ret = nand_rw(nand_dev_desc + curr_device, cmd, off, size,
			     (size_t *)&total, (u_char*)addr);

		printf (" %d bytes %s: %s\n", total,
			(cmd & NANDRW_READ) ? "read" : "written",
			ret ? "ERROR" : "OK");

		return ret;
	} else if (strncmp(argv[1], "r", 1) == 0 && argc == 5) {
		int ret;
		ulong off = simple_strtoul(argv[2], NULL, 16);
		ulong maddr  = simple_strtoul(argv[3], NULL, 16);
		ulong size = simple_strtoul(argv[4], NULL, 16);
		if (!size || curr_device < 0) {
				printf("size=0\n");
				return 1;
		}
		ret = WMTLoadImageFormNAND(nand_dev_desc + curr_device, off, maddr, size);
		return ret;
	} else if (strncmp(argv[1], "w", 1) == 0 && argc == 5) {
		int ret;
		ulong maddr = simple_strtoul(argv[2], NULL, 16);
		ulong off = simple_strtoul(argv[3], NULL, 16);
		ulong size = simple_strtoul(argv[4], NULL, 16);
		if (!size || curr_device < 0) {
				printf("size=0\n");
				return 1;
		}
		ret = WMTSaveImageToNAND(nand_dev_desc + curr_device, off, maddr, size);
		return ret;
	} else if ((strcmp(argv[1],"erase") == 0 && strncmp(argv[2], "address", 4) == 0)
	&& strcmp(argv[3], "clean") == 0) {
		struct nand_chip* nand = &nand_dev_desc[curr_device];
		ulong off = 0;
		ulong size = nand->totlen;
		int ret;

		printf ("\nNAND erase: device %d offset %ld, size %ld ... ",
			curr_device, off, size);

		ret = nand_erase(nand, off, size, 1);

		printf("%s\n", ret ? "ERROR" : "OK");

		return ret;
	} else if ((strcmp(argv[1],"erase") == 0 && (strcmp("address", argv[2]) == 0))
	&& (argc == 5 || strcmp("clean", argv[3]) == 0)) {
		int clean = argc == 6;
		ulong off = simple_strtoul(argv[3 + clean], NULL, 16);
		ulong size = simple_strtoul(argv[4 + clean], NULL, 16);
		int ret;

		printf ("\nNAND erase: device %d offset %ld, size %ld ... ",
			curr_device, off, size);

		ret = nand_erase(nand_dev_desc + curr_device, off, size, clean);

		printf("%s\n", ret ? "ERROR" : "OK");

		return ret;
	} else if (strcmp(argv[1],"erase") == 0 &&
		   (argc == 4)) {
		ulong start_blk = simple_strtoul(argv[2], NULL, 16);
		ulong blk_num = simple_strtoul(argv[3], NULL, 16);
		WMTEraseNAND(nand_dev_desc + curr_device, start_blk, blk_num, 0);
	} else {
		printf ("Usage:\n%s\n", cmdtp->usage);
		rcode = 1;
	}

	return rcode;
    }
}

U_BOOT_CMD(
	nandrw,	5,	1,	do_nand,
	"nandrw  - NAND sub-system\n",
	"nandrw info  - show available NAND devices\n"
	"nandrw device [dev] - show or set current device\n"
	"nandrw read[.jffs2[s]]  addr off size\n"
	"nandrw write[.jffs2] addr off size - read/write `size' bytes starting\n"
	"    at offset `off' to/from memory address `addr'\n"
	"nandrw erase address [clean] [off size] - erase `size' bytes from\n"
	"    offset `off' (entire device if not specified)\n"
	"nandrw bad - show bad blocks by checking initial defective marker\n"
	" \r\n"
	"nandrw erase start_block block_numbers - erase blocks skip bad block\n"
	"nandrw erase all   - erase full blocks except table blocks (last 4 blocks)\n"
	"nandrw erase table - erase table blocks (last 4 blocks)\n"
	/*"nandrw erase force - erase entire device including table blocks\n"*/
	"nandrw tellme nand - show nand flash info\n"
	"nandrw tellme bad - show bad blocks from bad block table\n"
	"nandrw tellme table - show bad block table\n"
	"nandrw read.oob addr off size - read out-of-band data\n"
	"nandrw r off addr size - read nand flash from offset 'off' to memory\n"
	"    address `addr' with skip bad blocks\n"
	"nandrw w addr off size - write nand flash from memory address 'addr' to nand\n"
	"    flash offset `off' with skip bad blocks\n"
	/*"nandrw write.oob addr off size - read out-of-band data\n"*/

);

int do_nandboot (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char *boot_device = NULL;
	char *ep;
	int dev;
	ulong cnt;
	ulong addr;
	ulong offset = 0;
	image_header_t *hdr;
	int rcode = 0;
	switch (argc) {
	case 1:
		addr = CFG_LOAD_ADDR;
		boot_device = getenv ("bootdevice");
		break;
	case 2:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = getenv ("bootdevice");
		break;
	case 3:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = argv[2];
		break;
	case 4:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = argv[2];
		offset = simple_strtoul(argv[3], NULL, 16);
		break;
	default:
		printf ("Usage:\n%s\n", cmdtp->usage);
		SHOW_BOOT_PROGRESS (-1);
		return 1;
	}

	if (!boot_device) {
		puts ("\n** No boot device **\n");
		SHOW_BOOT_PROGRESS (-1);
		return 1;
	}

	dev = simple_strtoul(boot_device, &ep, 16);

	if ((dev >= CFG_MAX_NAND_DEVICE) ||
	    (nand_dev_desc[dev].ChipID == NAND_ChipID_UNKNOWN)) {
		printf ("\n** Device %d not available\n", dev);
		SHOW_BOOT_PROGRESS (-1);
		return 1;
	}

	printf ("\nLoading from device %d: %s at 0x%lx (offset 0x%lx)\n",
		dev, nand_dev_desc[dev].name, nand_dev_desc[dev].IO_ADDR,
		offset);

	if (nand_rw(nand_dev_desc + dev, NANDRW_READ, offset,
		    SECTORSIZE, NULL, (u_char *)addr)) {
		printf ("** Read error on %d\n", dev);
		SHOW_BOOT_PROGRESS (-1);
		return 1;
	}

	hdr = (image_header_t *)addr;

	if (ntohl(hdr->ih_magic) == IH_MAGIC) {

		print_image_hdr (hdr);

		cnt = (ntohl(hdr->ih_size) + sizeof(image_header_t));
		cnt -= SECTORSIZE;
	} else {
		printf ("\n** Bad Magic Number 0x%x **\n", hdr->ih_magic);
		SHOW_BOOT_PROGRESS (-1);
		return 1;
	}

	if (nand_rw(nand_dev_desc + dev, NANDRW_READ, offset + SECTORSIZE, cnt,
		    NULL, (u_char *)(addr+SECTORSIZE))) {
		printf ("** Read error on %d\n", dev);
		SHOW_BOOT_PROGRESS (-1);
		return 1;
	}

	/* Loading ok, update default load address */

	load_addr = addr;

	/* Check if we should attempt an auto-start */
	if (((ep = getenv("autostart")) != NULL) && (strcmp(ep,"yes") == 0)) {
		char *local_args[2];
		extern int do_bootm (cmd_tbl_t *, int, int, char *[]);

		local_args[0] = argv[0];
		local_args[1] = NULL;

		printf ("Automatic boot of image at addr 0x%08lx ...\n", addr);

		do_bootm (cmdtp, 0, 1, local_args);
		rcode = 1;
	}
	return rcode;
}

U_BOOT_CMD(
	nboot,	4,	1,	do_nandboot,
	"nboot   - boot from NAND device\n",
	"loadAddr dev\n"
);

void WRITE_NAND_COMMAND(unsigned int cmd)
{
	pNFCRegs->NFCR2 = (unsigned char)cmd;
}

void nfc_ecc_set(unsigned int type, unsigned int ecc, struct nand_chip *nand)
{
	if (type == USE_HW_ECC) {
		printf("USE_HW_ECC ");
		pNFCRegs->NFCR15 &= (~(unsigned int)USE_SW_ECC);
		pNFCRegs->NFCR23 &= 0xfffffff8;
    pNFCRegs->NFCR23 |= ecc;
	/* enable 4bit ecc interrupt and new structure */
		if (ecc == ECC12bit || ecc == ECC4bit) {
			if (ecc == ECC12bit)
				printf("ECC12bit\n");
			else
				printf("ECC4bit\n");
			if (nand->oobblock < 8192)
				nand->nfc_read_page = nfc_BCH_read_page;
			else
				nand->nfc_read_page = nfc_BCH_read_8kpage;
			pNFCRegs->NFCR24 = eccBCH_interrupt_enable;
			pNFCRegs->NFCR12 &= (~OLDATA_EN);
			pNFCRegs->NFCR23 |= READ_RESUME;/* do for safty			 */
		} else if (ecc == ECC1bit) {
			/* disable 4bit ecc interrupt and old structure  */
			printf("ECC1bit\n");
			nand->nfc_read_page = nfc_1bit_read_page;
			/* make sure our status is clear before we start */
			pNFCRegs->NFCR1f |= 0x07;
			pNFCRegs->NFCR20 |= 0xffffffff;
			/* pNFCRegs->NFCR24 = ecc4bit_inetrrupt_disable; */
			pNFCRegs->NFCR12 |= OLDATA_EN;
			pNFCRegs->NFCR23 |= READ_RESUME;/* do for safty */
		}
	} else {
		printf("USE_SW_ECC\n");
		nand->nfc_read_page = nand_read_page;
		pNFCRegs->NFCR15 |= USE_SW_ECC;
		pNFCRegs->NFCR12 &= (~OLDATA_EN);
		pNFCRegs->NFCR23 &= 0xfffffff8;
	}
}

int shift_bit(unsigned int value)
{
	int i = 0;
	while (!(value & 1)) {
		value >>= 1;
		i++;
		if (i == 32)
			break;
	}
	/* return the number count of "zero" bit */
	return i;
}

int NFC_WAIT_READY(void)
{
	int i = 0;

	while (1) {
		if (!(pNFCRegs->NFCRa & NFC_BUSY))
			break;
		if (i>>21)
			return -3;
		i++;
	}
	return 0;
}

int NAND_WAIT_READY(void)
{
	int i = 0;

	while (1) {
		if (pNFCRegs->NFCRb & B2R)
			break;
		if ((i>>21))
			return -1;
		i++;
	}
	pNFCRegs->NFCRb |= B2R;
	return 0;
}

int NFC_WAIT_CMD_READY(void)
{
	int i = 0;

	while (1) {
		if (!(pNFCRegs->NFCRa & NFC_CMD_RDY))
			break;
		if (i>>21)
			return -4;
		i++;
	}
	return 0;
}

int NAND_WAIT_IDLE(void)
{
	int i = 0;

	while (1) {
		if (pNFCRegs->NFCR1d & NFC_IDLE)
			break;
		if (i>>21)
			return -2;
		i++;
	}
	return 0;
}
/* returns 0 if block containing pos is OK:
 *		valid erase block and
 *		not marked bad, or no bad mark position is specified
 * returns 1 if marked bad or otherwise invalid
 */
int check_block (struct nand_chip *nand, unsigned long pos)
{
	/*size_t retlen;
	uint8_t oob_data;
	uint16_t oob_data16[6];*/
	int j, rc = -1;
	int page[2];
	page[0] = pos & (-nand->erasesize);
	page[1] = page[0];
	int badpos = (nand->dwBIOffset&0xffff);//oob_config.badblock_pos;
	page[0] += ((nand->dwBI0Position&0xffff))<<nand->page_shift;
	page[1] += ((nand->dwBI1Position&0xffff))<<nand->page_shift;

	if (pos >= nand->totlen)
		return 1;

	if (badpos < 0)
		return 0;	/* no way to check, assume OK */

	for (j = 0; j < 2; j++) {
		rc = nand->nfc_read_page(nand, page[j],	(unsigned int)nand->data_buf, nand->oobblock);
		if (rc) {
			printf("scan bbt err at addr %d, rc=%d\n", page[j], rc);
			if (rc != -ERR_ECC_UNCORRECT)
				return rc;
			printf("read page fail:find bad block : block%d\n", pos/nand->erasesize);
			return 1;
		} else {
			if (pNFCRegs->FIFO[badpos] != 0xff) {
				printf("find bad block : block%d\n", pos/nand->erasesize);
				return 1;
			}
		}
	}
	#if 0
	if (nand->bus16) {
		if (nand_read_oob(nand, (page0 + 0), 12, &retlen, (uint8_t *)oob_data16)
		    || (oob_data16[2] & 0xff00) != 0xff00)
			return 1;
		if (nand_read_oob(nand, (page1 + 0), 12, &retlen, (uint8_t *)oob_data16)
		    || (oob_data16[2] & 0xff00) != 0xff00)
			return 1;
	} else {
		/* Note - bad block marker can be on first or second page */
		if (nand_read_oob(nand, page0 + badpos, 1, &retlen, (unsigned char *)&oob_data)
		    || oob_data != 0xff
		    || nand_read_oob (nand, page1 + badpos, 1, &retlen, (unsigned char *)&oob_data)
		    || oob_data != 0xff)
			return 1;
	}
	#endif
	

	return 0;
}

/* print bad blocks in NAND flash */
static void nand_print_bad(struct nand_chip* nand)
{
	unsigned long pos;
	nfc_ecc_set(USE_SW_ECC, ECC1bit, nand);
	NAND_ENABLE_CE(nand);
	for (pos = 0; pos < nand->totlen; pos += nand->erasesize) {
		if (check_block(nand, pos))
			printf(" 0x%8.8lx\n", pos);
	}
	NAND_DISABLE_CE(nand);
	if (nand->oobblock > 0x200) {
		if (nand->oobsize >= 218)
			nfc_ecc_set(USE_HW_ECC, ECC12bit, nand);
		else
			nfc_ecc_set(USE_HW_ECC, ECC4bit, nand);
	} else
		nfc_ecc_set(USE_HW_ECC, ECC1bit, nand);
	puts("\n");
}

/* cmd: 0: NANDRW_WRITE			write, fail on bad block
 *	1: NANDRW_READ			read, fail on bad block
 *	2: NANDRW_WRITE | NANDRW_JFFS2	write, skip bad blocks
 *	3: NANDRW_READ | NANDRW_JFFS2	read, data all 0xff for bad blocks
 *      7: NANDRW_READ | NANDRW_JFFS2 | NANDRW_JFFS2_SKIP read, skip bad blocks
 */
int nand_rw (struct nand_chip* nand, int cmd,
	    size_t start, size_t len,
	    size_t * retlen, u_char * buf)
{
	int ret = 0, n, total = 0;
	char eccbuf[6];
	/* eblk (once set) is the start of the erase block containing the
	 * data being processed.
	 */
	unsigned long eblk = ~0;	/* force mismatch on first pass */
	unsigned long erasesize = nand->erasesize;
	NAND_ENABLE_CE(nand);  /* set pin low */
	if (check_block_table(nand, 0)) {
		NAND_DISABLE_CE(nand);  /* set pin high */
		return -1;
	}

	while (len) {
		if ((start & (-erasesize)) != eblk) {
			/* have crossed into new erase block, deal with
			 * it if it is sure marked bad.
			 */
			eblk = start & (-erasesize); /* start of block */
			/*printf("check bad block in eblk = 0x%x\n", eblk);*/
			ret = nand->isbadblock(nand, eblk);
			/*printf("check bad block out ret= %d\n", ret);*/
			if (ret < 0) {
				printf("bbt is not exist\n");
				NAND_DISABLE_CE(nand);  /* set pin high */
				return ret;
			} else if (ret > 0) {
				if (cmd == (NANDRW_READ | NANDRW_JFFS2)) {
					while (len > 0 &&
					       start - eblk < erasesize) {
						*(buf++) = 0xff;
						++start;
						++total;
						--len;
					}
					continue;
				} else if (cmd == (NANDRW_READ | NANDRW_JFFS2 | NANDRW_JFFS2_SKIP)) {
					start += erasesize;
					continue;
				} else if (cmd == (NANDRW_WRITE | NANDRW_JFFS2)) {
					/* skip bad block */
					start += erasesize;
					continue;
				} else {
					ret = 1;
					break;
				}
			}
		}
		/* The ECC will not be calculated correctly if
		   less than 512 is written or read */
		/* Is request at least 512 bytes AND it starts on a proper boundry */
		if((start != ROUND_DOWN(start, 0x200)) || (len < 0x200))
			printf("Warning block writes should be at least 512 bytes and start on a 512 byte boundry\n");

		if (cmd & NANDRW_READ) {
			ret = nand_read_ecc(nand, start,
					   min(len, eblk + erasesize - start),
					   (size_t *)&n, (u_char*)buf, (u_char *)eccbuf);
		} else {
			ret = nand_write_ecc(nand, start,
					    min(len, eblk + erasesize - start),
					    (size_t *)&n, (u_char*)buf, (u_char *)eccbuf);
		}

		if (ret&1)
			break;

		start  += n;
		buf   += n;
		total += n;
		len   -= n;
	}
	if (retlen)
		*retlen = total;

	NAND_DISABLE_CE(nand);  /* set pin high */
	return (ret&1);
}

static void nand_print(struct nand_chip *nand)
{
	if (nand->numchips > 1) {
		printf("%s at 0x%lx,\n"
		       "\t  %d chips %s, size %d MB, \n"
		       "\t  total size %ld MB, sector size %ld kB\n",
		       nand->name, nand->IO_ADDR, nand->numchips,
		       nand->chips_name, 1 << (nand->chipshift - 20),
		       nand->totlen >> 20, nand->erasesize >> 10);
	}
	else {
		printf("%s at 0x%lx (", nand->chips_name, nand->IO_ADDR);
		print_size(nand->totlen, ", ");
		print_size(nand->erasesize, " sector)\n");
	}
}

/* ------------------------------------------------------------------------- */
#if 0
static int NanD_WaitReady(struct nand_chip *nand, int ale_wait)
{
	/* This is inline, to optimise the common case, where it's ready instantly */
	int ret = 0;

#ifdef NAND_NO_RB	/* in config file, shorter delays currently wrap accesses */
	if(ale_wait)
		NAND_WAIT_READY();	/* do the worst case 25us wait */
	else
		udelay(10);
#else	/* has functional r/b signal */
	NAND_WAIT_READY();
#endif
	return ret;
}

/* NanD_Command: Send a flash command to the flash chip */
#if 0
static inline int NanD_Command(struct nand_chip *nand, unsigned char command)
{
	unsigned long nandptr = nand->IO_ADDR;

	/* Assert the CLE (Command Latch Enable) line to the flash chip */
	NAND_CTL_SETCLE(nandptr);

	/* Send the command */
	WRITE_NAND_COMMAND(command, nandptr);

	/* Lower the CLE line */
	NAND_CTL_CLRCLE(nandptr);

#ifdef NAND_NO_RB
	if(command == NAND_CMD_RESET){
		u_char ret_val;
		NanD_Command(nand, NAND_CMD_STATUS);
		do {
			ret_val = READ_NAND(nandptr);/* wait till ready */
		} while((ret_val & 0x40) != 0x40);
	}
#endif
	return NanD_WaitReady(nand, 0);
}
#endif
#endif
/* NanD_Address: Set the current address for the flash chip */
void NanD_Address(struct nand_chip *nand, int numbytes, unsigned int col, unsigned int row)
{
	unsigned char addr[10];
	unsigned int i = 0, tmp;
	unsigned int nandptr = 0;

	memset(addr, 0, 10);
	nandptr = g_WMTNFCBASE + 0x0c;
	if (numbytes == ADDR_COLUMN_PAGE) {
		for (i = 0; i < nand->col; i++)
			addr[i] = (col>>(i*8));
		for (i = nand->col; i < (nand->row + nand->col); i++)
			addr[i] = (row>>((i-nand->col)*8));
		for (i = 0; i <= (nand->col+nand->row); i += 2, nandptr += 4) {
			tmp = (addr[i+1]<<8)+addr[i];
			WRITE_NAND16(tmp, nandptr);
			/*printf("write %x to %8.8x\n",tmp,nandptr);*/
		}
	}
	if (numbytes == ADDR_COLUMN) {
			for (i = 0; i < nand->col; i++)
				addr[i] = (col>>(i*8));
		for (i = 0; i < nand->col; i += 2, nandptr += 4) {
			tmp = (addr[i+1]<<8)+addr[i];
			WRITE_NAND16(tmp, nandptr);
		}
	}
	if (numbytes == ADDR_PAGE) {
		for (i = 0; i < nand->row; i++)
				addr[i] = (row>>(i*8));

		for (i = 0; i < nand->row; i += 2, nandptr += 4) {
			tmp = (addr[i+1]<<8)+addr[i];
			WRITE_NAND16(tmp, nandptr);
		}
	}
}
#if 0
static int NanD_Address(struct nand_chip *nand, int numbytes, unsigned long ofs)
{
	unsigned long nandptr;
	int i;

	nandptr = nand->IO_ADDR;

	/* Assert the ALE (Address Latch Enable) line to the flash chip */
	NAND_CTL_SETALE(nandptr);

	/* Send the address */
	/* Devices with 256-byte page are addressed as:
	 * Column (bits 0-7), Page (bits 8-15, 16-23, 24-31)
	 * there is no device on the market with page256
	 * and more than 24 bits.
	 * Devices with 512-byte page are addressed as:
	 * Column (bits 0-7), Page (bits 9-16, 17-24, 25-31)
	 * 25-31 is sent only if the chip support it.
	 * bit 8 changes the read command to be sent
	 * (NAND_CMD_READ0 or NAND_CMD_READ1).
	 */

	if (numbytes == ADDR_COLUMN || numbytes == ADDR_COLUMN_PAGE)
		WRITE_NAND_ADDRESS(ofs, nandptr);

	ofs = ofs >> nand->page_shift;

	if (numbytes == ADDR_PAGE || numbytes == ADDR_COLUMN_PAGE) {
		for (i = 0; i < nand->pageadrlen; i++, ofs = ofs >> 8) {
			WRITE_NAND_ADDRESS(ofs, nandptr);
		}
	}

	/* Lower the ALE line */
	NAND_CTL_CLRALE(nandptr);

	/* Wait for the chip to respond */
	return NanD_WaitReady(nand, 1);
}
#endif

int nfc_dma_cfg(unsigned int buf, unsigned int len, unsigned int wr)
{
	unsigned int status, CurDes_off = 0, i;

	if (!len || buf & 0x03) {
		printf("Error : length = %x , address = %x\r\n", len, buf);
		return -1;
	}

	if (!ReadDesc) {
		printf("alloc ReadDesc failed\n");
		return -1;
	}
	pNFCRegs->NFCR8 = len-1;
	/*	Interrupt setting	*/
	/*disable_irq();
	set_irq_handlers(IRQ_NFC_DMA, nand_dma_isr);
	set_int_route(IRQ_NFC_DMA, 0);
	unmask_interrupt(IRQ_NFC_DMA);
	enable_irq();*/

	if (pNand_PDma_Reg->DMA_ISR & NAND_PDMA_IER_INT_STS)
		pNand_PDma_Reg->DMA_ISR = NAND_PDMA_IER_INT_STS;

	if (pNand_PDma_Reg->DMA_ISR & NAND_PDMA_IER_INT_STS) {
		printf("PDMA interrupt status can't be clear ");
		printf("pNand_PDma_Reg->DMA_ISR = 0x%8.8x \n", (unsigned int)pNand_PDma_Reg->DMA_ISR);
	}

	status = nand_init_pdma();
	if (status)
		printf("nand_init_pdma fail status = 0x%x", status);
	nand_alloc_desc_pool(ReadDesc);
	/*printf(" ReadDesc = 0x%x\r\n", (unsigned int ) ReadDesc);*/
	/*printf(" buf = 0x%x\r\n", (unsigned int ) buf);*/
	for (i = 0; i < (len/512); i++) {
		nand_init_short_desc(ReadDesc+CurDes_off, 512, (unsigned long *)(buf+i*512),
		((i == ((len/512)-1)) && !(len%512)) ? 1 : 0);
		CurDes_off += sizeof(NAND_PDMA_DESC_S)/4;
	}
	if (len%512)
		nand_init_short_desc(ReadDesc+CurDes_off, (len%512),
		(unsigned long *)(buf+i*512), 1);	

	nand_config_pdma(ReadDesc, wr);
	return 0;
}

int nand_init_pdma(void)
{
	pNand_PDma_Reg->DMA_GCR = NAND_PDMA_GCR_SOFTRESET;
	pNand_PDma_Reg->DMA_GCR = NAND_PDMA_GCR_DMA_EN;
	if (pNand_PDma_Reg->DMA_GCR & NAND_PDMA_GCR_DMA_EN)
		return 0;
	else
		return 1;
}


int nand_free_pdma(void)
{
	pNand_PDma_Reg->DMA_DESPR	= 0;
	pNand_PDma_Reg->DMA_GCR = 0;
	return 0;
}


int nand_alloc_desc_pool(unsigned long *DescAddr)
{
	int i;
	for (i = 0; i < 40; i++)
		*(DescAddr+i) = 0;
	return 0;
}

int nand_init_short_desc(unsigned long *DescAddr, unsigned int ReqCount, unsigned long *BufferAddr, int End)
{
	struct _NAND_PDMA_DESC_S *CurDes_S;
	CurDes_S = (struct _NAND_PDMA_DESC_S *) DescAddr;
	CurDes_S->ReqCount = ReqCount;
	CurDes_S->i = End;
	CurDes_S->end = End;
	CurDes_S->format = 0;
	CurDes_S->DataBufferAddr = (unsigned long)BufferAddr;
	return 0;
}

int nand_init_long_desc(unsigned long *DescAddr, unsigned int ReqCount, unsigned long *BufferAddr,
unsigned long *BranchAddr, int End)
{
	struct _NAND_PDMA_DESC_L *CurDes_L;
	CurDes_L = (struct _NAND_PDMA_DESC_L *) DescAddr;
	CurDes_L->ReqCount = ReqCount;
	CurDes_L->i = 0;
	CurDes_L->format = 1;
	CurDes_L->DataBufferAddr = (unsigned long)BufferAddr;
	CurDes_L->BranchAddr = (unsigned long)BranchAddr;
	if (End) {
		CurDes_L->end = 1;
		CurDes_L->i = 1;
	}

	return 0;
}


int nand_config_pdma(unsigned long *DescAddr, unsigned int dir)
{
	/*pNand_PDma_Reg->DMA_IER = NAND_PDMA_IER_INT_EN;*/
	pNand_PDma_Reg->DMA_DESPR = (unsigned long)DescAddr;
	if (dir == NAND_PDMA_READ)
		pNand_PDma_Reg->DMA_CCR |= NAND_PDMA_CCR_peripheral_to_IF;
	else
		pNand_PDma_Reg->DMA_CCR &= ~NAND_PDMA_CCR_IF_to_peripheral;

	pNand_PDma_Reg->DMA_CCR |= NAND_PDMA_CCR_RUN;

	return 0;
}

int nand_pdma_handler(void)
{
	unsigned long status = 0;
	unsigned long count = 0;

	count = 0x100000;
	/*	 polling CSR TC status	*/
	do {
		count--;
		if (pNand_PDma_Reg->DMA_ISR & NAND_PDMA_IER_INT_STS) {
			status = pNand_PDma_Reg->DMA_CCR & NAND_PDMA_CCR_EvtCode;
			pNand_PDma_Reg->DMA_ISR &= NAND_PDMA_IER_INT_STS;
			break ;
		}
		if (count == 0) {
			printf("PDMA Time Out!\n");
			printf("pNand_PDma_Reg->DMA_CCR = 0x%8.8x\r\n",
			(unsigned int)pNand_PDma_Reg->DMA_CCR);

			break;
		}
	} while (1);
	if (status == NAND_PDMA_CCR_Evt_ff_underrun)
		printf("PDMA Buffer under run!\n");

	if (status == NAND_PDMA_CCR_Evt_ff_overrun)
		printf("PDMA Buffer over run!\n");

	if (status == NAND_PDMA_CCR_Evt_desp_read)
		printf("PDMA read Descriptor error!\n");

	if (status == NAND_PDMA_CCR_Evt_data_rw)
		printf("PDMA read/write memory descriptor error!\n");

	if (status == NAND_PDMA_CCR_Evt_early_end)
		printf("PDMA read early end!\n");

	if (count == 0) {
		printf("PDMA TimeOut!\n");
		while (1)
			;
	}

	return 0;
}

int NFC_CHECK_ECC(void)
{
	int i = 0;

	while (1) {
		if (!(pNFCRegs->NFCRa & NFC_BUSY))
			break;
		if (pNFCRegs->NFCR25 == (ERR_CORRECT | BCH_ERR))
			return 1;
		if (i>>21)
			return -3;
		i++;
	}
	if (pNFCRegs->NFCR25&BCH_ERR) {
		while(1) {
			if (pNFCRegs->NFCR25&ERR_CORRECT)
				return 1;
		}
	}
	return 0;
}

void nfc_BCH_ecc_correct(unsigned int bitcnt, unsigned int maddr)
{
	unsigned int ofs, i = 0, type;
	unsigned int posptr;
	unsigned short err_ofs;
	unsigned int byte_ofs, bit_ofs;
	unsigned char err_data;
	unsigned int ofs1 = 0;

#ifdef SHOWME_ECC
	printf("error happen at 0x%x", maddr);
#endif
	posptr = g_WMTNFCBASE + 0x9c;
	type = pNFCRegs->NFCR26 & BANK_DR;
	ofs = (pNFCRegs->NFCR26 & BANK_NUM)>>8;
	if (type) {
#ifdef SHOWME_ECC
		printf(" ( %d ) red area ",ofs);
#endif
		ofs = g_WMTNFCBASE + 0xc0;
	} else {
#ifdef SHOWME_ECC
		printf(" ( %d ) data area ",ofs);
#endif
		ofs1 = (ofs*512);
		ofs = maddr + (ofs*512);
	}

	for (i = 0; i < bitcnt; i++, posptr+=2) {
		RAED_NAND16(err_ofs, posptr);
		byte_ofs = (unsigned int)(err_ofs>>3);
		bit_ofs = (unsigned int)(err_ofs & 0x07);
#ifdef POST_TEST_FUNS
	if(type) {
		g_ecc_result.pred[g_ecc_rcnt] = (byte_ofs*8)+bit_ofs;
		g_ecc_rcnt++;
		if (g_ecc_rcnt >= MAX_ECC_BIT)
			g_ecc_rcnt = 0;
	} else {
		g_ecc_result.pos[g_ecc_dcnt] = ((ofs1+byte_ofs)*8)+bit_ofs;		
		g_ecc_dcnt++;
		if (g_ecc_dcnt >= MAX_ECC_BIT)
			g_ecc_dcnt = 0;
	}
#endif

		if (byte_ofs >= 512)
			continue;

#ifdef SHOWME_ECC
		printf(" byte%d , bit%d \n",byte_ofs,bit_ofs);
#endif
		RAED_NAND8(err_data, (ofs+byte_ofs));
#ifdef SHOWME_ECC
		printf("org ( 0x%x ) 0x%x => ",(ofs+byte_ofs),err_data);
#endif
    	if (err_data & (1<<bit_ofs))
    		err_data &= ~(1<<bit_ofs);
    	else
    		err_data |= (1<<bit_ofs);
#ifdef SHOWME_ECC
		printf(" 0x%x\n",err_data);
#endif
		WRITE_NAND8(err_data, (ofs+byte_ofs));
	}
}

int nfc_BCH_ecc_check(unsigned int maddr)
{
	unsigned int cnt = 0, count = 0x100;
	int rc = 0;

	if (pNFCRegs->NFCR25 == (ERR_CORRECT|BCH_ERR)) {
		cnt = pNFCRegs->NFCR26 & BCH_ERR_CNT;
		if (cnt == BCH_ERR_CNT) {
			printf("Too Many Error\n");
			rc = -ERR_ECC_UNCORRECT;
		} else {
			if (!cnt) {
				printf("report ecc error : count =%d\n", cnt);
				pNFCRegs->NFCR23 |= READ_RESUME;
				rc = -ERR_UNKNOW;
			}
		}
		if (rc) {
			pNFCRegs->NFCR25 = (ERR_CORRECT|BCH_ERR);
			pNFCRegs->NFCR23 |= READ_RESUME;
			return rc;
		}

		while ((0 != (pNand_PDma_Reg->DMA_RBR%0x200)) && (count--))
			printf("dma is not finish please wait");

		nfc_BCH_ecc_correct(cnt, maddr);
	}
	pNFCRegs->NFCR25 = (ERR_CORRECT|BCH_ERR);
	pNFCRegs->NFCR23 |= READ_RESUME;

	return 0;
}

int nfc_BCH_read_8kpage(struct nand_chip *nand, unsigned int start, unsigned int maddr, unsigned int len)
{
	unsigned int row = 0, col = 0;/*, j;*/
	int rc = -1, err = 0;

	/* First we calculate the starting page */
	row = start >> nand->page_shift;
	/* Get raw starting column */
	col = start & (nand->oobblock - 1);
	if (pNFCRegs->NFCR9&2) {
		if (nand->oobsize >= 218)
			col = nand->oobblock + 0xA0 + ((nand->oobblock < 8192) ? 0 : 218);
		else
			col = (nand->oobblock/512)*8 + nand->oobblock;
		printf("col=0x%x, row = 0x%x\r\n", col, row);
	} else {
		if (nfc_dma_cfg(maddr, len/2, 0))
			return -ERR_DMA_CFG;
		}
	/* addr */
	NanD_Address(nand, ADDR_COLUMN_PAGE, col, row);
	/* set command 1 cycle */
	WRITE_NAND_COMMAND(NAND_READ0);
	/*for (j = 0; j < 0x50; j += 4)
	printf("XDCR%x ~ XDCR%x = 0x%8.8x 0x%8.8x 0x%8.8x 0x%8.8x\r\n",
	j, j+3,
	*(unsigned int *)(&pNFCRegs->NFCR0+j+0),
	*(unsigned int *)(&pNFCRegs->NFCR0+j+1),
	*(unsigned int *)(&pNFCRegs->NFCR0+j+2),
	*(unsigned int *)(&pNFCRegs->NFCR0+j+3));*/
	
	pNFCRegs->NFCRb |= B2R; /* write to clear */
	pNFCRegs->NFCR1 = DPAHSE_DISABLE|((nand->row+nand->col+1)<<1)|NFC_TRIGGER;
	if (nand->oobblock > 512) {
		if (NFC_WAIT_CMD_READY())
			return -ERR_NFC_CMD;
		WRITE_NAND_COMMAND(NAND_READ_CONFIRM);
		pNFCRegs->NFCRb |= B2R; /* write to clear */
		pNFCRegs->NFCR1 = NAND2NFC|1<<1|NFC_TRIGGER;
	}

	if (pNFCRegs->NFCR15 & USE_SW_ECC) {
		if (NFC_WAIT_READY())
			return -ERR_NFC_READY;
	} else {
		row = (nand->oobblock/1024)+1;
		for (col = 0; col < row; col++) {
			rc = NFC_CHECK_ECC();
			if (rc < 0)
				return -ERR_NFC_READY;
			 else {
				if (rc)
					err = nfc_BCH_ecc_check(maddr);
			}
		}
	}
	if (!(pNFCRegs->NFCR9&2)) {
		rc = nand_pdma_handler();
		nand_free_pdma();
		/*if (rc)
			return -rc;*/
	}
	if (NAND_WAIT_READY())
			return -ERR_NAND_IDLE;

	/*-----------------read again start-------------------------*/
	if (!(pNFCRegs->NFCR9&2)) {
		start = start + 4096 + 218;
		row = start >> nand->page_shift;
		/* Get raw starting column */
		col = start & (nand->oobblock - 1);
		if (nfc_dma_cfg(maddr+4096, len/2, 0))
			return -ERR_DMA_CFG;
		/* addr */
		NanD_Address(nand, ADDR_COLUMN_PAGE, col, row);
		/* set command 1 cycle */
		WRITE_NAND_COMMAND(NAND_READ0);
		/*for (j = 0; j < 0x50; j += 4)
		printf("XDCR%x ~ XDCR%x = 0x%8.8x 0x%8.8x 0x%8.8x 0x%8.8x\r\n",
		j, j+3,
		*(unsigned int *)(&pNFCRegs->NFCR0+j+0),
		*(unsigned int *)(&pNFCRegs->NFCR0+j+1),
		*(unsigned int *)(&pNFCRegs->NFCR0+j+2),
		*(unsigned int *)(&pNFCRegs->NFCR0+j+3));*/
		
		pNFCRegs->NFCRb |= B2R; /* write to clear */
		pNFCRegs->NFCR1 = DPAHSE_DISABLE|((nand->row+nand->col+1)<<1)|NFC_TRIGGER;
		if (nand->oobblock > 512) {
			if (NFC_WAIT_CMD_READY())
				return -ERR_NFC_CMD;
			WRITE_NAND_COMMAND(NAND_READ_CONFIRM);
			pNFCRegs->NFCRb |= B2R; /* write to clear */
			pNFCRegs->NFCR1 = NAND2NFC|1<<1|NFC_TRIGGER;
		}
	
		row = (nand->oobblock/1024)+1;
		for (col = 0; col < row; col++) {
			rc = NFC_CHECK_ECC();
			if (rc < 0)
				return -ERR_NFC_READY;
			 else {
				if (rc)
					err = nfc_BCH_ecc_check(maddr+4096);
			}
		}
		rc = nand_pdma_handler();
		nand_free_pdma();
	
		if (NAND_WAIT_READY())
			return -ERR_NAND_IDLE;
	}
/*----------------------read again end------------------------*/

	if (pNFCRegs->NFCR9&2)
		memcpy((unsigned char *)maddr, (unsigned char *)&pNFCRegs->FIFO[0], 64);

	if (err)
			return err;
	return 0;
}

int nfc_BCH_read_page(struct nand_chip *nand, unsigned int start, unsigned int maddr, unsigned int len)
{
	unsigned int row = 0, col = 0;/*, j;*/
	int rc = -1, err = 0;

	/* First we calculate the starting page */
	row = start >> nand->page_shift;
	/* Get raw starting column */
	col = start & (nand->oobblock - 1);
	if (pNFCRegs->NFCR9&2) {
		if (nand->oobsize >= 218)
			col = nand->oobblock + 0xA0 + ((nand->oobblock < 8192) ? 0 : 218);
		else
			col = (nand->oobblock/512)*8 + nand->oobblock;
		printf("col=0x%x, row = 0x%x\r\n", col, row);
	} else {
		if (nfc_dma_cfg(maddr, len, 0))
			return -ERR_DMA_CFG;
		}
	/* addr */
	NanD_Address(nand, ADDR_COLUMN_PAGE, col, row);
	/* set command 1 cycle */
	WRITE_NAND_COMMAND(NAND_READ0);
	/*for (j = 0; j < 0x50; j += 4)
	printf("XDCR%x ~ XDCR%x = 0x%8.8x 0x%8.8x 0x%8.8x 0x%8.8x\r\n",
	j, j+3,
	*(unsigned int *)(&pNFCRegs->NFCR0+j+0),
	*(unsigned int *)(&pNFCRegs->NFCR0+j+1),
	*(unsigned int *)(&pNFCRegs->NFCR0+j+2),
	*(unsigned int *)(&pNFCRegs->NFCR0+j+3));*/
	
	pNFCRegs->NFCRb |= B2R; /* write to clear */
	pNFCRegs->NFCR1 = DPAHSE_DISABLE|((nand->row+nand->col+1)<<1)|NFC_TRIGGER;
	if (nand->oobblock > 512) {
		if (NFC_WAIT_CMD_READY())
			return -ERR_NFC_CMD;
		WRITE_NAND_COMMAND(NAND_READ_CONFIRM);
		pNFCRegs->NFCRb |= B2R; /* write to clear */
		pNFCRegs->NFCR1 = NAND2NFC|1<<1|NFC_TRIGGER;
	}

	if (pNFCRegs->NFCR15 & USE_SW_ECC) {
		if (NFC_WAIT_READY())
			return -ERR_NFC_READY;
	} else {
		row = (nand->oobblock/512)+1;
		for (col = 0; col < row; col++) {
			rc = NFC_CHECK_ECC();
			if (rc < 0)
				return -ERR_NFC_READY;
			 else {
				if (rc)
					err = nfc_BCH_ecc_check(maddr);
			}
		}
	}
	if (!(pNFCRegs->NFCR9&2)) {
		rc = nand_pdma_handler();
		nand_free_pdma();
		/*if (rc)
			return -rc;*/
	}
	if (NAND_WAIT_READY())
			return -ERR_NAND_IDLE;

	if (pNFCRegs->NFCR9&2)
		memcpy((unsigned char *)maddr, (unsigned char *)&pNFCRegs->FIFO[0], 64);

	if (err)
			return err;
	return 0;
}

int nand_read_status(unsigned char cmd)
{
	int cfg = 0, status = -1;

	pNFCRegs->NFCR2 = cmd;
	cfg = DPAHSE_DISABLE|NFC2NAND|(1<<1);
	pNFCRegs->NFCRb |= B2R; /* write to clear */
	pNFCRegs->NFCR1 = cfg|NFC_TRIGGER;
	if (NFC_WAIT_READY())
		return -ERR_NFC_READY;
	pNFCRegs->NFCRb |= B2R; /* write to clear */
	cfg = SING_RW|NAND2NFC;
	pNFCRegs->NFCR1 = cfg | NFC_TRIGGER;
	if (NFC_WAIT_READY())
		return -ERR_NFC_READY;
	if (NAND_WAIT_IDLE())
		return -ERR_NAND_IDLE;
	status = pNFCRegs->NFCR0 & 0xff;

	return status;
}

int tellme_badblock(struct nand_chip *nand)
{
	unsigned int naddr, count = 0;
	NAND_ENABLE_CE(nand);
	if (check_block_table(nand, 0)) {
		NAND_DISABLE_CE(nand);
		return -1;
	}
	if (nand->isbadblock == isbbtbadblock)
		printf("Using BBT to search bad blocks\n");
	else {
		printf("Unknow Table tpye\n");
		NAND_DISABLE_CE(nand);
		return -1;
	}
	for (naddr = 0; naddr < nand->dwBlockCount; naddr++) {
		if (nand->isbadblock(nand, naddr*nand->erasesize)) {
			printf("block%d is bad\n", naddr);
			count++;
		}
	}
	printf("Total %d Bad Block\n", count);
	NAND_DISABLE_CE(nand);
	return 0;
}

int tellme_whereistable(struct nand_chip *nand)
{
	unsigned int i;
	unsigned int cnt = 0;
	NAND_ENABLE_CE(nand);
	if (check_block_table(nand, 0)) {
		NAND_DISABLE_CE(nand);
		return -1;
	}
	for (i = 0; i < 6; i++) {
		if (bad_block_pos[i]) {
			cnt++;
			if ((bad_block_pos[i] & 0xffff) == 1)
				printf("Find Major BBT Table ");
			else
				printf("Find Minor BBT Table ");
			printf("at block%d\n", (bad_block_pos[i]>>16)&0xffff);
		}
	}
	printf("Total %d Tables\n", cnt);
	NAND_DISABLE_CE(nand);
	return 0;
}

int tellme_nandinfo(struct nand_chip *nand)
{
	int rc = -1;

	if (g_WMTNFCBASE != __NFC_BASE) {
		rc = nand_probe(__NFC_BASE);
		if (!rc) {
			printf("Init Flash Failed rc=%d\r\n", rc);
			return rc;
		}
	}
	printf("NAND information : ");
	switch (nand->mfr) {
	case NAND_HYNIX:
	case NAND_HYNIX_new:
		printf("current CHIP structure addr 0x%x NAND_HYNIX\n", (int)nand);
		break;
	case NAND_SAMSUNG:
		printf("current CHIP structure addr 0x%x NAND_SAMSUNG\n", (int)nand);
		break;
	case NAND_TOSHIBA:
		printf("current CHIP structure addr 0x%x NAND_TOSHIBA\n", (int)nand);
		break;
	default:
		printf(" UNKNOW ( id = %x )\n", nand->mfr);
		return -1;
	}
	printf("page size = %d , Spare size = %d, %d pages per block\n",
	nand->oobblock, nand->oobsize, nand->dwPageCount);
	printf("column cycle = %d, row cycle = %d\n", nand->col, nand->row);
	printf("Erase size = 0x%x, Total Blocks = %d\n", nand->erasesize, nand->dwBlockCount);
	printf("page_shift = %d, chips_name = %s\n", nand->page_shift, nand->chips_name);
	printf("bus16 = %d, IO_ADDR = 0x%x\n", nand->bus16, nand->IO_ADDR);
	printf("numchips = %d\n", nand->numchips);
  return 0;
}

int nand_read_page(struct nand_chip *nand, unsigned int start, unsigned int maddr, unsigned int len)
{
	unsigned int page = 0, col = 0;
	int rc = -1;

	/* First we calculate the starting page */
	page = start >> nand->page_shift;
	/* Get raw starting column */
	col = start & (nand->oobblock - 1);

	if (pNFCRegs->NFCR9&2) {
		col = nand->oobblock;
		printf("1 bit ecc col=0x%x, row = 0x%x\r\n", col, page);
	} else {
		if (nfc_dma_cfg(maddr, len, 0))
			return -ERR_DMA_CFG;
	}
	/* addr */
	NanD_Address(nand, ADDR_COLUMN_PAGE, col, page);
	/* set command 1 cycle */
	WRITE_NAND_COMMAND(NAND_READ0);
	pNFCRegs->NFCRb |= B2R; /* write to clear */
	if (nand->oobblock > 512)
		pNFCRegs->NFCR1 = DPAHSE_DISABLE|((nand->row+nand->col+1)<<1)|NFC_TRIGGER;
	else
		pNFCRegs->NFCR1 = NAND2NFC|((nand->row+nand->col+1)<<1)|NFC_TRIGGER;
	if (nand->oobblock > 512) {
		if (NFC_WAIT_CMD_READY())
			return -ERR_NFC_CMD;
		WRITE_NAND_COMMAND(NAND_READ_CONFIRM);
		pNFCRegs->NFCRb |= B2R; /* write to clear */
		pNFCRegs->NFCR1 = NAND2NFC|1<<1|NFC_TRIGGER;
	}
	if (NFC_WAIT_READY())
			return -ERR_NFC_READY;

	if (!(pNFCRegs->NFCR9&2)) {
		rc = nand_pdma_handler();
		nand_free_pdma();
		if (rc)
			return -rc;
	}

	if (NAND_WAIT_READY())
		return -ERR_NAND_IDLE;

	if (pNFCRegs->NFCR9&2)
		memcpy((unsigned char *)maddr, (unsigned char *)&pNFCRegs->FIFO[0], 64);

	return 0;
}

int wmt_nand_erase(struct nand_chip *nand, unsigned int start)
{
	unsigned int page = 0, col = 0;

	/* First we calculate the starting page */
	page = start >> nand->page_shift;
	col = start & (nand->oobblock - 1);
	/* addr */
	NanD_Address(nand, ADDR_PAGE, col, page);

	WRITE_NAND_COMMAND(NAND_ERASE_SET);
	/*  trigger */
	pNFCRegs->NFCR1 = DPAHSE_DISABLE|((nand->row+1)<<1)|NFC_TRIGGER;
	/*for (j = 0; j < 0x2A; j += 4)
		printf("NFCR%x ~ NFCR%x = 0x%8.8x 0x%8.8x 0x%8.8x 0x%8.8x\r\n",
		j, j+3,
		*(unsigned int *)(&pNFCRegs->NFCR0+j+0),
		*(unsigned int *)(&pNFCRegs->NFCR0+j+1),
		*(unsigned int *)(&pNFCRegs->NFCR0+j+2),
		*(unsigned int *)(&pNFCRegs->NFCR0+j+3));*/
	if (NFC_WAIT_CMD_READY())/* kevin:wait cmd ready excluding data phase  */
		return -ERR_NFC_CMD;
	if (NAND_WAIT_IDLE())
		return -ERR_NAND_IDLE;
	WRITE_NAND_COMMAND(NAND_ERASE_CONFIRM);
	pNFCRegs->NFCRb |= B2R; /* write to clear */
	pNFCRegs->NFCR1 = DPAHSE_DISABLE|(1<<1)|NFC_TRIGGER;
	if (NAND_WAIT_READY())
		return -ERR_NAND_READY;
	if (NAND_WAIT_IDLE())
		return -ERR_NAND_IDLE;
	/* status */
	return nand_read_status(NAND_STATUS);
}

int nand_page_program(struct nand_chip *nand, unsigned int start, unsigned int maddr, unsigned int len)
{
	unsigned int row = 0, col = 0;

	/* First we calculate the starting page */
	row = start >> nand->page_shift;
	col = start & (nand->oobblock - 1);

	if (nfc_dma_cfg(maddr, len, 1))
		return -ERR_DMA_CFG;

	/* reset nand	 */
	WRITE_NAND_COMMAND(NAND_READ0);
	pNFCRegs->NFCR1 = DPAHSE_DISABLE|(1<<1)|NFC_TRIGGER;
	if (NFC_WAIT_READY())
		return -ERR_NFC_READY;

	NanD_Address(nand, ADDR_COLUMN_PAGE, col, row);
	WRITE_NAND_COMMAND(NAND_SEQIN);
	/*  trigger */
	pNFCRegs->NFCR1 = ((nand->row+nand->col+1)<<1)|NFC_TRIGGER;/* command 1 cycle  */
	if (NFC_WAIT_READY())/* wait command &data completed */
		return -ERR_NFC_READY;
	/* while (!(pNFCRegs->NFCR1d&NFC_IDLE));  */
	if (NAND_WAIT_IDLE())
		return -ERR_NAND_IDLE;
	WRITE_NAND_COMMAND(NAND_PAGEPROG);
	pNFCRegs->NFCRb |= B2R; /* write to clear */
	pNFCRegs->NFCR1 = DPAHSE_DISABLE|(1<<1)|NFC_TRIGGER;/* command 2 cycle */
	if (NAND_WAIT_READY())
		return -ERR_NAND_READY;
	if (NAND_WAIT_IDLE())
		return -ERR_NAND_IDLE;

	/* status */
	return nand_read_status(NAND_STATUS);
}
int nand_page_program_random(struct nand_chip *nand, unsigned int start, unsigned int maddr, unsigned int len)
{
	unsigned int row = 0, col = 0, start2;
	int status = -ERR_DMA_CFG;

	/* First we calculate the starting page */
	row = start >> nand->page_shift;
	col = start & (nand->oobblock - 1);
	start2 = start + 4096 + 218;

	if (nfc_dma_cfg(maddr, len/2, 1))
		return -ERR_DMA_CFG;

	/* reset nand	 */
	WRITE_NAND_COMMAND(NAND_READ0);
	pNFCRegs->NFCR1 = DPAHSE_DISABLE|(1<<1)|NFC_TRIGGER;
	if (NFC_WAIT_READY())
		return -ERR_NFC_READY;

	NanD_Address(nand, ADDR_COLUMN_PAGE, col, row);
	WRITE_NAND_COMMAND(NAND_SEQIN);
	/*  trigger */
	pNFCRegs->NFCR1 = ((nand->row+nand->col+1)<<1)|NFC_TRIGGER;/* command 1 cycle  */
	if (NFC_WAIT_READY())/* wait command &data completed */
		return -ERR_NFC_READY;
	/* while (!(pNFCRegs->NFCR1d&NFC_IDLE));  */
	if (NAND_WAIT_IDLE())
		return -ERR_NAND_IDLE;
	status = nand_pdma_handler();
	if (status)
		printf("err : pdma status=%d \n", status);//return status;
	nand_free_pdma();
	
	if (nfc_dma_cfg(maddr + (len/2), len/2, 1))
		return -ERR_DMA_CFG;
/*printf("phase 2 maddr = 0x%x, len/2=0x%x \n", maddr, len/2);*/
	row = start2 >> nand->page_shift;
	col = start2 & (nand->oobblock - 1);
/*printf("phase 3 row = 0x%x, col=0x%x \n", row, col);*/
	NanD_Address(nand, ADDR_COLUMN_PAGE, col, row);
	WRITE_NAND_COMMAND(NAND_CMD_RANDOM_DATA_IN);
	pNFCRegs->NFCR1 = ((nand->col+1)<<1)|NFC_TRIGGER;/* command 2 cycle */
	if (NFC_WAIT_READY())/* wait command &data completed */
		return -ERR_NFC_READY;
	if (NAND_WAIT_IDLE())
		return -ERR_NAND_IDLE;

	WRITE_NAND_COMMAND(NAND_PAGEPROG);
	pNFCRegs->NFCRb |= B2R; /* write to clear */
	pNFCRegs->NFCR1 = DPAHSE_DISABLE|(1<<1)|NFC_TRIGGER;/* command 2 cycle */
	if (NAND_WAIT_READY())
		return -ERR_NAND_READY;
	if (NAND_WAIT_IDLE())
		return -ERR_NAND_IDLE;

	/* status */
	return nand_read_status(NAND_STATUS);
}

int WMTEraseNANDALL(struct nand_chip *nand, unsigned int all)
{
	unsigned int naddr = 0;
	int rc = -1;

	NAND_ENABLE_CE(nand);
	if (g_WMTNFCBASE != __NFC_BASE) {
		rc = nand_probe(__NFC_BASE);
		if (!rc) {
			printf("Init Flash Failed rc=%d\r\n", rc);
			return -1;
		}
	}
	if (all) {
		for (naddr = 0; naddr < nand->dwBlockCount; naddr++)
			rc = wmt_nand_erase(nand, naddr*nand->erasesize);
	} else {
		for (naddr = (nand->dwBlockCount-BBT_MAX_BLOCK); naddr < nand->dwBlockCount; naddr++)
			rc = wmt_nand_erase(nand, naddr*nand->erasesize);
	}
	bbt = NULL;
	NAND_DISABLE_CE(nand);
	return 0;
}
int WMTEraseNAND(struct nand_chip *nand, unsigned int start, unsigned int nums, unsigned int all)
{
	unsigned int naddr, cnt1 = 0, cnt = 0;
	int rc = -1;
	unsigned int pos, end, need = 0;

	NAND_ENABLE_CE(nand);
	if (check_block_table(nand, 0)) {
		NAND_DISABLE_CE(nand);
		return -1;
	}
	pos = nand->dwBlockCount - BBT_MAX_BLOCK;
	end = start + nums;
	if (all) {
		start = 0;
		end = nand->dwBlockCount;
	}
	if (end >= pos)
		end = pos;
	printf("Erase start at block%d end at block%d , bad pos block%d\n", start, end-1, pos);
	need = 0;
	for (naddr = start; naddr < end; naddr++) {
		if (!nand->isbadblock(nand, naddr*nand->erasesize)) {
			rc = wmt_nand_erase(nand, naddr*nand->erasesize);
			if (rc < 0 || rc & 0x01) {
				cnt1++;
				nand->update_table_inram(nand, naddr*nand->erasesize);
				printf("erase fail uddate table block%d \n", naddr);
				need = 1;
			} else
				cnt++;
		}
	}
	if (need) {
		printf("update bbt to nand ...... \n");
		rc = nand->update_table_inflash(nand, 0);
		if (rc) {
			printf("failed\n");
			return rc;
		}
		printf("success\n");
	}
	printf("Erase success %d failed %d\n", cnt, cnt1);
	NAND_DISABLE_CE(nand);
	return 0;
}

int nand_write_block(struct nand_chip *nand, unsigned int start, unsigned int naddr, unsigned int size, unsigned char *oob)
{
	unsigned int i = 0, page = 0;
	unsigned int addr = start;
	int rc = -1;

	if (oob) {
		if (nand->oobsize == 128) {
			pNFCRegs->NFCRd |= 0x08;
			for (i = 0; i < 64; i++)
				pNFCRegs->FIFO[i] = oob[64+i];
			pNFCRegs->NFCRd &= 0x07;
			for (i = 0; i < 64; i++)
				pNFCRegs->FIFO[i] = oob[i];
		} else if (nand->oobsize > 64) {
			for (i = 0; i < 64; i++)
				pNFCRegs->FIFO[i] = oob[i];
		} else {
			for (i = 0; i < nand->oobsize; i++)
				pNFCRegs->FIFO[i] = oob[i];
		}
	} else {
		if (nand->oobsize == 128) {
			pNFCRegs->NFCRd |= 0x08;
			for (i = 0; i < 64; i++)
				pNFCRegs->FIFO[i] = 0xff;
			pNFCRegs->NFCRd &= 0x07;
			for (i = 0; i < 64; i++)
				pNFCRegs->FIFO[i] = 0xff;
		} else if (nand->oobsize > 64) {
			for (i = 0; i < 64; i++)
				pNFCRegs->FIFO[i] = 0xff;
		} else {
			for (i = 0; i < nand->oobsize; i++)
				pNFCRegs->FIFO[i] = 0xff;
		}
	}

	page = size/(nand->oobblock);
	/* printf("need %d pages\r\n",size); 	 */
	for (i = 0; i < page; i++) {
		/* memcpy((unsigned int *)g_EootpVirDMAbuf,(unsigned int *)addr,nand->oobblock); */
		if (nand->oobblock < 8192)
			rc = nand_page_program(nand, naddr, addr, nand->oobblock);
		else
			rc = nand_page_program_random(nand, naddr, addr, nand->oobblock);
		if (rc < 0 || rc & 0x01)
			return rc;
		/* printf("write to %x page%d\n",naddr,(naddr>>nand->page_shift)&0x3f); */
		addr = addr + nand->oobblock;
		naddr = naddr + (1<<nand->page_shift);
	}
	size = size&(nand->oobblock-1);
	if (size) {
		/* memcpy((unsigned int *)g_EootpVirDMAbuf,(unsigned int *)addr,size); */
		if (nand->oobblock < 8192)
			rc = nand_page_program(nand, naddr, addr, nand->oobblock);
		else
			rc = nand_page_program_random(nand, naddr, addr, nand->oobblock);
		if (rc < 0 || rc & 0x01)
			return rc;
	}

	return 0;
}

int nand_read_block(struct nand_chip *nand, unsigned int start, unsigned int naddr, unsigned int size)
{
	unsigned int i = 0, page = 0;
	unsigned int addr = start;
	int rc = -1;

	page = size/(nand->oobblock);
	for (i = 0; i < page; i++) {
	//*(volatile unsigned int *)0xd81100d0 &= ~0x80;
 // *(volatile unsigned int *)0xd81100d0 |= 0x80; /* NANDIO15 */
  //*(volatile unsigned int *)0xd81100d0 &= ~0x80;
		rc = nand->nfc_read_page(nand, naddr, addr, nand->oobblock);
		//*(volatile unsigned int *)0xd81100d0 &= ~0x80;
  //*(volatile unsigned int *)0xd81100d0 |= 0x80; /* NANDIO15 */
  //*(volatile unsigned int *)0xd81100d0 &= ~0x80;
		if (rc) {
			printf("Err1 at %x\n", naddr);
			return rc;
		}
		addr = addr+nand->oobblock;
		naddr = naddr+(1<<nand->page_shift);
	}
	size = size&(nand->oobblock-1);
	if (size) {
		rc = nand->nfc_read_page(nand, naddr, addr, nand->oobblock);
		/*rc = nand->nfc_read_page(nand, naddr, addr, size);*/
		if (rc) {
			printf("Err2 at %x\n", naddr);
			return rc;
		}
	}

	return 0;
}

int WMTSaveImageToNAND(
	struct nand_chip *nand,
	unsigned int naddr,
	unsigned int dwImageStart,
	unsigned int dwImageLength
	)
{
	unsigned int saddr = dwImageStart;
	unsigned int ret = dwImageLength;
	unsigned int done = 0, need = 0;
	unsigned int size = 0;
	unsigned int blocksize = 0;
	int rc = -1;

	NAND_ENABLE_CE(nand);
	if (check_block_table(nand, 1)) {
		NAND_DISABLE_CE(nand);
		return -1;
	}

	if (nand->oobblock > 0x200) {
		if (nand->oobsize >= 218)
			nfc_ecc_set(USE_HW_ECC, ECC12bit, nand);
		else
			nfc_ecc_set(USE_HW_ECC, ECC4bit, nand);
	} else
		nfc_ecc_set(USE_HW_ECC, ECC1bit, nand);
	blocksize = nand->erasesize;
	size = blocksize;
	if (ret < size) {
		size = ret;
		ret = blocksize;
	}
	done = need = 0;
	while (1) {
		if (nand->isbadblock(nand, naddr)) {
			naddr = naddr+nand->erasesize;
			continue;
		}
		rc = wmt_nand_erase(nand, naddr);
		if (rc < 0 || rc & 0x01) {
			printf("Erase failed at block%d\n", naddr/(nand->erasesize));
			nand->update_table_inram(nand, naddr);
			naddr = naddr+nand->erasesize;
			need = 1;
			continue;
		}

		if (done)
			printf("last :  data form %x to block%d , ret = %x ( size = %x ) \n",
				saddr, naddr/nand->erasesize, ret, size);
		else
			printf("data from %x to block%d , ret = %x ( size = %x ) \n",
				saddr, naddr/nand->erasesize, ret, size);

		rc = nand_write_block(nand, saddr, naddr, size, NULL);
		if (rc == 0)
			saddr = saddr + blocksize;
		else {
			printf("Write block failed at block%d\n", naddr/nand->erasesize);
			nand->update_table_inram(nand, naddr);
			naddr = naddr+nand->erasesize;
			need = 1;
			continue;
		}
		if (done)
			goto finish;

		naddr = naddr+nand->erasesize;
		ret = ret-blocksize;
		if (!ret)
			goto finish;
		else if (ret <= blocksize) {
			size = ret;
			done = 1;
		}
	}
	return -1;
finish:
	if (need) {
		printf("update bbt to nand ...... ");
		rc = nand->update_table_inflash(nand, 0);
		if (rc) {
			printf("failed\n");
			NAND_DISABLE_CE(nand);
			return rc;
		}
		printf("success\n");
	}
	printf("\r\nWrite To NAND Flash OK\r\n");
	NAND_DISABLE_CE(nand);
	return 0;
}


int WMTLoadImageFormNAND(struct nand_chip *nand, unsigned int naddr, unsigned int maddr, unsigned int size)
{
	unsigned int ret;
	unsigned int done = 0, blocksize;
	int rc = -1;

	NAND_ENABLE_CE(nand);
	printf("Load Image Form NAND Flash\r\n");
	if (check_block_table(nand, 0)) {
		NAND_DISABLE_CE(nand);
		return -1;
	}

	if (nand->oobblock > 0x200) {
		if (nand->oobsize == 218)
			nfc_ecc_set(USE_HW_ECC, ECC12bit, nand);
		else
			nfc_ecc_set(USE_HW_ECC, ECC4bit, nand);
	} else
		nfc_ecc_set(USE_HW_ECC, ECC1bit, nand);
	blocksize = nand->erasesize;
	ret = size;
	size = blocksize;
	if (ret < blocksize) {
		size = ret;
		ret = blocksize;
	}
	/* printf("naddr %x \n",naddr);	 */
	while (1) {
		if (nand->isbadblock(nand, naddr)) {
			naddr = naddr+nand->erasesize;
			continue;
		}

		/*if (done)
			printf("last : data from block%d to %x , ret = %x ( size = %x ) \n",
				naddr/nand->erasesize, maddr, ret, size);
		else
			printf("data from block%d to %x , ret = %x ( size = %x ) \n",
				naddr/nand->erasesize, maddr, ret, size);*/
	//*(volatile unsigned int *)0xd8110050 |= 0x80;
  //*(volatile unsigned int *)0xd8110090 |= 0x80;
  //*(volatile unsigned int *)0xd81100d0 &= ~0x80;
  //*(volatile unsigned int *)0xd81100d0 |= 0x80; /* NANDIO15 */
  //*(volatile unsigned int *)0xd81100d0 &= ~0x80;
		rc = nand_read_block(nand, maddr, naddr, size);
		if (rc)
			return rc;
		if (done) {
			NAND_DISABLE_CE(nand);
			printf("\r\nRead NAND Flash OK\r\n");
			return 0;
		}
		maddr = maddr+blocksize;
		naddr = naddr+nand->erasesize;
		ret = ret-blocksize;
		if (!ret) {
			printf("\r\nRead NAND Flash OK\r\n");
			NAND_DISABLE_CE(nand);
			return 0;
		} else if (ret <= blocksize) {
			size = ret;
			done = 1;
		}
	}
	NAND_DISABLE_CE(nand);
	return -1;
}

int isbbtbadblock(struct nand_chip *nand, unsigned int addr)
{
#ifdef USE_BBT
	unsigned int ffword = 0x03;

	if (!bbt)
		return -1;
	addr /= nand->erasesize;
	if ((bbt[addr>>4] & (ffword<<((addr&0x0f)*2))) != (ffword<<((addr&0x0f)*2))) {
		/*printf("bbt : block%d is bad\n", addr);*/
		return 1;
	}
#endif
	return 0;
}

int update_bbt_inflash(struct nand_chip *nand, unsigned int last)
{
	unsigned int i, j, block = 0;
	int rc = -1;
	unsigned char *pattern;
	unsigned int need;
	unsigned char fifo[64];
	unsigned int ofs, ofs1;

	if (!bbt) {
		printf("bbt = NULL\n");
		return -1;
	}

	if (nand->oobblock > 0x200) {
		if (nand->oobsize >= 218)
			nfc_ecc_set(USE_HW_ECC, ECC12bit, nand);
		else
			nfc_ecc_set(USE_HW_ECC, ECC4bit, nand);
	} else
		nfc_ecc_set(USE_HW_ECC, ECC1bit, nand);

	ofs = NAND_LARGE_DWRESERVED1_OFS;
	ofs1 = NAND_LARGE_DWRESERVED2_OFS;
	if (nand->oobblock <= 0x200) {
		ofs = NAND_SMALL_DWRESERVED1_OFS;
		ofs1 = NAND_SMALL_DWRESERVED2_OFS;
	}
	/* first update to last block */
	if (last)
		block = last & 0xffff;
	else {
		bbt_version++;
		pattern = bbt_pattern;
		for (i = nand->dwBlockCount-1; i >= (nand->dwBlockCount-BBT_MAX_BLOCK); i--) {
			if (isbbtbadblock(nand, i*nand->erasesize))
				continue;
			rc = wmt_nand_erase(nand, i*nand->erasesize);
			if (rc < 0 || rc & 0x01) {
				printf("Erase failed at block%d\n", i);
				update_bbt_inram(nand, i*nand->erasesize);
				continue;
			}
			memset(fifo, 0xff, 64);
			for (j = 0; j < 4; j++) {
				fifo[ofs+j] = pattern[j];
				fifo[ofs1+j] = (unsigned char)((bbt_version>>(8*j))&0xff);
			}
			rc = nand_write_block(
				nand,
				(unsigned int)&bbt[0],
				(i*nand->erasesize),
				nand->dwBlockCount,
				fifo
			);
			if (rc)
				update_bbt_inram(nand, i*nand->erasesize);
			else{
				printf("write bbt_pattern to block%d , bbt 0x%8.8x\n",
					i, (unsigned int)&bbt[0]);
				break;
			}
		}
		block = i;
	}
	if (block <= (nand->dwBlockCount-BBT_MAX_BLOCK)) {
		printf("err : we have only %d table blocks (block=%d)\n", BBT_MAX_BLOCK, block);
		return -1;
	}
	need = 0;
	for (i = (nand->dwBlockCount-BBT_MAX_BLOCK); i < block; i++) {
		if (isbbtbadblock(nand, i*nand->erasesize))
			continue;
		rc = wmt_nand_erase(nand, i*nand->erasesize);
		if (rc < 0 || rc & 0x01) {
			update_bbt_inram(nand, i*nand->erasesize);
			printf("Erase failed at %x\n", i*nand->erasesize);
			if (!need)
				bbt_version++;
			need = 1;
		}
	}
	pattern = mirror_pattern;
	if (last) {
		if ((last>>16) - 1)
			pattern = bbt_pattern;
	}
	for (i = block-1; i >= (nand->dwBlockCount-BBT_MAX_BLOCK); i--) {
		if (isbbtbadblock(nand, i*nand->erasesize))
			continue;
		memset(fifo, 0xff, 64);
		for (j = 0; j < 4; j++) {
			fifo[ofs+j] = pattern[j];
			fifo[ofs1+j] = (unsigned char)((bbt_version>>(8*j))&0xff);
		}
		rc = nand_write_block(
			nand,
			(unsigned int)&bbt[0],
			(i*nand->erasesize),
			nand->dwBlockCount,
			fifo
		);
		if (rc) {
			update_bbt_inram(nand, (i*nand->erasesize));
			if (!need)
				bbt_version++;
			need = 1;
		} else {
			printf("write mirror_pattern to block%d bbt = 0x%8.8x\n", i, (unsigned int)&bbt[0]);
			break;
		}
	}

	return 0;
}

int update_bbt_inram(struct nand_chip *nand, unsigned int addr)
{
	unsigned int ffword = 0x03;

	addr /= nand->erasesize;
	bbt[addr>>4] &= ~(ffword<<((addr&0x0f)*2));

	return 0;
}

int creat_bbt(struct nand_chip *nand)
{
	unsigned int i, j, count = 0;
	unsigned int page[2];
	unsigned int bpos[2];
	unsigned int ffword = 0x03;
	int rc = -1;

	printf("creat_bbt\n");
	nfc_ecc_set(USE_SW_ECC, ECC1bit, nand);
	/* scan bbt first */
	bpos[0] = (nand->dwBIOffset&0xffff);
	page[0] = (nand->dwBI0Position&0xffff);
	page[0] = page[0]<<nand->page_shift;
	bpos[1] = (nand->dwBIOffset&0xffff);
	page[1] = (nand->dwBI1Position&0xffff);
	page[1] = page[1]<<nand->page_shift;
	bbt = malloc(nand->oobblock);
	if (!bbt) {
		printf("alloc bbt failed\n");
		return -1;
	}
	memset(bbt, 0xff, nand->oobblock);
	for (i = 0; i < nand->dwBlockCount; i++) {
		for (j = 0; j < 2; j++) {
			rc = nand->nfc_read_page(
				nand,
				(i*nand->erasesize)+page[j],
				(unsigned int)nand->data_buf,
				nand->oobblock
			);
			if (rc) {
				printf("scan bbt err at block%d rc=%d\n", i+page[j], rc);
				if (rc != -ERR_ECC_UNCORRECT)
					return rc;
				bbt[i>>4] &= ~(ffword<<((i&0x0f)*2));
				count++;
				printf("find bad block : block%d\n", i);
				break;
			} else {
				if (pNFCRegs->FIFO[bpos[j]] != 0xff) {
					bbt[i>>4] &= ~(ffword<<((i&0x0f)*2));
					count++;
					printf("find bad block : block%d\n", i);
					break;
				}
			}
		}
	}
	printf("Total find %d bad blocks\n", count);
	if (nand->oobblock > 0x200) {
			if (nand->oobsize >= 218)
				nfc_ecc_set(USE_HW_ECC, ECC12bit, nand);
			else
				nfc_ecc_set(USE_HW_ECC, ECC4bit, nand);
		} else
			nfc_ecc_set(USE_HW_ECC, ECC1bit, nand);
	bbt_version = 0;
	nand->update_table_inflash = update_bbt_inflash;
	nand->update_table_inram = update_bbt_inram;
	nand->isbadblock = isbbtbadblock;

	return update_bbt_inflash(nand, 0);
}

/* NanD_SelectChip: Select a given flash chip within the current floor */
#if 0
static inline int NanD_SelectChip(struct nand_chip *nand, int chip)
{
	/* Wait for it to be ready */
	return NanD_WaitReady(nand, 0);
}
#endif

int nfc_1bit_ecc_correct(struct nand_chip *nand, unsigned int start, unsigned int maddr, unsigned int len)
{
	unsigned int err = 0, bank = 0, i;
	unsigned int cp, lp = 0;
	unsigned char data = 0;
	int rc = -1;

	/* correct data area , first */
	err = pNFCRegs->NFCR20;
	if (err & 0xffffffff) {
		printf("data error err = %x\n", err);

		/* find which bank is err */
		for (i = 0; i < 32; i += 4) {
			if ((err>>i) & 0x0f) {
				err = (err>>i) & 0x0f;
				bank = i>>2;
				lp++;
			}
		}
		if (lp != 1)
			return -ERR_ECC_BANK;
		if (err & 0x0d)
			return -ERR_ECC_UNCORRECT;
		/* reset bank select */
		err = pNFCRegs->NFCR15;
		err &= 0xfffffffc;
		err |= ((bank+1)>>1);
		pNFCRegs->NFCR15 = err;
		rc = nand_read_page(nand, start, maddr, len);
		if (rc)
			return rc;
		/* correct data , now */
		if ((bank+1)%2) {
			lp = (pNFCRegs->NFCR1a & 0x1ff);
			cp = (pNFCRegs->NFCR1a>>9) & 0x07;
		} else {
			lp = (pNFCRegs->NFCR1b & 0x1ff);
			cp = (pNFCRegs->NFCR1b>>9) & 0x07;
		}
		bank = bank - 1;
		data = *(unsigned char *)(maddr+(bank*256)+lp);
		if (data & (1<<cp))
			data &= ~(1<<cp);
		else
			data |= (1<<cp);
		*(unsigned char *)(maddr+(bank*256)+lp) = data;
	}
	/* need to correct spare area ?	 */
	err = pNFCRegs->NFCR1f;
	if (err & 0x07) {
		printf(" red error \n");
		lp = (pNFCRegs->NFCR1c & 0x3f);
		cp = (pNFCRegs->NFCR1c>>8) & 0x07;
		data = pNFCRegs->FIFO[lp];
		if (data & (1<<cp))
			data &= ~(1<<cp);
		else
			data |= (1<<cp);
		pNFCRegs->FIFO[lp] = data;
	}
	return 0;
}

int nfc_1bit_read_page(struct nand_chip *nand, unsigned int start, unsigned int maddr, unsigned int len)
{
	int rc = -1;

	/* printf("nfc_1bit_read_page : addr = 0x%x (len = 0x%x) \n",start,len); */
	rc = nand_read_page(nand, start, maddr, len);
	if (rc < 0)
		return rc;
	if (pNFCRegs->NFCR1f & 0x07 || pNFCRegs->NFCR20 & 0xffffffff) {
		printf("ECC error \n");
		rc = nfc_1bit_ecc_correct(nand, start, maddr, len);
		if (rc)
			return rc;
	}

	return 0;
}

int wmt_nand_reset(void)
{
	int status = -1;

	pNFCRegs->NFCR2 = 0xff; /* set command  */
	pNFCRegs->NFCRb |= B2R; /* write to clear*/
	pNFCRegs->NFCR1 = DPAHSE_DISABLE|(1<<1)|NFC_TRIGGER;
	status = NAND_WAIT_READY();
	if (status) {
		printf("\r\n reset wait busy status = 0x%x time out\r\n", status);
		return status;
	}
	return status;
}

int nand_readID(unsigned char *id)
{
	unsigned int cfg = 0, i = 0;
	int status = -1;

	pNFCRegs->NFCR2 = NAND_READID;
	pNFCRegs->NFCR3 = 0xff00;
	cfg = DPAHSE_DISABLE|(0x02<<1);
	pNFCRegs->NFCR1 = cfg|NFC_TRIGGER;	 /* cfg & start */
	status = NFC_WAIT_READY();
	if (status)
		return status;
	cfg = NAND2NFC|SING_RW;
	for (i = 0; i < 5; i++) {
		pNFCRegs->NFCR1 = cfg|NFC_TRIGGER;
		status = NFC_WAIT_READY();
		if (status)
			return status;
		status = NAND_WAIT_IDLE();
		if (status)
			return status;
		id[i] = (unsigned char)pNFCRegs->NFCR0;
	}

	return 0;
}

void copy_filename (char *dst, char *src, int size);

/* NanD_IdentChip: Identify a given NAND chip given {floor,chip} */
static int NanD_IdentChip(struct nand_chip *nand, int floor, int chip)
{
	unsigned int mfr, id, i, value, found = 0;
	unsigned char ids[5];
	char *s = NULL, *tmp;
	int rc = -1;

	pNFCRegs->NFCR22 = 0;
	pNFCRegs->NFCRd = 0;
	NAND_ENABLE_CE(nand);  /* set pin low */
	pNFCRegs->NFCR11 = 0xfe;  /* enable chip 0  	 */
	pNFCRegs->NFCR12 = PAGE_2K|WIDTH_8|WP_DISABLE|DIRECT_MAP|OLDATA_EN|0x40;
	pNFCRegs->NFCR14 &= 0xffff0000;
	pNFCRegs->NFCR14 |= 0x3636;/* in case of margine short	 */
	/* Reset the chip */
	wmt_nand_reset();
#if 0
	if (NanD_Command(nand, NAND_CMD_RESET)) {
#ifdef NAND_DEBUG
		printf("NanD_Command (reset) for %d,%d returned true\n",
		       floor, chip);
#endif
		NAND_DISABLE_CE(nand);  /* set pin high */
		return 0;
	}

	/* Read the NAND chip ID: 1. Send ReadID command */
	if (NanD_Command(nand, NAND_CMD_READID)) {
#ifdef NAND_DEBUG
		printf("NanD_Command (ReadID) for %d,%d returned true\n",
		       floor, chip);
#endif
		NAND_DISABLE_CE(nand);  /* set pin high */
		return 0;
	}

	/* Read the NAND chip ID: 2. Send address byte zero */
	NanD_Address(nand, ADDR_COLUMN, 0);

	/* Read the manufacturer and device id codes from the device */

	mfr = READ_NAND(nand->IO_ADDR);

	id = READ_NAND(nand->IO_ADDR);
#endif
	rc = nand_readID(ids);
	if (rc) {
		printf("get id failed\n");
		return 0;
	}
	mfr = ids[0];
	id = (ids[1]<<16) + (ids[2]<<8) + ids[3];
	NAND_DISABLE_CE(nand);  /* set pin high */
#ifdef NAND_DEBUG
	printf("NanD_Command (ReadID) got %x %x\n", mfr, id);
#endif
	if (mfr == 0xff || mfr == 0) {
		/* No response - return failure */
		return 0;
	}

	/* Check it's the same as the first chip we identified.
	 * M-Systems say that any given nand_chip device should only
	 * contain _one_ type of flash part, although that's not a
	 * hardware restriction. */
	if (nand->mfr) {
		if (nand->mfr == mfr && nand->id == id) {
			return 1;	/* This is another the same the first */
		} else {
			printf("Flash chip at floor %d, chip %d is different:\n",
			       floor, chip);
		}
	}
	/*-------identify by env start-----------*/
	for (i = 0; WMT_nand_flash_ids[i].dwFlashID != 0; i++)
		if (((unsigned int)id + (mfr<<24)) == WMT_nand_flash_ids[i].dwFlashID) {
			found = 1;
			break;
		}
	if (!(*((volatile unsigned int *)(0xd8110100))&2))
		s = getenv("wmt.io.nand");
	if (s != NULL) {
		if (found == 1)
			printf("flash id has been established in flash id list\n");

		value = simple_strtoul(s, &tmp, 16);
		if ((unsigned int)((unsigned int)id + (mfr<<24)) != value) {
			printf("uboot env config id not match, env_id = 0x%x, chip_id = 0x%x\n",
			value, (unsigned int)id + (mfr<<24));
			goto list;
		}
		nand->dwFlashID = value;
		s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		nand->dwBlockCount = value;
		s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		nand->dwPageSize = value;
		s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		nand->dwSpareSize = value;
		s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		nand->dwBlockSize = value;
		s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		nand->dwAddressCycle = value;
		s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		nand->dwBI0Position = value;
		s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		nand->dwBI1Position = value;
		s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		nand->dwBIOffset = value;
		s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		nand->dwDataWidth = value;
		s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		nand->dwPageProgramLimit = value;
		s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		nand->dwSeqRowReadSupport = value;
		s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		nand->dwSeqPageProgram = value;
		s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		nand->dwNandType = value;
		/*s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		nand->dwECCBitNum = value;*/
		s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		nand->dwRWTimming = value;
		s = tmp+1;
		copy_filename(nand->cProductName, s, MAX_PRODUCT_NAME_LENGTH);

		if (nand->dwBlockCount < 1024 || nand->dwBlockCount > 8192) {
			printf("dwBlockCount = 0x%x is abnormal\n", nand->dwBlockCount);
			goto list;
		}
		if (nand->dwPageSize < 512  || nand->dwPageSize > 8192) {
			printf("dwPageSize = 0x%x is abnormal\n", nand->dwPageSize);
			goto list;
		}
		if (nand->dwBlockSize < (1024*64)  || nand->dwBlockSize > (8192*64)) {
			printf("dwBlockSize = 0x%x is abnormal\n", nand->dwBlockSize);
			goto list;
		}
		if (nand->dwAddressCycle < 3  || nand->dwAddressCycle > 5) {
			printf("dwAddressCycle = 0x%x is abnoraml\n", nand->dwAddressCycle);
			goto list;
		}
		if (nand->dwBI0Position != 0  &&
		nand->dwBI0Position > ((nand->dwBlockSize/nand->dwPageSize)-1)) {
			printf("dwBI0Position = 0x%x is abnoraml\n", nand->dwBI0Position);
			goto list;
		}
		if (nand->dwBI1Position != 0  &&
		nand->dwBI1Position > ((nand->dwBlockSize/nand->dwPageSize)-1)) {
			printf("dwBI1Position = 0x%x is abnoraml\n", nand->dwBI1Position);
			goto list;
		}
		if (nand->dwBIOffset != 0 && nand->dwBIOffset != 5) {
			printf("dwBIOffset = 0x%x is abnoraml\n", nand->dwBIOffset);
			goto list;
		}
		if (nand->dwDataWidth != 0/* && nand->dwDataWidth != 1*/) {
			printf("dwDataWidth = 0x%x is abnoraml\n", nand->dwDataWidth);
			goto list;
		}
		/*printf("dwFlashID = 0x%x\n", nand->dwFlashID);
		printf("dwBlockCount = 0x%x\n", nand->dwBlockCount);
		printf("dwPageSize = 0x%x\n", nand->dwPageSize);
		printf("dwSpareSize = 0x%x\n", nand->dwSpareSize);
		printf("dwBlockSize = 0x%x\n", nand->dwBlockSize);
		printf("dwAddressCycle = 0x%x\n", nand->dwAddressCycle);
		printf("dwBI0Position = 0x%x\n", nand->dwBI0Position);
		printf("dwBI1Position = 0x%x\n", nand->dwBI1Position);
		printf("dwBIOffset = 0x%x\n", nand->dwBIOffset);
		printf("dwDataWidth = 0x%x\n", nand->dwDataWidth);
		printf("dwPageProgramLimit = 0x%x\n", nand->dwPageProgramLimit);
		printf("dwSeqRowReadSupport = 0x%x\n", nand->dwSeqRowReadSupport);
		printf("dwSeqPageProgram = 0x%x\n", nand->dwSeqPageProgram);
		printf("dwNandType = 0x%x\n", nand->dwNandType);
		printf("dwECCBitNum = 0x%x\n", nand->dwECCBitNum);
		printf("dwRWTimming = 0x%x\n", nand->dwRWTimming);
		printf("cProductName = %s\n", nand->cProductName);*/

		nand->mfr = mfr;
		nand->id = id;
		nand->chipshift = shift_bit(nand->dwBlockCount) + shift_bit(nand->dwBlockSize);
		nand->page256 = 0;/*nand_flash_ids[i].page256;*/
		nand->eccsize = 512;//256;
		if (nand->page256) {
			nand->oobblock = 256;
			nand->oobsize = 8;
			nand->page_shift = 8;
		} else {
			nand->oobblock = nand->dwPageSize;//512;
			nand->oobsize = nand->dwSpareSize;//16;
			nand->page_shift = shift_bit(nand->dwPageSize);//9;
		}
		nand->erasesize  = nand->dwBlockSize;//nand_flash_ids[i].erasesize;
		nand->chips_name = nand->cProductName;
		nand->bus16	 = nand->dwDataWidth;
		nand->dwPageCount = nand->dwBlockSize/nand->dwPageSize;
		nand->IO_ADDR = __NFC_BASE;
		wmt_get_timing(nand, (unsigned int)nand->dwRWTimming);
		/*-------------move from cmd_nandrw.c below-----------------*/
		pNFCRegs->NFCR12 = WP_DISABLE | DIRECT_MAP | 0x40;

		switch (nand->oobblock) {
		case 512:
			pNFCRegs->NFCR12 |= PAGE_512;
			nand->col = 1;
			break;
		case 2048:
			pNFCRegs->NFCR12 |= PAGE_2K;
			nand->col = 2;
			break;
		case 4096:
		case 8192:
			pNFCRegs->NFCR12 |= PAGE_4K;
			nand->col = 2;
			break;
		default:
			return 0;
		}
		nand->row = nand->dwAddressCycle - nand->col;
		nand->pageadrlen = nand->row;
		switch (nand->dwPageCount) {
		case 32:
		case 64:
			pNFCRegs->NFCR17 |= nand->dwPageCount;
			break;
		case 128:
			pNFCRegs->NFCR17 |= 0x60;
			break;
		case 256:
			pNFCRegs->NFCR17 |= 0x80;
			break;
		case 512:
			pNFCRegs->NFCR17 |= 0xA0;
			break;
		default:
			return 0;
		}
		if (nand->bus16)
			pNFCRegs->NFCR12 |= WIDTH_16;
		else
			pNFCRegs->NFCR12 |= WIDTH_8;
	
		if (nand->oobblock > 512) {
			if (nand->oobblock >= 4096 && nand->oobsize >= 218)
				nfc_ecc_set(USE_HW_ECC, ECC12bit, nand);
			else
				nfc_ecc_set(USE_HW_ECC, ECC4bit, nand);
		} else
			nfc_ecc_set(USE_HW_ECC, ECC1bit, nand);

		return 1;
	}

list:
	if (found == 0) {
		/* We haven't fully identified the chip. Print as much as we know. */
		printf("Unknown flash chip found: %2.2X %2.2X\n", mfr, id);
		return 0;
	}
	printf("env nand config fail, use default flash id list info\n");
	/*-------identify by env end----------*/

	/* Print and store the manufacturer and ID codes. */
	for (i = 0; WMT_nand_flash_ids[i].dwFlashID != 0; i++) {
		if (((unsigned int)id + (mfr<<24)) == WMT_nand_flash_ids[i].dwFlashID) {
#ifdef NAND_DEBUG
			printf("Flash chip found:\n\t Manufacturer ID: 0x%2.2X, "
			       "Chip ID: 0x%2.2X (%s)\n", mfr, id,
			       nand_flash_ids[i].ProductName);
#endif
			if (!nand->mfr) {
				memcpy(nand, &WMT_nand_flash_ids[i], sizeof(struct WMT_nand_flash_dev));
				nand->mfr = mfr;
				nand->id = id;
				nand->chipshift = shift_bit(WMT_nand_flash_ids[i].dwBlockCount) +
				    shift_bit(WMT_nand_flash_ids[i].dwBlockSize);
				nand->page256 = 0;//nand_flash_ids[i].page256;
				nand->eccsize = 512;//256;
				if (nand->page256) {
					nand->oobblock = 256;
					nand->oobsize = 8;
					nand->page_shift = 8;
				} else {
					nand->oobblock = WMT_nand_flash_ids[i].dwPageSize;//512;
					nand->oobsize = WMT_nand_flash_ids[i].dwSpareSize;//16;
					nand->page_shift = shift_bit(WMT_nand_flash_ids[i].dwPageSize);//9;
				}
				nand->erasesize  = WMT_nand_flash_ids[i].dwBlockSize;//nand_flash_ids[i].erasesize;
				nand->chips_name = WMT_nand_flash_ids[i].ProductName;
				nand->bus16	 = WMT_nand_flash_ids[i].dwDataWidth;
				nand->dwPageCount = WMT_nand_flash_ids[i].dwBlockSize/WMT_nand_flash_ids[i].dwPageSize;
				nand->IO_ADDR = __NFC_BASE;
				wmt_get_timing(nand, (unsigned int)WMT_nand_flash_ids[i].dwRWTimming);
				/*-------------move from cmd_nandrw.c below-----------------*/
				pNFCRegs->NFCR12 = WP_DISABLE | DIRECT_MAP | 0x40;

				switch (nand->oobblock) {
				case 512:
					pNFCRegs->NFCR12 |= PAGE_512;
					nand->col = 1;
					break;
				case 2048:
					pNFCRegs->NFCR12 |= PAGE_2K;
					nand->col = 2;
					break;
				case 4096:
				case 8192:
					pNFCRegs->NFCR12 |= PAGE_4K;
					nand->col = 2;
					break;
				default:
					return 0;
				}
				nand->row = WMT_nand_flash_ids[i].dwAddressCycle - nand->col;
				nand->pageadrlen = nand->row;
				switch (nand->dwPageCount) {
				case 32:
				case 64:
					pNFCRegs->NFCR17 |= nand->dwPageCount;
					break;
				case 128:
					pNFCRegs->NFCR17 |= 0x60;
					break;
				case 256:
					pNFCRegs->NFCR17 |= 0x80;
					break;
				case 512:
					pNFCRegs->NFCR17 |= 0xA0;
					break;
				default:
					return 0;
				}
				if (nand->bus16)
					pNFCRegs->NFCR12 |= WIDTH_16;
				else
					pNFCRegs->NFCR12 |= WIDTH_8;
			
				if (nand->oobblock > 512) {
					if (nand->oobblock >= 4096 && nand->oobsize >= 218)
						nfc_ecc_set(USE_HW_ECC, ECC12bit, nand);
					else
						nfc_ecc_set(USE_HW_ECC, ECC4bit, nand);
				} else
					nfc_ecc_set(USE_HW_ECC, ECC1bit, nand);
		/*----------------------------------------------*/
 				return 1;
			}
			return 0;
		}
	}


/*#ifdef NAND_DEBUG*/
	/* We haven't fully identified the chip. Print as much as we know. */
	printf("Unknown flash chip found: %2.2X %2.2X\n",
	       mfr, id);
/*#endif*/

	return 0;
}

int wmt_calc_clock(struct nand_chip *nand, unsigned int spec_clk,
unsigned int *T, unsigned int *divisor, unsigned int *Thold)
{
	unsigned int i, div1=0, div2, clk1, clk2=0, comp, T1=0, T2=0, clk, pllb;
	unsigned int tREA, tREH, Thold2, Ttmp, tADL, tWP;
	
	pllb = *(volatile unsigned int *)PMPMB_ADDR;
	printf("pllb=0x%x, spec_clk=0x%x\n", pllb, spec_clk);
	pllb = (((pllb>>16)&0xFF)+1)/((((pllb>>8)&0x3F)+1)*(1<<(pllb&7)));

	tREA = (spec_clk>>24)&0xFF;
	tREH = (spec_clk>>16)&0xFF;
	tWP  = (spec_clk>>8)&0xFF;
	tADL = spec_clk&0xFF;
	for (i = 1; i < 16; i++) {
		if (NFC_MAX_CLOCK >= ((pllb*WMT_PLL_CLOCK)/i)) {
			div1 = i;
			break;
		}
	}

	clk1 = (1000 * div1)/(pllb*WMT_PLL_CLOCK);
	/*printf("clk1=%d, div1=%d, spec_clk=%d\n", clk1, div1, spec_clk);*/
	for (T1 = 1; T1 < 10; T1++) {
		if ((T1*clk1) >= (tREA + NFC_SETUPTIME)) /* 8.64 = tSKEW 5.64 + tDLY 2.42 + tSETUP 0.4 */
			break;
	}
	i = 1;
	while (i*clk1 <= tREH) {
		i++;
	}
	*Thold = i;
	printf("T1=%d, clk1=%d, div1=%d, Thold=%d, tREA=%d+delay(9)\n", T1, clk1, div1, *Thold, tREA);
	Ttmp = *T = T1;
	clk = clk1;
	*divisor = div1;
	div2 = div1;
	while (Ttmp > 1 && clk != 0) {
		div2++;
		clk2 = (1000 * div2)/(pllb*WMT_PLL_CLOCK);
		comp = 0;
		for (T2 = 1; T2 < Ttmp; T2++) {
			if ((T2*clk2) >= (tREA + NFC_SETUPTIME)) { /* 8.64 = tSKEW 5.64 + tDLY 2.42 + tSETUP 0.4 */
				Ttmp = T2;
				comp = 1;
				i = 1;
				while (i*clk2 <= tREH) {
					i++;
				}
				Thold2 = i;
				printf("T2=%d, clk2=%d, div2=%d, Thold2=%d, comp=1\n", T2, clk2, div2, Thold2);
				break;
			}
		}
		if (comp == 1) {
			clk1 = clk * (*T+*Thold) * nand->oobblock;
			div1 = clk2 * (T2+Thold2) * nand->oobblock;
			printf("Tim1=%d , Tim2=%d\n", clk1, div1);
			if ((clk * (*T+*Thold) * nand->oobblock) > (clk2 * (T2+Thold2) * nand->oobblock)) {
				*T = T2;
				clk = clk2;
				*divisor = div2;
				*Thold = Thold2;
			} else {
				printf("T2 is greater and not use\n");
			}
		}
	} /* end of while */
	//printf("Tadfasdfil\n");
	i = 1;
	*Thold |= 0x100; /* set write setup/hold time */
	while (((i*2+2)*clk) <= (tADL-tWP) || (i*clk) <= (tWP+1)) {/*+1*/
		*Thold += 0x100;
		i++;
	}
	/* set write hold time */
	/* tWP > tWH*/
	/*i = 1;
	*Thold |= 0x10000;
	while (((i*2+2)*clk) <= tADL || (i*clk) < tWP) {
		*Thold += 0x10000;
		i++;
	}*/
	
	printf("T=%d, clk=%d, divisor=%d, Thold=0x%x\n", *T, clk, *divisor, *Thold);
	if ((NFC_MAX_CLOCK < (pllb*WMT_PLL_CLOCK)/(*divisor)) || clk == 0 || *T == 0 || clk > 45)
		return 1;

	return 0;
}

int wmt_get_timing(struct nand_chip *nand, unsigned int nand_timing)
{
	unsigned int T, Thold, divisor, NFC_RWTimming;	

#ifdef NFC_TIMEING_CAL
	if (wmt_calc_clock(nand, nand_timing, &T, &divisor, &Thold)) {
		printf("timming setting fail");
		return 1;
	}
	NFC_RWTimming = 0x2424;//((Thold&0xFF) << 12) + ((T + (Thold&0xFF)) << 8) +
	/* nand write timing 1T2T has bug, will cause write fail only can set 2T4T */
	//(2 << 4) + 4;/*(((Thold>>8)&0xFF) << 4) + 2*((Thold>>8)&0xFF);*/

	*(volatile unsigned long *)PMNAND_ADDR = (divisor&0x1FF);
	while ((*(volatile unsigned long *)PMCS_ADDR)&(1 << 25))
		;
	pNFCRegs->NFCR14 &=  0xffff0000;
	pNFCRegs->NFCR14 |= NFC_RWTimming;
	printf("divisor is set 0x%x, NFC_timing=0x%x\n", divisor, NFC_RWTimming);
#endif
	return 0;
}

/* NanD_ScanChips: Find all NAND chips present in a nand_chip, and identify them */

static void NanD_ScanChips(struct nand_chip *nand)
{
	int floor, chip;
	int numchips[NAND_MAX_FLOORS];
	int maxchips = NAND_MAX_CHIPS;
	int ret = 1;

	nand->numchips = 0;
	nand->mfr = 0;
	nand->id = 0;
	nand->totlen = 0;


	/* For each floor, find the number of valid chips it contains */
	for (floor = 0; floor < NAND_MAX_FLOORS; floor++) {
		ret = 1;
		numchips[floor] = 0;
		for (chip = 0; chip < maxchips && ret != 0; chip++) {

			ret = NanD_IdentChip(nand, floor, chip);
			if (ret) {
				numchips[floor]++;
				nand->numchips++;
			}
		}
	}

	/* If there are none at all that we recognise, bail */
	if (!nand->numchips) {
#ifdef NAND_DEBUG
		puts ("No NAND flash chips recognised.\n");
#endif
		return;
	}

	/* Allocate an array to hold the information for each chip */
	nand->chips = malloc(sizeof(struct Nand) * nand->numchips);
	if (!nand->chips) {
		puts ("No memory for allocating chip info structures\n");
		return;
	}

	ret = 0;

	/* Fill out the chip array with {floor, chipno} for each
	 * detected chip in the device. */
	for (floor = 0; floor < NAND_MAX_FLOORS; floor++) {
		for (chip = 0; chip < numchips[floor]; chip++) {
			nand->chips[ret].floor = floor;
			nand->chips[ret].chip = chip;
			nand->chips[ret].curadr = 0;
			nand->chips[ret].curmode = 0x50;
			ret++;
		}
	}

	/* Calculate and print the total size of the device */
	nand->totlen = nand->numchips * ((long long)1 << nand->chipshift);
	//nand->totlen = (long long)nand->dwBlockCount * (long long)nand->dwBlockSize;

#ifdef NAND_DEBUG
	printf("%d flash chips found. Total nand_chip size: %ld MB\n",
	       nand->numchips, nand->totlen >> 20);
#endif
}

/* we need to be fast here, 1 us per read translates to 1 second per meg */
#if 0
static void NanD_ReadBuf (struct nand_chip *nand, u_char * data_buf, int cntr)
{
	unsigned long nandptr = nand->IO_ADDR;

	//NanD_Command (nand, NAND_CMD_READ0);

	if (nand->bus16) {
		u16 val;

		while (cntr >= 16) {
			val = READ_NAND (nandptr);
			*data_buf++ = val & 0xff;
			*data_buf++ = val >> 8;
			val = READ_NAND (nandptr);
			*data_buf++ = val & 0xff;
			*data_buf++ = val >> 8;
			val = READ_NAND (nandptr);
			*data_buf++ = val & 0xff;
			*data_buf++ = val >> 8;
			val = READ_NAND (nandptr);
			*data_buf++ = val & 0xff;
			*data_buf++ = val >> 8;
			val = READ_NAND (nandptr);
			*data_buf++ = val & 0xff;
			*data_buf++ = val >> 8;
			val = READ_NAND (nandptr);
			*data_buf++ = val & 0xff;
			*data_buf++ = val >> 8;
			val = READ_NAND (nandptr);
			*data_buf++ = val & 0xff;
			*data_buf++ = val >> 8;
			val = READ_NAND (nandptr);
			*data_buf++ = val & 0xff;
			*data_buf++ = val >> 8;
			cntr -= 16;
		}

		while (cntr > 0) {
			val = READ_NAND (nandptr);
			*data_buf++ = val & 0xff;
			*data_buf++ = val >> 8;
			cntr -= 2;
		}
	} else {
		while (cntr >= 16) {
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			*data_buf++ = READ_NAND (nandptr);
			cntr -= 16;
		}

		while (cntr > 0) {
			*data_buf++ = READ_NAND (nandptr);
			cntr--;
		}
	}
}
#endif
/*
 * NAND read with ECC
 */
static int nand_read_ecc(struct nand_chip *nand, size_t start, size_t len,
		 size_t * retlen, u_char *buf, u_char *ecc_code)
{
	int col, page;
	int ecc_status = 0;
#ifdef CONFIG_MTD_NAND_ECC
	int j;
	int ecc_failed = 0;
	u_char *data_poi;
	/*u_char ecc_calc[6];*/
#endif

	/* Do not allow reads past end of device */
	if ((start + len) > nand->totlen) {
		printf ("%s: Attempt read beyond end of device %x %x %x\n",
			__FUNCTION__, (uint) start, (uint) len, (uint) nand->totlen);
		*retlen = 0;
		return -1;
	}

	/* First we calculate the starting page */
	/*page = shr(start, nand->page_shift);*/
	page = start >> nand->page_shift;

	/* Get raw starting column */
	col = start & (nand->oobblock - 1);

	/* Initialize return value */
	*retlen = 0;

	/* Select the NAND device */
	NAND_ENABLE_CE(nand);  /* set pin low */

	/* Loop until all data read */
	while (*retlen < len) {

#ifdef CONFIG_MTD_NAND_ECC
		/* Do we have this page in cache ? */
		if (nand->cache_page == page)
			goto readdata;
		/* Send the read command */
		//NanD_Command(nand, NAND_CMD_READ0);
		/*if (nand->bus16) {
 			NanD_Address(nand, ADDR_COLUMN_PAGE,
				     (page << nand->page_shift) + (col >> 1));
		} else {
 			NanD_Address(nand, ADDR_COLUMN_PAGE,
				     (page << nand->page_shift) + col);
		}*/

		/* Read in a page + oob data */
		/*NanD_ReadBuf(nand, nand->data_buf, nand->oobblock + nand->oobsize);*/
		j = nand->nfc_read_page(nand, (page<<nand->page_shift), (unsigned int)nand->data_buf, nand->oobblock);
		if (j)
			ecc_failed++;
		/* copy data into cache, for read out of cache and if ecc fails */
		if (nand->data_cache) {
			memcpy (nand->data_cache, nand->data_buf,
				nand->oobblock + nand->oobsize);
		}

		/* Pick the ECC bytes out of the oob data */
		for (j = 0; j < 6; j++) {
			ecc_code[j] = nand->data_buf[(nand->oobblock + oob_config.ecc_pos[j])];
		}

		/* Calculate the ECC and verify it */
		/* If block was not written with ECC, skip ECC */
		#if 0
		if (oob_config.eccvalid_pos != -1 &&
		    (nand->data_buf[nand->oobblock + oob_config.eccvalid_pos] & 0x0f) != 0x0f) {

			nand_calculate_ecc (&nand->data_buf[0], &ecc_calc[0]);
			switch (nand_correct_data (&nand->data_buf[0], &ecc_code[0], &ecc_calc[0])) {
			case -1:
				printf ("%s: Failed ECC read, page 0x%08x\n", __FUNCTION__, page);
				ecc_failed++;
				break;
			case 1:
			case 2:	/* transfer ECC corrected data to cache */
				if (nand->data_cache)
					memcpy (nand->data_cache, nand->data_buf, 256);
				break;
			}
		}

		if (oob_config.eccvalid_pos != -1 &&
		    nand->oobblock == 512 && (nand->data_buf[nand->oobblock + oob_config.eccvalid_pos] & 0xf0) != 0xf0) {

			nand_calculate_ecc (&nand->data_buf[256], &ecc_calc[3]);
			switch (nand_correct_data (&nand->data_buf[256], &ecc_code[3], &ecc_calc[3])) {
			case -1:
				printf ("%s: Failed ECC read, page 0x%08x\n", __FUNCTION__, page);
				ecc_failed++;
				break;
			case 1:
			case 2:	/* transfer ECC corrected data to cache */
				if (nand->data_cache)
					memcpy (&nand->data_cache[256], &nand->data_buf[256], 256);
				break;
			}
		}
		#endif
readdata:
		/* Read the data from ECC data buffer into return buffer */
		data_poi = (nand->data_cache) ? nand->data_cache : nand->data_buf;
		data_poi += col;
		if ((*retlen + (nand->oobblock - col)) >= len) {
			memcpy (buf + *retlen, data_poi, len - *retlen);
			*retlen = len;
		} else {
			memcpy (buf + *retlen, data_poi,  nand->oobblock - col);
			*retlen += nand->oobblock - col;
		}
		/* Set cache page address, invalidate, if ecc_failed */
		nand->cache_page = (nand->data_cache && !ecc_failed) ? page : -1;

		ecc_status += ecc_failed;
		ecc_failed = 0;
/*#else*/
#if 0
		/* Send the read command */
		//NanD_Command(nand, NAND_CMD_READ0);
		if (nand->bus16) {
			NanD_Address(nand, ADDR_COLUMN_PAGE,
				     (page << nand->page_shift) + (col >> 1));
		} else {
			NanD_Address(nand, ADDR_COLUMN_PAGE,
				     (page << nand->page_shift) + col);
		}

		/* Read the data directly into the return buffer */
		if ((*retlen + (nand->oobblock - col)) >= len) {
			NanD_ReadBuf(nand, buf + *retlen, len - *retlen);
			*retlen = len;
			/* We're done */
			continue;
		} else {
			NanD_ReadBuf(nand, buf + *retlen, nand->oobblock - col);
			*retlen += nand->oobblock - col;
			}
#endif
#endif
		/* For subsequent reads align to page boundary. */
		col = 0;
		/* Increment page address */
		page++;
	}

	/* De-select the NAND device */
	NAND_DISABLE_CE(nand);  /* set pin high */

	/*
	 * Return success, if no ECC failures, else -EIO
	 * fs driver will take care of that, because
	 * retlen == desired len and result == -EIO
	 */
	return ecc_status ? -1 : 0;
}

/*
 *	Nand_page_program function is used for write and writev !
 */
#if 0
static int nand_write_page (struct nand_chip *nand,
			    int page, int col, int last, u_char * ecc_code)
{

	int i;
	unsigned long nandptr = nand->IO_ADDR;

#ifdef CONFIG_MTD_NAND_ECC
#ifdef CONFIG_MTD_NAND_VERIFY_WRITE
	int ecc_bytes = (nand->oobblock == 512) ? 6 : 3;
#endif
#endif
	/* pad oob area */
	for (i = nand->oobblock; i < nand->oobblock + nand->oobsize; i++)
		nand->data_buf[i] = 0xff;

#ifdef CONFIG_MTD_NAND_ECC
	/* Zero out the ECC array */
	for (i = 0; i < 6; i++)
		ecc_code[i] = 0x00;

	/* Read back previous written data, if col > 0 */
	if (col) {
		NanD_Command (nand, NAND_CMD_READ0);
		if (nand->bus16) {
			NanD_Address (nand, ADDR_COLUMN_PAGE,
				      (page << nand->page_shift) + (col >> 1));
		} else {
			NanD_Address (nand, ADDR_COLUMN_PAGE,
				      (page << nand->page_shift) + col);
		}

		if (nand->bus16) {
			u16 val;

			for (i = 0; i < col; i += 2) {
				val = READ_NAND (nandptr);
				nand->data_buf[i] = val & 0xff;
				nand->data_buf[i + 1] = val >> 8;
			}
		} else {
			for (i = 0; i < col; i++)
				nand->data_buf[i] = READ_NAND (nandptr);
		}
	}

	/* Calculate and write the ECC if we have enough data */
	if ((col < nand->eccsize) && (last >= nand->eccsize)) {
		nand_calculate_ecc (&nand->data_buf[0], &(ecc_code[0]));
		for (i = 0; i < 3; i++) {
			nand->data_buf[(nand->oobblock +
					oob_config.ecc_pos[i])] = ecc_code[i];
		}
		if (oob_config.eccvalid_pos != -1) {
			nand->data_buf[nand->oobblock +
				       oob_config.eccvalid_pos] = 0xf0;
		}
	}

	/* Calculate and write the second ECC if we have enough data */
	if ((nand->oobblock == 512) && (last == nand->oobblock)) {
		nand_calculate_ecc (&nand->data_buf[256], &(ecc_code[3]));
		for (i = 3; i < 6; i++) {
			nand->data_buf[(nand->oobblock +
					oob_config.ecc_pos[i])] = ecc_code[i];
		}
		if (oob_config.eccvalid_pos != -1) {
			nand->data_buf[nand->oobblock +
				       oob_config.eccvalid_pos] &= 0x0f;
		}
	}
#endif
	/* Prepad for partial page programming !!! */
	for (i = 0; i < col; i++)
		nand->data_buf[i] = 0xff;

	/* Postpad for partial page programming !!! oob is already padded */
	for (i = last; i < nand->oobblock; i++)
		nand->data_buf[i] = 0xff;

	/* Send command to begin auto page programming */
	//NanD_Command (nand, NAND_CMD_READ0);
	//NanD_Command (nand, NAND_CMD_SEQIN);
	if (nand->bus16) {
		NanD_Address (nand, ADDR_COLUMN_PAGE,
			      (page << nand->page_shift) + (col >> 1));
	} else {
		NanD_Address (nand, ADDR_COLUMN_PAGE,
			      (page << nand->page_shift) + col);
	}

	/* Write out complete page of data */
	if (nand->bus16) {
		for (i = 0; i < (nand->oobblock + nand->oobsize); i += 2) {
			WRITE_NAND (nand->data_buf[i] +
				    (nand->data_buf[i + 1] << 8),
				    nand->IO_ADDR);
		}
	} else {
		for (i = 0; i < (nand->oobblock + nand->oobsize); i++)
			WRITE_NAND (nand->data_buf[i], nand->IO_ADDR);
	}

	/* Send command to actually program the data */
	//NanD_Command (nand, NAND_CMD_PAGEPROG);
	//NanD_Command (nand, NAND_CMD_STATUS);
#ifdef NAND_NO_RB
	{
		u_char ret_val;

		do {
			ret_val = READ_NAND (nandptr);	/* wait till ready */
		} while ((ret_val & 0x40) != 0x40);
	}
#endif
	/* See if device thinks it succeeded */
	if (READ_NAND (nand->IO_ADDR) & 0x01) {
		printf ("%s: Failed write, page 0x%08x, ", __FUNCTION__,
			page);
		return -1;
	}
#ifdef CONFIG_MTD_NAND_VERIFY_WRITE
	/*
	 * The NAND device assumes that it is always writing to
	 * a cleanly erased page. Hence, it performs its internal
	 * write verification only on bits that transitioned from
	 * 1 to 0. The device does NOT verify the whole page on a
	 * byte by byte basis. It is possible that the page was
	 * not completely erased or the page is becoming unusable
	 * due to wear. The read with ECC would catch the error
	 * later when the ECC page check fails, but we would rather
	 * catch it early in the page write stage. Better to write
	 * no data than invalid data.
	 */

	/* Send command to read back the page */
	if (col < nand->eccsize)
		NanD_Command (nand, NAND_CMD_READ0);
	else
		NanD_Command (nand, NAND_CMD_READ1);
	if (nand->bus16) {
		NanD_Address (nand, ADDR_COLUMN_PAGE,
			      (page << nand->page_shift) + (col >> 1));
	} else {
		NanD_Address (nand, ADDR_COLUMN_PAGE,
			      (page << nand->page_shift) + col);
	}

	/* Loop through and verify the data */
	if (nand->bus16) {
		for (i = col; i < last; i = +2) {
			if ((nand->data_buf[i] +
			     (nand->data_buf[i + 1] << 8)) != READ_NAND (nand->IO_ADDR)) {
				printf ("%s: Failed write verify, page 0x%08x ",
					__FUNCTION__, page);
				return -1;
			}
		}
	} else {
		for (i = col; i < last; i++) {
			if (nand->data_buf[i] != READ_NAND (nand->IO_ADDR)) {
				printf ("%s: Failed write verify, page 0x%08x ",
					__FUNCTION__, page);
				return -1;
			}
		}
	}

#ifdef CONFIG_MTD_NAND_ECC
	/*
	 * We also want to check that the ECC bytes wrote
	 * correctly for the same reasons stated above.
	 */
	NanD_Command (nand, NAND_CMD_READOOB);
	if (nand->bus16) {
		NanD_Address (nand, ADDR_COLUMN_PAGE,
			      (page << nand->page_shift) + (col >> 1));
	} else {
		NanD_Address (nand, ADDR_COLUMN_PAGE,
			      (page << nand->page_shift) + col);
	}
	if (nand->bus16) {
		for (i = 0; i < nand->oobsize; i += 2) {
			u16 val;

			val = READ_NAND (nand->IO_ADDR);
			nand->data_buf[i] = val & 0xff;
			nand->data_buf[i + 1] = val >> 8;
		}
	} else {
		for (i = 0; i < nand->oobsize; i++) {
			nand->data_buf[i] = READ_NAND (nand->IO_ADDR);
		}
	}
	for (i = 0; i < ecc_bytes; i++) {
		if ((nand->data_buf[(oob_config.ecc_pos[i])] != ecc_code[i]) && ecc_code[i]) {
			printf ("%s: Failed ECC write "
				"verify, page 0x%08x, "
				"%6i bytes were succesful\n",
				__FUNCTION__, page, i);
			return -1;
		}
	}
#endif	/* CONFIG_MTD_NAND_ECC */
#endif	/* CONFIG_MTD_NAND_VERIFY_WRITE */
	return 0;
}
#endif
static int nand_write_ecc (struct nand_chip* nand, size_t to, size_t len,
			   size_t * retlen, const u_char * buf, u_char * ecc_code)
{
	int i, page, col, cnt, ret = 0;

	/* Do not allow write past end of device */
	if ((to + len) > nand->totlen) {
		printf ("%s: Attempt to write past end of page\n", __FUNCTION__);
		return -1;
	}

	/* Shift to get page */
	page = ((int) to) >> nand->page_shift;

	/* Get the starting column */
	col = to & (nand->oobblock - 1);

	/* Initialize return length value */
	*retlen = 0;

	/* Select the NAND device */
#ifdef CONFIG_OMAP1510
	archflashwp(0,0);
#endif
#ifdef CFG_NAND_WP
	NAND_WP_OFF();
#endif

    	NAND_ENABLE_CE(nand);  /* set pin low */

	/* Check the WP bit */
	/*NanD_Command(nand, NAND_CMD_STATUS);*/
	if (!(nand_read_status(NAND_STATUS) & 0x80)) {
		printf ("%s: Device is write protected!!!\n", __FUNCTION__);
		ret = -1;
		goto out;
	}

	/* Loop until all data is written */
	while (*retlen < len) {
		/* Invalidate cache, if we write to this page */
		if (nand->cache_page == page)
			nand->cache_page = -1;

		/* Write data into buffer */
		if ((col + len) >= nand->oobblock) {
			for (i = col, cnt = 0; i < nand->oobblock; i++, cnt++) {
				nand->data_buf[i] = buf[(*retlen + cnt)];
			}
		} else {
			for (i = col, cnt = 0; cnt < (len - *retlen); i++, cnt++) {
				nand->data_buf[i] = buf[(*retlen + cnt)];
			}
		}
		/* We use the same function for write and writev !) */
		/*ret = nand_write_page (nand, page, col, i, ecc_code);*/
		if (nand->oobblock < 8192)
			ret = nand_page_program(nand, (page << nand->page_shift), (unsigned int)nand->data_buf, i);
		else
			ret = nand_page_program_random(nand, (page << nand->page_shift), (unsigned int)nand->data_buf, i);
		if (ret < 0 || ret & 0x01)
			goto out;

		/* Next data start at page boundary */
		col = 0;

		/* Update written bytes count */
		*retlen += cnt;

		/* Increment page address */
		page++;
	}

	/* Return happy */
	*retlen = len;

out:
	/* De-select the NAND device */
	NAND_DISABLE_CE(nand);  /* set pin high */
#ifdef CONFIG_OMAP1510
    	archflashwp(0,1);
#endif
#ifdef CFG_NAND_WP
	NAND_WP_ON();
#endif

	return ret;
}

/* read from the 16 bytes of oob data that correspond to a 512 byte
 * page or 2 256-byte pages.
 */
static int nand_read_oob(struct nand_chip* nand, size_t ofs, size_t len,
			 size_t * retlen, u_char * buf)
{
	/*int len256 = 0;*/
	struct Nand *mychip;
	int ret = 0;

	mychip = &nand->chips[ofs >> nand->chipshift];

	/* update address for 2M x 8bit devices. OOB starts on the second */
	/* page to maintain compatibility with nand_read_ecc. */
	/*if (nand->page256) {
		if (!(ofs & 0x8))
			ofs += 0x100;
		else
			ofs -= 0x8;
	}*/

	NAND_ENABLE_CE(nand);  /* set pin low */
	/*NanD_Command(nand, NAND_CMD_READOOB);
	if (nand->bus16) {
 		NanD_Address(nand, ADDR_COLUMN_PAGE,
			     ((ofs >> nand->page_shift) << nand->page_shift) +
 				((ofs & (nand->oobblock - 1)) >> 1));
	} else {
		NanD_Address(nand, ADDR_COLUMN_PAGE, ofs);
	}*/
	if (nand->oobblock >= 2048) {
		pNFCRegs->NFCR9|= 0x2;/* read sidinfo */
		ret = nand->nfc_read_page(nand,
		ofs + (pNFCRegs->NFCR9&0x2),
		(unsigned int)&buf[0], 32);
	} else {
		ret = nand->nfc_read_page(nand, ofs,
		(unsigned int)nand->data_buf, nand->oobblock);

		memcpy(buf, (unsigned char *)&pNFCRegs->FIFO[0], 16);
	}
	
	if (ret) {
		printf("Read oob failed ret=%d at addr %x\n", ret, ofs);
		return ret;
	}
	if (nand->oobblock >= 2048)
		pNFCRegs->NFCR9 &= 0xFFFFFFFD;    /* return to default value */


	/* treat crossing 8-byte OOB data for 2M x 8bit devices */
	/* Note: datasheet says it should automaticaly wrap to the */
	/*       next OOB block, but it didn't work here. mf.      */
	/*if (nand->page256 && ofs + len > (ofs | 0x7) + 1) {
		len256 = (ofs | 0x7) + 1 - ofs;
		NanD_ReadBuf(nand, buf, len256);

		NanD_Command(nand, NAND_CMD_READOOB);
		NanD_Address(nand, ADDR_COLUMN_PAGE, ofs & (~0x1ff));
	}*/

	/*NanD_ReadBuf(nand, &buf[len256], len - len256);*/

	*retlen = len;
	/* Reading the full OOB data drops us off of the end of the page,
	 * causing the flash device to go into busy mode, so we need
	 * to wait until ready 11.4.1 and Toshiba TC58256FT nands */

	/*ret = NanD_WaitReady(nand, 1);*/
	NAND_DISABLE_CE(nand);  /* set pin high */

	return ret;
}

/* write to the 16 bytes of oob data that correspond to a 512 byte
 * page or 2 256-byte pages.
 */
#if 0
static int nand_write_oob(struct nand_chip* nand, size_t ofs, size_t len,
		  size_t * retlen, const u_char * buf)
{
	int len256 = 0;
	int i;
	unsigned long nandptr = nand->IO_ADDR;

#ifdef PSYCHO_DEBUG
	printf("nand_write_oob(%lx, %d): %2.2X %2.2X %2.2X %2.2X ... %2.2X %2.2X .. %2.2X %2.2X\n",
	       (long)ofs, len, buf[0], buf[1], buf[2], buf[3],
	       buf[8], buf[9], buf[14],buf[15]);
#endif

	NAND_ENABLE_CE(nand);  /* set pin low to enable chip */

	/* Reset the chip */
	/*NanD_Command(nand, NAND_CMD_RESET);*/

	/* issue the Read2 command to set the pointer to the Spare Data Area. */
	/*NanD_Command(nand, NAND_CMD_READOOB);
	if (nand->bus16) {
 		NanD_Address(nand, ADDR_COLUMN_PAGE,
			     ((ofs >> nand->page_shift) << nand->page_shift) +
 				((ofs & (nand->oobblock - 1)) >> 1));
	} else {
 		NanD_Address(nand, ADDR_COLUMN_PAGE, ofs);
	}*/

	/* update address for 2M x 8bit devices. OOB starts on the second */
	/* page to maintain compatibility with nand_read_ecc. */
	/*if (nand->page256) {
		if (!(ofs & 0x8))
			ofs += 0x100;
		else
			ofs -= 0x8;
	}*/

	/* issue the Serial Data In command to initial the Page Program process */
	/*NanD_Command(nand, NAND_CMD_SEQIN);
	if (nand->bus16) {
 		NanD_Address(nand, ADDR_COLUMN_PAGE,
			     ((ofs >> nand->page_shift) << nand->page_shift) +
 				((ofs & (nand->oobblock - 1)) >> 1));
	} else {
 		NanD_Address(nand, ADDR_COLUMN_PAGE, ofs);
	}*/

	/* treat crossing 8-byte OOB data for 2M x 8bit devices */
	/* Note: datasheet says it should automaticaly wrap to the */
	/*       next OOB block, but it didn't work here. mf.      */
	if (nand->page256 && ofs + len > (ofs | 0x7) + 1) {
		len256 = (ofs | 0x7) + 1 - ofs;
		for (i = 0; i < len256; i++)
			WRITE_NAND(buf[i], nandptr);

		NanD_Command(nand, NAND_CMD_PAGEPROG);
		NanD_Command(nand, NAND_CMD_STATUS);
#ifdef NAND_NO_RB
   		{ u_char ret_val;
			do {
				ret_val = READ_NAND(nandptr); /* wait till ready */
			} while ((ret_val & 0x40) != 0x40);
		}
#endif
		if (READ_NAND(nandptr) & 1) {
			puts ("Error programming oob data\n");
			/* There was an error */
			NAND_DISABLE_CE(nand);  /* set pin high */
			*retlen = 0;
			return -1;
		}
		NanD_Command(nand, NAND_CMD_SEQIN);
		NanD_Address(nand, ADDR_COLUMN_PAGE, ofs & (~0x1ff));
	}

	if (nand->bus16) {
		for (i = len256; i < len; i += 2) {
			WRITE_NAND(buf[i] + (buf[i+1] << 8), nandptr);
		}
	} else {
		for (i = len256; i < len; i++)
			WRITE_NAND(buf[i], nandptr);
	}

	NanD_Command(nand, NAND_CMD_PAGEPROG);
	NanD_Command(nand, NAND_CMD_STATUS);
#ifdef NAND_NO_RB
	{	u_char ret_val;
		do {
			ret_val = READ_NAND(nandptr); /* wait till ready */
		} while ((ret_val & 0x40) != 0x40);
	}
#endif
	if (READ_NAND(nandptr) & 1) {
		puts ("Error programming oob data\n");
		/* There was an error */
		NAND_DISABLE_CE(nand);  /* set pin high */
		*retlen = 0;
		return -1;
	}

	NAND_DISABLE_CE(nand);  /* set pin high */
	*retlen = len;
	return 0;

}
#endif
int nand_erase(struct nand_chip* nand, size_t ofs, size_t len, int clean)
{
	/* This is defined as a structure so it will work on any system
	 * using native endian jffs2 (the default).
	 */
/*	static struct jffs2_unknown_node clean_marker = {
		JFFS2_MAGIC_BITMASK,
		JFFS2_NODETYPE_CLEANMARKER,
		8		*//* 8 bytes in this node */
/*	};*/
	unsigned long nandptr;
	struct Nand *mychip;
	int ret = 0;
	if ((ofs & (nand->erasesize-1)) || (len & (nand->erasesize-1))) {
		printf ("Offset and size must be sector aligned, ofs = 0x%x, erasesize = %d len=0x%x\n",
			(int)ofs, (int) nand->erasesize, (int) len);
		return -1;
	}

	nandptr = nand->IO_ADDR;

	/* Select the NAND device */
#ifdef CONFIG_OMAP1510
	archflashwp(0,0);
#endif
#ifdef CFG_NAND_WP
	NAND_WP_OFF();
#endif
    NAND_ENABLE_CE(nand);  /* set pin low */

	/* Check the WP bit */
	/*NanD_Command(nand, NAND_CMD_STATUS);*/
	if (!(nand_read_status(NAND_STATUS) & 0x80)) {
		printf ("nand_write_ecc: Device is write protected!!!\n");
		ret = -1;
		goto out;
	}

	/* Check the WP bit */
	/*NanD_Command(nand, NAND_CMD_STATUS);*/
	if (!(nand_read_status(NAND_STATUS) & 0x80)) {
		printf ("%s: Device is write protected!!!\n", __FUNCTION__);
		ret = -1;
		goto out;
	}
	if (check_block_table(nand, 0)) {
			NAND_DISABLE_CE(nand);  /* set pin high */
			return -1;
		}
	/* FIXME: Do nand in the background. Use timers or schedule_task() */
	while(len) {
		/*mychip = &nand->chips[shr(ofs, nand->chipshift)];*/
		mychip = &nand->chips[ofs >> nand->chipshift];

		/* always check for bad block first, genuine bad blocks
		 * should _never_  be erased.
		 */
		/*if (ALLOW_ERASE_BAD_DEBUG || !check_block(nand, ofs)) {*/
		if (ALLOW_ERASE_BAD_DEBUG || !nand->isbadblock(nand, ofs)) {
			/* Select the NAND device */
			NAND_ENABLE_CE(nand);  /* set pin low */

			//WMTEraseNAND(ofs/nand->erasesize, len/nand->erasesize, 0);
			ret = wmt_nand_erase(nand, ofs);
			/*printf ("erase finish ofs = 0x%x, ret = 0x%x len=0x%x\n",
			(int)ofs, (int) ret, (int) len);*/
/*		NanD_Command(nand, NAND_CMD_ERASE1);
			NanD_Address(nand, ADDR_PAGE, ofs);
			NanD_Command(nand, NAND_CMD_ERASE2);

			NanD_Command(nand, NAND_CMD_STATUS);
*/
#ifdef NAND_NO_RB
			{	u_char ret_val;
				do {
					ret_val = READ_NAND(nandptr); /* wait till ready */
				} while ((ret_val & 0x40) != 0x40);
			}
#endif
			if (ret < 0 || (ret&1)) {
				printf ("%s: Error erasing at 0x%lx\n",
					__FUNCTION__, (long)ofs);
				/* There was an error */
				ret = -1;
				goto out;
			} else
				ret = 0;
			if (clean) {
				/*int n;*/	/* return value not used */
				int p, l;

				/* clean marker position and size depend
				 * on the page size, since 256 byte pages
				 * only have 8 bytes of oob data
				 */
				if (nand->page256) {
					p = NAND_JFFS2_OOB8_FSDAPOS;
					l = NAND_JFFS2_OOB8_FSDALEN;
				} else {
					p = NAND_JFFS2_OOB16_FSDAPOS;
					l = NAND_JFFS2_OOB16_FSDALEN;
				}

				/*ret = nand_write_oob(nand, ofs + p, l, (size_t *)&n,
						     (u_char *)&clean_marker);*/
				/* quit here if write failed */
				/*if (ret)
					goto out;*/
			}
		}
		ofs += nand->erasesize;
		len -= nand->erasesize;
	}

out:
	/* De-select the NAND device */
	NAND_DISABLE_CE(nand);  /* set pin high */
#ifdef CONFIG_OMAP1510
    	archflashwp(0,1);
#endif
#ifdef CFG_NAND_WP
	NAND_WP_ON();
#endif

	return ret;
}

static inline int nandcheck(unsigned long potential, unsigned long physadr)
{
	return 0;
}

int get_pattern_small(struct nand_chip *nand, unsigned int block, unsigned int *tag, unsigned int *version)
{
	unsigned int pos, pos1;
	unsigned char *buf;
	int rc = -1;
	buf = nand->data_buf;

	rc = nand->nfc_read_page(nand, (block*nand->erasesize), (unsigned int)buf,	nand->oobblock);
	if (rc) {
		printf("Read Tag failed rc=%d at %x\n", rc, (block*nand->erasesize));
		return rc;
	}
	pos = NAND_SMALL_DWRESERVED1_OFS;
	pos1 = NAND_SMALL_DWRESERVED2_OFS;
	*tag = (pNFCRegs->FIFO[pos] << 24)|
	(pNFCRegs->FIFO[pos+1] << 16)|
	(pNFCRegs->FIFO[pos+2] << 8)|
	pNFCRegs->FIFO[pos+3];
	*version = pNFCRegs->FIFO[pos1]/*|
		(pNFCRegs->FIFO[pos1+1] << 8)|
		(pNFCRegs->FIFO[pos1+2] << 16)|
		(pNFCRegs->FIFO[pos1+3] << 24)*/;

	return 0;
}

int get_pattern_large(struct nand_chip *nand, unsigned int block, unsigned int *tag, unsigned int *version)
{
	unsigned int pos;
	unsigned char *buf;
	int rc = -1;

	pos = NAND_LARGE_DWRESERVED1_OFS;
	buf = nand->data_buf;
	pNFCRegs->NFCR9|= 0x2;/* read sidinfo */
	rc = nand->nfc_read_page(nand, (block*nand->erasesize), (unsigned int)buf, 32);
	if (rc) {
		printf("Read Tag failed rc=%d at %x\n", rc, (block*nand->erasesize));
		pNFCRegs->NFCR9 &= 0xFFFFFFFD;    /* return to default value			 */
		return rc;
	}
	pNFCRegs->NFCR9 &= 0xFFFFFFFD;    /* return to default value			 */
	pNFCRegs->NFCR23 |= READ_RESUME;
	*tag = (buf[pos] << 24)|(buf[pos+1] << 16)|(buf[pos+2] << 8)|buf[pos+3];
	pos = NAND_LARGE_DWRESERVED2_OFS;

	*version = buf[pos]/*|(buf[pos+1] << 8)|(buf[pos+2] << 16)|(buf[pos+3] << 24)*/;
	return 0;
}

int WMT_check_pattern(struct nand_chip *nand, unsigned int block, unsigned int *type, unsigned int *version)
{
	unsigned int tag = 0;

	*type = 0;
	if (nand->oobblock > 0x200)
		get_pattern_large(nand, block, &tag, version);
	else
		get_pattern_small(nand, block, &tag, version);
	printf("block%d tag=%x ", block, tag);
	switch (tag) {
	case 0x42627430:
		*type = 1;
		printf(" version =%x\n", *version);
		return 0;
	case 0x31746242:
		*type = 2;
		printf(" version =%x\n", *version);
		return 0;
	default:
		break;
	}
	printf(" no version\n");
	return 0;
}

int find_bbt(struct nand_chip *nand)
{
	int rc = -1;
	unsigned int i, type, size = 0;
	unsigned int count = 2, first = 0;
	unsigned int version = 0xff;

	if (bbt)
		return 0;

	for (i = 0; i < 10; i++)
		bad_block_pos[i] = 0;

	nand->update_table_inflash = NULL;
	nand->update_table_inram = NULL;
	nand->isbadblock = NULL;
	for (i = nand->dwBlockCount-1; i > (nand->dwBlockCount-1-BBT_MAX_BLOCK); i--) {
		WMT_check_pattern(nand, i, &type, &version);
		if (type) {
			bad_block_pos[nand->dwBlockCount-i-1] = (type|(i<<16));
			count--;
		} else
			continue;
		if (!size) {
			if (i == (nand->dwBlockCount-BBT_MAX_BLOCK)) {
				printf("find first table at block%d , failed\n", i);
				return -1;
			}
			nand->update_table_inflash = update_bbt_inflash;
			nand->update_table_inram = update_bbt_inram;
			nand->isbadblock = isbbtbadblock;
			count = 1;
			first = (i|1<<16);
			size = nand->dwBlockCount>>2;

			bbt_version = version;
			size += (nand->oobblock-1);
			size &= ~(nand->oobblock-1);
			if (!bbt) {
				bbt = malloc(size);
				if (!bbt) {
					printf("alloc bbt failed\n");
					return -1;
				}
			}
			rc = nand_read_block(nand, (unsigned int)&bbt[0], i*nand->erasesize, size);
			if (rc)
				return rc;
		} else if (version > bbt_version) {
			bbt_version = version;
			rc = nand_read_block(nand, (unsigned int)&bbt[0],	i*nand->erasesize,	size);
			if (rc)
				return rc;
		}
		//fixme: bbt1 replace bbt0 without merge.
		if (!count)
			break;
	}
	if (!first) {
		printf("bbt table is not found\n");
		return 1;
	}
	printf("bbt table is found\n");
	return 0;
}

int check_block_table(struct nand_chip *nand, unsigned int scan)
{
	int rc = -1;

	if ((g_WMTNFCBASE != __NFC_BASE) || !bbt) {
		if (bbt) {
			printf("free bbt\n");
			free(bbt);
			bbt = NULL;
		}
		if (g_WMTNFCBASE != __NFC_BASE)
			rc = nand_probe(__NFC_BASE);
		/*if (!rc) {
			printf("Init Flash Failed rc=%d\r\n", rc);
			return -1;
		}*/
#ifdef USE_BBT
		rc = find_bbt(nand);
		/*printf("return of find_bbt failed rc = %d \n", rc);*/
		if (rc < 0) {
			printf("find_bbt failed\n");
			return -1;
		}
		if (rc > 0 && scan) {
			if (creat_bbt(nand)) {
				printf("create bbt failed\n");
				return -1;
			}
		} else if (rc > 0)
			return -2;
#endif
	}
	return 0;
}

unsigned long nand_probe(unsigned long physadr)
{
	struct nand_chip *nand = NULL;
	unsigned char *dataBuf;
	int i = 0, ChipID = 1;
	pNFCRegs = (WMT_NFC_CFG *) __NFC_BASE;
	pNand_PDma_Reg = (struct _NAND_PDMA_REG_ *) (__NFC_BASE + 0x100);
	g_WMTNFCBASE = __NFC_BASE;
#ifdef CONFIG_MTD_NAND_ECC_JFFS2
	oob_config.ecc_pos[0] = NAND_JFFS2_OOB_ECCPOS0;
	oob_config.ecc_pos[1] = NAND_JFFS2_OOB_ECCPOS1;
	oob_config.ecc_pos[2] = NAND_JFFS2_OOB_ECCPOS2;
	oob_config.ecc_pos[3] = NAND_JFFS2_OOB_ECCPOS3;
	oob_config.ecc_pos[4] = NAND_JFFS2_OOB_ECCPOS4;
	oob_config.ecc_pos[5] = NAND_JFFS2_OOB_ECCPOS5;
	oob_config.eccvalid_pos = 4;
#else
	oob_config.ecc_pos[0] = NAND_NOOB_ECCPOS0;
	oob_config.ecc_pos[1] = NAND_NOOB_ECCPOS1;
	oob_config.ecc_pos[2] = NAND_NOOB_ECCPOS2;
	oob_config.ecc_pos[3] = NAND_NOOB_ECCPOS3;
	oob_config.ecc_pos[4] = NAND_NOOB_ECCPOS4;
	oob_config.ecc_pos[5] = NAND_NOOB_ECCPOS5;
	oob_config.eccvalid_pos = NAND_NOOB_ECCVPOS;
#endif
	oob_config.badblock_pos = 5;

	for (i=0; i<CFG_MAX_NAND_DEVICE; i++) {
		if (nand_dev_desc[i].ChipID == NAND_ChipID_UNKNOWN) {
			nand = &nand_dev_desc[i];
			break;
		}
	}
	if (!nand)
		return (0);

	memset((char *)nand, 0, sizeof(struct nand_chip));
	nand->IO_ADDR = physadr;
	nand->cache_page = -1;  /* init the cache page */
	NanD_ScanChips(nand);

	if (nand->totlen == 0) {
		/* no chips found, clean up and quit */
		memset((char *)nand, 0, sizeof(struct nand_chip));
		nand->ChipID = NAND_ChipID_UNKNOWN;
		return 0;
	}

	nand->ChipID = ChipID;
	if (curr_device == -1)
		curr_device = i;

	dataBuf = malloc (nand->oobblock + nand->oobsize + 0x300);
	if (!dataBuf) {
		puts ("Cannot allocate memory for data structures.\n");
		return (0);
	}
	ReadDesc = (unsigned long *)((((unsigned int)dataBuf)&(~0x1f)) + 0x20);
	nand->data_buf = (unsigned char *)((unsigned int)ReadDesc) + 0x100;
	/*printf(" dataBuf = 0x%x\r\n", (unsigned int)dataBuf);
	printf(" ReadDesc = 0x%x\r\n", (unsigned int)ReadDesc);
	printf(" nand->data_buf = 0x%x\r\n", (unsigned int)nand->data_buf);*/

	return (nand->totlen);
}

#ifdef CONFIG_MTD_NAND_ECC
/*
 * Pre-calculated 256-way 1 byte column parity
 */
static const u_char nand_ecc_precalc_table[] = {
	0x00, 0x55, 0x56, 0x03, 0x59, 0x0c, 0x0f, 0x5a,
	0x5a, 0x0f, 0x0c, 0x59, 0x03, 0x56, 0x55, 0x00,
	0x65, 0x30, 0x33, 0x66, 0x3c, 0x69, 0x6a, 0x3f,
	0x3f, 0x6a, 0x69, 0x3c, 0x66, 0x33, 0x30, 0x65,
	0x66, 0x33, 0x30, 0x65, 0x3f, 0x6a, 0x69, 0x3c,
	0x3c, 0x69, 0x6a, 0x3f, 0x65, 0x30, 0x33, 0x66,
	0x03, 0x56, 0x55, 0x00, 0x5a, 0x0f, 0x0c, 0x59,
	0x59, 0x0c, 0x0f, 0x5a, 0x00, 0x55, 0x56, 0x03,
	0x69, 0x3c, 0x3f, 0x6a, 0x30, 0x65, 0x66, 0x33,
	0x33, 0x66, 0x65, 0x30, 0x6a, 0x3f, 0x3c, 0x69,
	0x0c, 0x59, 0x5a, 0x0f, 0x55, 0x00, 0x03, 0x56,
	0x56, 0x03, 0x00, 0x55, 0x0f, 0x5a, 0x59, 0x0c,
	0x0f, 0x5a, 0x59, 0x0c, 0x56, 0x03, 0x00, 0x55,
	0x55, 0x00, 0x03, 0x56, 0x0c, 0x59, 0x5a, 0x0f,
	0x6a, 0x3f, 0x3c, 0x69, 0x33, 0x66, 0x65, 0x30,
	0x30, 0x65, 0x66, 0x33, 0x69, 0x3c, 0x3f, 0x6a,
	0x6a, 0x3f, 0x3c, 0x69, 0x33, 0x66, 0x65, 0x30,
	0x30, 0x65, 0x66, 0x33, 0x69, 0x3c, 0x3f, 0x6a,
	0x0f, 0x5a, 0x59, 0x0c, 0x56, 0x03, 0x00, 0x55,
	0x55, 0x00, 0x03, 0x56, 0x0c, 0x59, 0x5a, 0x0f,
	0x0c, 0x59, 0x5a, 0x0f, 0x55, 0x00, 0x03, 0x56,
	0x56, 0x03, 0x00, 0x55, 0x0f, 0x5a, 0x59, 0x0c,
	0x69, 0x3c, 0x3f, 0x6a, 0x30, 0x65, 0x66, 0x33,
	0x33, 0x66, 0x65, 0x30, 0x6a, 0x3f, 0x3c, 0x69,
	0x03, 0x56, 0x55, 0x00, 0x5a, 0x0f, 0x0c, 0x59,
	0x59, 0x0c, 0x0f, 0x5a, 0x00, 0x55, 0x56, 0x03,
	0x66, 0x33, 0x30, 0x65, 0x3f, 0x6a, 0x69, 0x3c,
	0x3c, 0x69, 0x6a, 0x3f, 0x65, 0x30, 0x33, 0x66,
	0x65, 0x30, 0x33, 0x66, 0x3c, 0x69, 0x6a, 0x3f,
	0x3f, 0x6a, 0x69, 0x3c, 0x66, 0x33, 0x30, 0x65,
	0x00, 0x55, 0x56, 0x03, 0x59, 0x0c, 0x0f, 0x5a,
	0x5a, 0x0f, 0x0c, 0x59, 0x03, 0x56, 0x55, 0x00
};


/*
 * Creates non-inverted ECC code from line parity
 */
#if 0
static void nand_trans_result(u_char reg2, u_char reg3,
	u_char *ecc_code)
{
	u_char a, b, i, tmp1, tmp2;

	/* Initialize variables */
	a = b = 0x80;
	tmp1 = tmp2 = 0;

	/* Calculate first ECC byte */
	for (i = 0; i < 4; i++) {
		if (reg3 & a)		/* LP15,13,11,9 --> ecc_code[0] */
			tmp1 |= b;
		b >>= 1;
		if (reg2 & a)		/* LP14,12,10,8 --> ecc_code[0] */
			tmp1 |= b;
		b >>= 1;
		a >>= 1;
	}

	/* Calculate second ECC byte */
	b = 0x80;
	for (i = 0; i < 4; i++) {
		if (reg3 & a)		/* LP7,5,3,1 --> ecc_code[1] */
			tmp2 |= b;
		b >>= 1;
		if (reg2 & a)		/* LP6,4,2,0 --> ecc_code[1] */
			tmp2 |= b;
		b >>= 1;
		a >>= 1;
	}

	/* Store two of the ECC bytes */
	ecc_code[0] = tmp1;
	ecc_code[1] = tmp2;
}
#endif
/*
 * Calculate 3 byte ECC code for 256 byte block
 */
#if 0
static void nand_calculate_ecc (const u_char *dat, u_char *ecc_code)
{
	u_char idx, reg1, reg3;
	int j;

	/* Initialize variables */
	reg1 = reg3 = 0;
	ecc_code[0] = ecc_code[1] = ecc_code[2] = 0;

	/* Build up column parity */
	for(j = 0; j < 256; j++) {

		/* Get CP0 - CP5 from table */
		idx = nand_ecc_precalc_table[dat[j]];
		reg1 ^= idx;

		/* All bit XOR = 1 ? */
		if (idx & 0x40) {
			reg3 ^= (u_char) j;
		}
	}

	/* Create non-inverted ECC code from line parity */
	nand_trans_result((reg1 & 0x40) ? ~reg3 : reg3, reg3, ecc_code);

	/* Calculate final ECC code */
	ecc_code[0] = ~ecc_code[0];
	ecc_code[1] = ~ecc_code[1];
	ecc_code[2] = ((~reg1) << 2) | 0x03;
}

/*
 * Detect and correct a 1 bit error for 256 byte block
 */
static int nand_correct_data (u_char *dat, u_char *read_ecc, u_char *calc_ecc)
{
	u_char a, b, c, d1, d2, d3, add, bit, i;

	/* Do error detection */
	d1 = calc_ecc[0] ^ read_ecc[0];
	d2 = calc_ecc[1] ^ read_ecc[1];
	d3 = calc_ecc[2] ^ read_ecc[2];

	if ((d1 | d2 | d3) == 0) {
		/* No errors */
		return 0;
	} else {
		a = (d1 ^ (d1 >> 1)) & 0x55;
		b = (d2 ^ (d2 >> 1)) & 0x55;
		c = (d3 ^ (d3 >> 1)) & 0x54;

		/* Found and will correct single bit error in the data */
		if ((a == 0x55) && (b == 0x55) && (c == 0x54)) {
			c = 0x80;
			add = 0;
			a = 0x80;
			for (i=0; i<4; i++) {
				if (d1 & c)
					add |= a;
				c >>= 2;
				a >>= 1;
			}
			c = 0x80;
			for (i=0; i<4; i++) {
				if (d2 & c)
					add |= a;
				c >>= 2;
				a >>= 1;
			}
			bit = 0;
			b = 0x04;
			c = 0x80;
			for (i=0; i<3; i++) {
				if (d3 & c)
					bit |= b;
				c >>= 2;
				b >>= 1;
			}
			b = 0x01;
			a = dat[add];
			a ^= (b << bit);
			dat[add] = a;
			return 1;
		}
		else {
			i = 0;
			while (d1) {
				if (d1 & 0x01)
					++i;
				d1 >>= 1;
			}
			while (d2) {
				if (d2 & 0x01)
					++i;
				d2 >>= 1;
			}
			while (d3) {
				if (d3 & 0x01)
					++i;
				d3 >>= 1;
			}
			if (i == 1) {
				/* ECC Code Error Correction */
				read_ecc[0] = calc_ecc[0];
				read_ecc[1] = calc_ecc[1];
				read_ecc[2] = calc_ecc[2];
				return 2;
			}
			else {
				/* Uncorrectable Error */
				return -1;
			}
		}
	}

	/* Should never happen */
	return -1;
}
#endif /* end of #if 0 by dannier*/
#endif

#ifdef CONFIG_JFFS2_NAND

int read_jffs2_nand(size_t start, size_t len,
		    size_t * retlen, u_char * buf, int nanddev)
{
	return nand_rw(nand_dev_desc + nanddev, NANDRW_READ | NANDRW_JFFS2,
		       start, len, retlen, buf);
}

#endif /* CONFIG_JFFS2_NAND */


//#endif /* (CONFIG_COMMANDS & CFG_CMD_NAND) */
