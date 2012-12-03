/*++ 
Copyright (c) 2010 WonderMedia Technologies, Inc.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software 
Foundation, either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details. You
should have received a copy of the GNU General Public License along with this
program. If not, see http://www.gnu.org/licenses/>.

WonderMedia Technologies, Inc.
4F, 531, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/
#include <common.h>
#include <command.h>
#include <linux/ctype.h>
#include <asm/arch/common_def.h>

#include "wmt_display.h"
#include "minivgui.h"

//#define CHAR_WIDTH 8
//#define CHAR_HEIGHT 20  // 16

int text_x = 30, text_y = 30 - CHAR_HEIGHT;
// ------------------- Extern Variable ------------------------

//-------------------- Extern Function -----------------------
extern void mv_initPrimary(const mv_surface * s);
//extern void arm_memset(void * s, int c, size_t count);
extern int display_init(int on, int force);
extern struct fb_var_screeninfo vfb_var;

static int atoi(char *s) //added by howayhuo
{
    int i,n,sign;

    for(i=0;isspace(s[i]);i++) //跳过空白符
          ;
    sign=(s[i]=='-')?-1:1;
    if(s[i]=='+'||s[i]=='-')   //跳过符号
          i++;
    for(n=0;isdigit(s[i]);i++)
          n=10*n+(s[i]-'0');  //将数字字符转换成整形数字
    return sign *n;
}

int do_textout(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    int  tmpx, tmpy;
    unsigned int rgb=0;
    char * text_first, *text_last;
    char r, g, b;
    int i,len, textLen;
    mv_Rect rect;
    char tmpbuf[200];

    if(display_init(1, 0))
        return -1;

    switch (argc)
    {
        case 1:
        case 2:
        case 3:		/* use load_addr as default address */
        case 4:		/* use argument */
            printf("<ERROR> too few argument\n");
            printf ("Usage:\n%s\n", cmdtp->usage);
        return -1;

        default:
            //tmpx = simple_strtoul(argv[1], NULL, 10);
            //tmpy = simple_strtoul(argv[2], NULL, 10);
            tmpx = atoi(argv[1]);
            tmpy = atoi(argv[2]);

            //printf("tmpx=%d, tmpy=%d\n", tmpx,tmpy);

            if(tmpx >= 0)
                text_x = tmpx;

            if(tmpy >= 0)
                text_y = tmpy;
            else
                text_y = text_y + CHAR_HEIGHT;

            printf("x=%d, y=%d\n", text_y, text_y);

            rgb = simple_strtoul(argv[argc -1], NULL, 16);

            text_first = argv[3];
            if(*text_first != '"')
            {
                printf("<ERROR> please specify the text begin with \" \n", *text_first);
                printf ("Usage:\n%s\n", cmdtp->usage);

                return -1;
            }

            text_last = argv[argc-2];
            while(*text_last != '\0')
                text_last++;

            if(*--text_last != '"')
            {
                printf("<ERROR> please specify the text end with \" \n", *text_last);
                printf ("Usage:\n%s\n", cmdtp->usage);
                return -1;
            }
        break;
    }
    len = 0;
    for(i=3; i<argc-1; i++)
    {
        printf("%s ", argv[i]);
        len += sprintf(tmpbuf+len, "%s ", argv[i]);
    }
    printf("\n");
    //if you input:
    //   drawtext 0 0 "aa bb cc" ff0000
    //then the tmpbuf is ["aa bb cc" ], it have redundant space at the end.
    tmpbuf[len -2] = '\0';    //ignore the last  'double quotation marks' and 'space'

    textLen = len - 3; // total len- ' start " ' (1 char) - 'space' (1 char) -' end " ' (1 char)
    rect.left = 0;
    rect.top = text_y;
    rect.right = vfb_var.xres;
    rect.bottom = text_y + CHAR_HEIGHT;
    //mv_fillRect(&rect, mv_RGB2Color(0, 0, 0));
    mv_fillRect(&rect, 0, 0, 0);

    r = (rgb >> 16) & 0xFF;
    g = (rgb >> 8)  & 0xFF;
    b = rgb & 0xFF;
    mv_textOut(text_x, text_y, tmpbuf+1, r, g, b);

    return 0;
}

U_BOOT_CMD(
	textout,	100,	5,	do_textout,
	"textout - show text to the screen \n"
	"textout x y \"str\" color\n"
	"color is 24bit Hex, R[23:16], G[15:8], B[7:0]\n"
	"for example: textout 0 0 \"hello world\" FFFFFF\n",
	"- show text to the screen \n"
	"usage: textout x y \"str\" color\n"
	"(x,y) is the coordinate. x, y are decimal. (x,y) default value is (30,30)\n"
	"if x < 0, the x coordinate is unchaged\n"
	"if y < 0, the y coordinate auto add 22 pixels to move to next line\n"
	"color is 24bit Hex, R[23:16], G[15:8], B[7:0]\n"
	"for example: textout 0 0 \"hello world\" FFFFFF\n"
	"             textout -1 -1 \"come on\" FF0000\n"
);

