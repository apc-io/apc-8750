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

#ifndef __TCONVERT_H__
#define __TCONVERT_H__

#if !defined(__TTYPE_H__)
#include "ttype.h"
#endif
#if !defined(__TTCHAR_H__)
#include "ttchar.h"
#endif

/*---------------------  Export Definitions -------------------------*/

/*---------------------  Export Types  ------------------------------*/

/*---------------------  Export Macros ------------------------------*/

/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/
#ifdef __cplusplus
extern "C" {                            /* Assume C declarations for C++ */
#endif /* __cplusplus */

void CVTvBy6ToStr12(PBYTE pbyEtherAddr, const PTSTR pszResult);
void CVTvStr12ToBy6(const PCTSTR pszSrc, PBYTE pbyEtherAddr);
void CVTvHexToAsc(PBYTE pbyValue, const PTSTR pszResult, UINT cbByte);
void CVTvAscToHex(const PCTSTR pszSrc, PBYTE pbyValue, UINT cbByte);
void CVTvIP4ToStr15(PBYTE pbyIPAddr, const PTSTR pszResult);
void CVTvStr15ToIP4(const PCTSTR pszSrc, PBYTE pbyIPAddr);

#ifdef __cplusplus
}                                       /* End of extern "C" { */
#endif /* __cplusplus */

#endif /* __TCONVERT_H__ */
