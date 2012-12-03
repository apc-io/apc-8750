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

/* For IO-IO */
#if defined(__WATCOMC__)
/* #include <conio.h> */
#endif

#if !defined(__UASSERT_H__)
#include "uassert.h"
#endif
#if !defined(__TMACRO_H__)
#include "tmacro.h"
#endif
#if !defined(__CARD_H__)
#include "card.h"
#endif
#if !defined(__MAC_H__)
#include "mac.h"
#endif

/*---------------------  Static Definitions -------------------------*/

/*---------------------  Static Classes  ----------------------------*/

/*---------------------  Static Variables  --------------------------*/

/*---------------------  Static Functions  --------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/

void GCARDvSetMediaLinkMode(DWORD dwIoBase, BYTE byRevId, UINT uConnectionType)
{
	DWORD   dwData;

	/* detect the the brand of PHY */
	GMIIbReadEmbeded(dwIoBase, byRevId, MII_REG_PHYID2, (PWORD)&dwData);
	GMIIbReadEmbeded(dwIoBase, byRevId, MII_REG_PHYID1, (PWORD)&dwData + 1);

	if ((dwData & CID_REV_ID_MASK_OFF) == CID_CICADA_CIS8201 ||
	(dwData & CID_REV_ID_MASK_OFF) == CID_CICADA_CIS3216I ||
	(dwData & CID_REV_ID_MASK_OFF) == CID_CICADA_CIS3216I64) {
		/* Patch VT3119/VT3216 speed polling bug of MII Full-Duplex mode */
		if (uConnectionType == MEDIA_100M_FULL || uConnectionType == MEDIA_10M_FULL)
			MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_TCSR, TCSR_ECHODIS);
		else
			MIIvRegBitsOff(dwIoBase, byRevId, MII_REG_TCSR, TCSR_ECHODIS);
	}

	/* clear force MAC mode bit */
	MACvRegBitsOff(dwIoBase, MAC_REG_CHIPGCR, CHIPGCR_FCMODE);

	/* if connection type is AUTO */
	if (uConnectionType == MEDIA_AUTO) {
		MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_ANAR, ANAR_100FD|ANAR_100|ANAR_10FD|ANAR_10);
		MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_GCR, GCR_1000FD|GCR_1000);
		/* enable AUTO-NEGO mode */
		MIIvSetAutoNegotiationOn(dwIoBase, byRevId);
	} else {
		/* Nway force */
		WORD    wANAR, wGTCR;

		/* In forced mode, always turn-on MAC FCMODE */
		MACvRegBitsOn(dwIoBase, MAC_REG_CHIPGCR, CHIPGCR_FCMODE);

		MIIvSetAutoNegotiationOff(dwIoBase, byRevId);

		GMIIbReadEmbeded(dwIoBase, byRevId, MII_REG_ANAR, &wANAR);
		GMIIbReadEmbeded(dwIoBase, byRevId, MII_REG_GCR, &wGTCR);

		wANAR &= (~(ANAR_100FD | ANAR_100 | ANAR_10FD | ANAR_10));
		wGTCR &= (~(GCR_1000FD | GCR_1000));

		switch (uConnectionType) {
		case MEDIA_1G_FULL:
			wGTCR |= GCR_1000FD;
			/* Turn on the forced mode of MAC */
			MACvRegBitsOn(dwIoBase, MAC_REG_CHIPGCR, CHIPGCR_FCGMII);
			MACvRegBitsOn(dwIoBase, MAC_REG_CHIPGCR, CHIPGCR_FCFDX);
			break;
		case MEDIA_1G_HALF:
			wGTCR |= GCR_1000;
			/* Turn on the forced mode of MAC */
			MACvRegBitsOn(dwIoBase, MAC_REG_CHIPGCR, CHIPGCR_FCGMII);
			MACvRegBitsOff(dwIoBase, MAC_REG_CHIPGCR, CHIPGCR_FCFDX);
			break;
		case MEDIA_100M_FULL:
			wANAR |= ANAR_100FD;
			/* Turn on the forced mode of MAC */
			MACvRegBitsOff(dwIoBase, MAC_REG_CHIPGCR, CHIPGCR_FCGMII);
			MACvRegBitsOn(dwIoBase, MAC_REG_CHIPGCR, CHIPGCR_FCFDX);
			break;
		case MEDIA_100M_HALF:
			wANAR |= ANAR_100;
			/* Turn on the forced mode of MAC */
			MACvRegBitsOff(dwIoBase, MAC_REG_CHIPGCR, CHIPGCR_FCGMII);
			MACvRegBitsOff(dwIoBase, MAC_REG_CHIPGCR, CHIPGCR_FCFDX);
			break;
		case MEDIA_10M_FULL:
			wANAR |= ANAR_10FD;
			/* Turn on the forced mode of MAC */
			MACvRegBitsOff(dwIoBase, MAC_REG_CHIPGCR, CHIPGCR_FCGMII);
			MACvRegBitsOn(dwIoBase, MAC_REG_CHIPGCR, CHIPGCR_FCFDX);
			break;
		case MEDIA_10M_HALF:
			wANAR |= ANAR_10;
			/* Turn on the forced mode of MAC */
			MACvRegBitsOff(dwIoBase, MAC_REG_CHIPGCR, CHIPGCR_FCGMII);
			MACvRegBitsOff(dwIoBase, MAC_REG_CHIPGCR, CHIPGCR_FCFDX);
			break;
		default:
			DBG_ASSERT(FALSE);
			break;
		}

		GMIIbWriteEmbeded(dwIoBase, byRevId, MII_REG_ANAR, wANAR);
		GMIIbWriteEmbeded(dwIoBase, byRevId, MII_REG_GCR, wGTCR);

		/* enable AUTO-NEGO mode */
		MIIvSetAutoNegotiationOn(dwIoBase, byRevId);

	} /* end if */
}

void GCARDvSetLoopbackMode(DWORD dwIoBase, BYTE byRevId, WORD wLoopbackMode)
{
	switch (wLoopbackMode) {
	case CARD_LB_NONE:
	case CARD_LB_MAC:
	case CARD_LB_MII:
		break;
	default:
		DBG_ASSERT(FALSE);
		break;
	}

	/* set MAC loopback */
	GMACvSetLoopbackMode(dwIoBase, LOBYTE(wLoopbackMode));
	/* set MII loopback */
	GMIIvSetLoopbackMode(dwIoBase, byRevId, HIBYTE(wLoopbackMode));
}

BOOL GCARDbSoftwareReset(DWORD dwIoBase, BYTE byRevId)
{
	/* reset MAC */
	if (!GMACbSafeSoftwareReset(dwIoBase, byRevId))
		return FALSE;
	/* reset PHY */
	if (!GMIIbSafeSoftwareReset(dwIoBase, byRevId))
		return FALSE;

	return TRUE;
}
