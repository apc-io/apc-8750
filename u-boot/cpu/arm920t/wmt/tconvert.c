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

/* #include <string.h> */
/* #include <stdlib.h> */

#if !defined(__UASSERT_H__)
#include "uassert.h"
#endif
#if !defined(__TASCII_H__)
#include "tascii.h"
#endif
#if !defined(__TCONVERT_H__)
#include "tconvert.h"
#endif

/*---------------------  Static Definitions -------------------------*/

/*---------------------  Static Classes  ----------------------------*/

/*---------------------  Static Variables  --------------------------*/

/*---------------------  Static Functions  --------------------------*/

/*---------------------  Export Variables  --------------------------*/

void CVTvBy6ToStr12(PBYTE pbyEtherAddr, const PTSTR pszResult)
{
	int     ii;
	UINT    uDigit;
	PBYTE   pbyResult = (PBYTE)pszResult;

	for (ii = 0; ii < 6; ii++) {
		/* each time we convert one byte to two digits */

		/* the 16^^1 portion */
		uDigit = pbyEtherAddr[ii] / 16;

		if (uDigit < 10)
			*pbyResult = (BYTE)(MC_0 + uDigit);
		else
			*pbyResult = (BYTE)(MC_A + uDigit - 10);
		pbyResult++;

		/* the 16^^0 portion */
		uDigit = pbyEtherAddr[ii] % 16;
		if (uDigit < 10)
			*pbyResult = (BYTE)(MC_0 + uDigit);
		else
			*pbyResult = (BYTE)(MC_A + uDigit - 10);
		pbyResult++;
	}
	*pbyResult = 0;
}

void CVTvStr12ToBy6(const PCTSTR pszSrc, PBYTE pbyEtherAddr)
{
	int     ii;
	PBYTE   pbySrc = (PBYTE)pszSrc;

	/* before conversion, make it all uppercase */
	/* strupr((PTSTR)pbySrc); */

	for (ii = 0; ii < 6; ii++) {
		/* each time we convert two digits to one byte */

		/* the 16^^1 portion */
		if (MC_0 <= *pbySrc && *pbySrc <= MC_9)
			pbyEtherAddr[ii] = (BYTE)((*pbySrc - MC_0) * 16);
		else
			pbyEtherAddr[ii] = (BYTE)((*pbySrc - MC_A + 10) * 16);
		pbySrc++;

		/* the 16^^0 portion */
		if (MC_0 <= *pbySrc && *pbySrc <= MC_9)
			pbyEtherAddr[ii] += (BYTE)(*pbySrc - MC_0);
		else
			pbyEtherAddr[ii] += (BYTE)(*pbySrc - MC_A + 10);
		pbySrc++;
	}
}

/*
 * Description:
 *
 * Parameters:
 *
 */
void CVTvHexToAsc(PBYTE pbyValue, const PTSTR pszResult, UINT cbByte)
{
	int     ii;
	UINT    uDigit;
	PBYTE   pbyResult = (PBYTE)pszResult;


	for (ii = cbByte - 1; ii >= 0; ii--) {
		/* each time we convert one byte to two digits */

		/* the 16^^1 portion */
		uDigit = pbyValue[ii] / 16;

		if (uDigit < 10)
			*pbyResult = (BYTE)(MC_0 + uDigit);
		else
			*pbyResult = (BYTE)(MC_A + uDigit - 10);
		pbyResult++;

		/* the 16^^0 portion */
		uDigit = pbyValue[ii] % 16;
		if (uDigit < 10)
			*pbyResult = (BYTE)(MC_0 + uDigit);
		else
			*pbyResult = (BYTE)(MC_A + uDigit - 10);
		pbyResult++;
	}
	*pbyResult = 0;
}

/*
 * Description:
 *
 * Parameters:
 *
 */
void CVTvAscToHex(const PCTSTR pszSrc, PBYTE pbyValue, UINT cbByte)
{
	int     ii;
	PBYTE   pbySrc = (PBYTE)pszSrc;
	PBYTE   pbyHi2Lo;
	UINT    cbDigit;
	UINT    cbAscByte;

	/* before conversion, make it all uppercase */
	/* strupr((PTSTR)pbySrc); */
	/* conversion from highest byte to lowerest byte */
	pbyHi2Lo = pbyValue + cbByte - 1;

	cbDigit = _tcslen((PTSTR)pbySrc);

	/* # of bytes is (# of hex digits + 1) / 2 */
	cbAscByte = (cbDigit + 1) / 2;

	/* pre padding zero */
	if (cbByte > cbAscByte) {
		for (ii = 0; ii < (cbByte - cbAscByte); ii++) {
			*pbyHi2Lo = 0;
			pbyHi2Lo--;
		}
	}	else {
		/* if # of bytes of ascii string is greater than cbByte, */
		/* only cbByte will be done */
		cbAscByte = cbByte;
	}

	/* pre padding half byte */
	if ((cbDigit % 2) != 0) {
		if (MC_0 <= *pbySrc && *pbySrc <= MC_9)
			*pbyHi2Lo = (BYTE)(*pbySrc - MC_0);
		else
			*pbyHi2Lo = (BYTE)(*pbySrc - MC_A + 10);
		pbySrc++;
		pbyHi2Lo--;
		/* # of bytes we want to do should minus 1 */
		cbAscByte--;
	}

	for (ii = 0; (ii < cbAscByte) && (pbySrc != _T('\0')); ii++) {
		if (MC_0 <= *pbySrc && *pbySrc <= MC_9)
			*pbyHi2Lo = (BYTE)((*pbySrc - MC_0) * 16);
		else
			*pbyHi2Lo = (BYTE)((*pbySrc - MC_A + 10) * 16);
		pbySrc++;

		if (MC_0 <= *pbySrc && *pbySrc <= MC_9)
			*pbyHi2Lo += (BYTE)(*pbySrc - MC_0);
		else
			*pbyHi2Lo += (BYTE)(*pbySrc - MC_A + 10);
		pbySrc++;
		pbyHi2Lo--;
	}
}

void CVTvIP4ToStr15(PBYTE pbyIPAddr, const PTSTR pszResult)
{
	int     ii;
	BYTE    uNetAddr;
	TCHAR   IPStr[4];
	PTSTR   pbyResult = pszResult;

	for (ii = 0; ii < 4; ii++) {
		uNetAddr = pbyIPAddr[ii];

		if (uNetAddr < 0x0A) {
			/* pad two '0' if lower than 10 */
			strncpy(pbyResult, "00", 2);
			strncpy(pbyResult+2, IPStr, 1);
		} else if (uNetAddr < 0x064) {
			/* pad '0' if lower than 100 */
			strncpy(pbyResult, "0", 1);
			strncpy(pbyResult+1, IPStr, 2);
		} else {
		/* if > 100 , no padding needs to be performed */
		strncpy(pbyResult, IPStr, 3);
		}

		pbyResult += 3;
		if (ii == 3)
			break;
		*pbyResult = '.';
		pbyResult++;
	}

	*pbyResult = '\0';
}

void CVTvStr15ToIP4(const PCTSTR pszSrc, PBYTE pbyIPAddr)
{
	int     ii, jj;
	PBYTE   pbySrc = (PBYTE)pszSrc;
	TCHAR   IPStr[16];
	PBYTE   pbyIPStr = (PBYTE)IPStr;

	ii = 0;
	while (TRUE) {
		/* reset digit counter */
		jj = 0;
		pbyIPStr = (PBYTE)IPStr;
		while ((*pbySrc != '.') && (*pbySrc != '\0')) {
			*pbyIPStr = *pbySrc;
			pbyIPStr++;
			pbySrc++;
			jj++;
		}

		*pbyIPStr = '\0';
		if (jj > 3)
			/* more than 3 digits */
			pbyIPAddr[ii] = 0xFF;
		else
			/* pbyIPAddr[ii]=(BYTE)atoi(IPStr); */

			if (*pbySrc == '\0')
				break;
		ii++;
		pbySrc++;     /* skip "." */
	}
}
