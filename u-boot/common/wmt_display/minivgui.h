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

/**
 * A mini gui implementation (copied some code from VGUI)
 * functions:
 * 1. draw line (horz or vert only)
 * 2. draw rectangle, fill rectangle,
 * 3. draw bitmap ( Windows bmp format)
 * 4. draw text ( note, ascii only and the text font was gererated by
 * vgui's FontGen 1.0 tool)
 *
 * Usage:
 *  first maybe you want to change this line to fit your rgb format
	#define mv_RGB_FORMAT mv_RGB_FORMAT_565
 *  in the source code firstly you need to prepare all HW related jobs,
 *  such as turn on panel backlight/init LCDC
 *  after that the first function you need to invoked is mv_initPrimary.
 *  You need to set the correct member in mv_surface argument.
 *  After that you can call all functions
 *
 *  Here is a example:
 *
 *
	mv_surface s;
        s.width = 800;
        s.height = 480;
	s.lineBytes = 800 * 2;
		// if the panel is 16bit
	s.startAddr = 0x7C000000;
	// the frame buffer address

	mv_initPrimary(&s);

        mv_Rect r;
	r.left = 0;
	r.right = s.width;
	r.top = 0;
	r.bottom = s.height;
	//fill whole screen with red color
	mv_fillRect(&r, mv_RGB2Color(255, 0, 0));
		
	//draw some text on the screen (build-in font size is 12x24)
	mv_textOut(0, 0, "Hello,world!", mv_RGB2Color(0, 0, 0));
	mv_textOut(0, 25, "Hello, mini VGUI!", mv_RGB2Color(0, 0, 0));
 *
 */
#ifndef MINIVGUI_H_INCLUDED
#define MINIVGUI_H_INCLUDED

#ifdef  __cplusplus
extern "C" {
#endif

#include "wmt_display.h"


/**
 * init minivgui primary screen. this is the first function need be called
 */
void mv_initPrimary(const mv_surface * s);

void mv_drawLine(int x1, int y1, int x2, int y2, unsigned int color);
void mv_drawRect(const mv_Rect* rect, unsigned int color);
void mv_fillRect(const mv_Rect* rect, unsigned char r, unsigned char g, unsigned char b);


/**
 * draw text on the screen (Note: only acsii supported)
 */
void mv_textOut(int x, int y, const char * string, unsigned char r, unsigned char g, unsigned char b);
int mv_loadBmp(unsigned char* fileBuffer);



#ifdef  __cplusplus
}
#endif

#endif // MINIVGUI_H_INCLUDED


