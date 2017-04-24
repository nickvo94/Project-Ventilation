/*
 * DigitalIoPin.h
 *
 *  Created on: 21.11.2016
 *      Author: Jake
 */

#ifndef DIGITALIOPIN_H_
#define DIGITALIOPIN_H_

class DigitalIoPin {
	public:
		DigitalIoPin(int port, int pin, bool input, bool pullup = true, bool invert = false) {
			if (input) {
				Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin, (IOCON_MODE_PULLUP | IOCON_DIGMODE_EN | IOCON_INV_EN));
				Chip_GPIO_SetPinDIRInput(LPC_GPIO, port, pin);
			} else {
				Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin, (IOCON_MODE_INACT | IOCON_DIGMODE_EN));
				Chip_GPIO_SetPinDIROutput(LPC_GPIO, port, pin);
			}
			sport = port;
			spin = pin;
		};
		virtual ~DigitalIoPin(){};
		bool read() {
			if (Chip_GPIO_GetPinState(LPC_GPIO, sport, spin)) {
				return true;
			} else {
				return false;
			}
		};
		void write(bool value) {
			Chip_GPIO_SetPinState(LPC_GPIO, sport, spin, value);
		};
	private:
		int sport;
		int spin;
};



#endif /* DIGITALIOPIN_H_ */
