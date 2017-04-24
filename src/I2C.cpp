/*
 * I2C.cpp
 *
 *  Created on: 21.2.2016
 *      Author: krl
 *      Based on example provided by NXP Semiconductors. See coyright notice below.
 */

/*
 * @brief I2CM bus master example using polling mode
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2014
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "I2C.h"


I2C::I2C(int deviceNumber, uint32_t speed) {
	if(deviceNumber == 0) {
		device = LPC_I2C0;
		Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 22, IOCON_DIGMODE_EN | I2C_MODE);
		Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 23, IOCON_DIGMODE_EN | I2C_MODE);
		Chip_SWM_EnableFixedPin(SWM_FIXED_I2C0_SCL);
		Chip_SWM_EnableFixedPin(SWM_FIXED_I2C0_SDA);
	}
	else {
		// currently we support only I2C number 0
	}
	/* Enable I2C clock and reset I2C peripheral - the boot ROM does not
	   do this */
	Chip_I2C_Init(device);

	/* Setup clock rate for I2C */
	Chip_I2C_SetClockDiv(device, I2C_CLK_DIVIDER);

	/* Setup I2CM transfer rate */
	Chip_I2CM_SetBusSpeed(device, speed);

	/* Enable Master Mode */
	Chip_I2CM_Enable(device);
}

I2C::~I2C() {
	// TODO Auto-generated destructor stub
}


bool I2C::transaction(uint8_t devAddr, uint8_t *txBuffPtr, uint16_t txSize, uint8_t *rxBuffPtr, uint16_t rxSize) {
	I2CM_XFER_T  i2cmXferRec;
	/* Setup I2C transfer record */
	i2cmXferRec.slaveAddr = devAddr;
	i2cmXferRec.status = 0;
	i2cmXferRec.txSz = txSize;
	i2cmXferRec.rxSz = rxSize;
	i2cmXferRec.txBuff = txBuffPtr;
	i2cmXferRec.rxBuff = rxBuffPtr;

	Chip_I2CM_XferBlocking(device, &i2cmXferRec);

	/* Test for valid operation */
	if (i2cmXferRec.status == I2CM_STATUS_OK) {
		return true;
	}
	else {
		return false;
	}
}

