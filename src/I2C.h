/*
 * I2C.h
 *
 *  Created on: 21.2.2016
 *      Author: krl
 */

#ifndef I2C_H_
#define I2C_H_

#include "chip.h"

class I2C {
public:
	I2C(int deviceNumber, uint32_t speed);
	virtual ~I2C();
	bool transaction(uint8_t devAddr, uint8_t *txBuffPtr, uint16_t txSize, uint8_t *rxBuffPtr, uint16_t rxSize);
private:
	LPC_I2C_T *device;
	static const unsigned int I2C_CLK_DIVIDER = 40;
	static const unsigned int I2C_MODE = 0;
};

#endif /* I2C_H_ */
