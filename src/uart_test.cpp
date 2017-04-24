/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
 */

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>

// TODO: insert other include files here

// TODO: insert other definitions and declarations here

#include <cstring>
#include <cstdio>

#include "ModbusMaster.h"
#include "I2C.h"
#include "DigitalIoPin.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/


/*****************************************************************************
 * Public functions
 ****************************************************************************/
static volatile int counter;
static volatile uint32_t systicks;
static volatile int pressureMin = 14;
static volatile int pressureMax = 16;

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief	Handle interrupt from SysTick timer
 * @return	Nothing
 */
void SysTick_Handler(void)
{
	systicks++;
	if(counter > 0) counter--;
}
#ifdef __cplusplus
}
#endif

void Sleep(int ms)
{
	counter = ms;
	while(counter > 0) {
		__WFI();
	}
}

/* this function is required by the modbus library */
uint32_t millis() {
	return systicks;
}

/**
 * @brief	Main UART program body
 * @return	Always returns 1
 */

void printRegister(ModbusMaster& node, uint16_t reg) {
	uint8_t result;
	// slave: read 16-bit registers starting at reg to RX buffer
	result = node.readHoldingRegisters(reg, 1);

	// do something with data if read is successful
	if (result == node.ku8MBSuccess)
	{
		printf("R%d=%04X\n", reg, node.getResponseBuffer(0));
	}
	else {
		printf("R%d=???\n", reg);
	}
}

bool setFrequency(ModbusMaster& node, uint16_t freq) {
	uint8_t result;
	int ctr;
	bool atSetpoint;
	const int delay = 500;

	node.writeSingleRegister(1, freq); // set motor frequency

	printf("Set freq = %d\n", freq/40); // for debugging

	// wait until we reach set point or timeout occurs
	ctr = 0;
	atSetpoint = false;
	do {
		Sleep(delay);
		// read status word
		result = node.readHoldingRegisters(3, 1);
		// check if we are at setpoint
		if (result == node.ku8MBSuccess) {
			if(node.getResponseBuffer(0) & 0x0100) atSetpoint = true;
		}
		ctr++;
	} while(ctr < 20 && !atSetpoint);

	//printf("Elapsed: %d\n", ctr * delay); // for debugging

	return atSetpoint;
}


int main(void)
{

#if defined (__USE_LPCOPEN)
	// Read clock settings and update SystemCoreClock variable
	SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
	// Set up and initialize all required blocks and
	// functions related to the board hardware
	Board_Init();
	// Set the LED to the state of "On"
	Board_LED_Set(0, true);
#endif
#endif

	/* Set up SWO to PIO1_2 */
	Chip_SWM_MovablePortPinAssign(SWM_SWO_O, 1, 2); // Needed for SWO printf

	/* Enable and setup SysTick Timer at a periodic rate */
	SysTick_Config(SystemCoreClock / 1000);

	Board_LED_Set(0, false);
	Board_LED_Set(1, true);
	printf("Started\n");

	ModbusMaster node(2); // Create modbus object that connects to slave id 2
	I2C i2c(0, 100000);
    DigitalIoPin sw1 (0, 17, true, true, false);
    DigitalIoPin sw2 (1, 9, true, true, false);//
    DigitalIoPin sw3 (1, 11, true, true, false);

    node.begin(9600); // set transmission rate - other parameters are set inside the object and can be changed here
	printRegister(node, 3);
	node.writeSingleRegister(0, 0x0406);
	printRegister(node, 3);
	Sleep(1000);
	printRegister(node, 3);
	node.writeSingleRegister(0, 0x047F);
	printRegister(node, 3);
	Sleep(1000);
	printRegister(node, 3);

	int j = 0;
	uint16_t speed = 5000;
	uint16_t oldSpeed = 0;
	uint8_t pressureData[3];
	uint8_t readPressureCmd = 0xF1;
	int16_t pressure = 0;
	uint8_t result;
	int different;
	const uint16_t fa[20] = { 1000, 2000, 3000, 3500, 4000, 5000, 7000, 8000, 8300, 10000, 10000, 9000, 8000, 7000, 6000, 5000, 4000, 3000, 2000, 1000 };

	int button2press = 0;

	uint32_t waitUntil = systicks;
	uint32_t waitInterval = 4000;
	int db1, db2, db3;

	while(1) {


		if (oldSpeed != speed) {
			setFrequency(node, speed);
			oldSpeed = speed;
			printf("-------------------------------\n");
			printf(" Speed changed to: %d\n", speed);
			printf("-------------------------------\n");
		}

		if(sw2.read()){
			//while(sw2.read()){}
			button2press++;
			if(button2press > 1){
				button2press = 0;
				printf("manual mode\n");
				Sleep(1000);
			} else {
				printf("automatic mode\n");
				Sleep(1000);
			}
		}

		//Manual mode
		if(button2press == 0){
			if (sw1.read()) {
				//while (sw1.read()) {}
				if (speed < 20000) {
					speed = speed + 1000;
					printf("speed=%d\n",speed);
				}
			}
			if (sw3.read()) {
				//while (sw3.read()) {}
				if (speed > 1000) {
					speed = speed - 1000;
					printf("speed=%d\n",speed);
				}
			}
		}else if(button2press == 1){
			if (sw1.read()) {
				//while (sw1.read()) {}
				if (pressureMax < 120) {
					pressureMax++;
					pressureMin++;
					printf("pressureMax = %d, pressureMin = %d \n", pressureMax, pressureMin);
					Sleep(1000);
				}
			}
			if (sw3.read()) {
				//while (sw3.read()) {}
				if (pressureMin > 0) {
					pressureMax--;
					pressureMin--;
					printf("pressureMax = %d, pressureMin = %d \n", pressureMax, pressureMin);
					Sleep(1000);
				}
			}
		}


		if ( waitUntil < systicks){
			waitUntil = systicks + waitInterval;
			db1 = Chip_GPIO_GetPinState(LPC_GPIO, 0, 17);
			db2 = Chip_GPIO_GetPinState(LPC_GPIO, 1, 9);
			db3 = Chip_GPIO_GetPinState(LPC_GPIO, 1, 11);
			printf("Next update: %d\n", waitUntil);
			printf("-------------------------------------\n");
			printf("Buttons: %d %d %d\n", db1, db2, db3);
			printf("Speed: %d\n", speed);
			// READ PRESSURE
			if (i2c.transaction(0x40, &readPressureCmd, 1, pressureData, 3)) {
				pressure = (pressureData[0] << 8) | pressureData[1];
				different = pressure/240.0 - 15;
				printf("different = %d\n", different);
				DEBUGOUT("Pressure read over I2C is %.1f Pa\r\n",	pressure/240.0);
			} else {
				DEBUGOUT("Error reading pressure.\r\n");
			}


			//Sleep(1000);

			// READ MODBUS
			j = 0;
			do {
				result = node.readHoldingRegisters(102, 2);
				j++;
			} while(j < 3 && result != node.ku8MBSuccess);
			if (result == node.ku8MBSuccess) {
				printf("F=%4d, I=%4d  (ctr=%d)\n", node.getResponseBuffer(0), node.getResponseBuffer(1),j);
			} else {
				printf("ctr=%d\n",j);
			}
			//Sleep(3000);

			// AUTOMATIC
			if(button2press == 1){
				float pPascal = (pressure/240.0);
				printf("pPascal: %f\n", pPascal);
				if (pPascal < pressureMin  || pPascal > pressureMax ) {

					if (pPascal < pressureMin ) {
						speed = speed + 250;
						if (speed >= 20040) {
							speed = 20040; // 120 / 20040 = 167
						}
					} else if (pPascal > pressureMax ) {
						speed = speed - 250;
						if (speed >= 20040) {
							speed = 20040;
						}
					}
					if (speed < 1000) { speed = 1000; }
				}
			}

		}
	}

	/*while (1) {
		static uint32_t i;
		uint8_t j, result;
		uint16_t data[6];

		for(j = 0; j < 6; j++) {
			i++;
			// set word(j) of TX buffer to least-significant word of counter (bits 15..0)
			node.setTransmitBuffer(j, i & 0xFFFF);
		}
		// slave: write TX buffer to (6) 16-bit registers starting at register 0
		result = node.writeMultipleRegisters(0, 6);

		// slave: read (6) 16-bit registers starting at register 2 to RX buffer
		result = node.readHoldingRegisters(2, 6);

		// do something with data if read is successful
		if (result == node.ku8MBSuccess)
		{
			for (j = 0; j < 6; j++)
			{
				data[j] = node.getResponseBuffer(j);
			}
			printf("%6d, %6d, %6d, %6d, %6d, %6d\n", data[0], data[1], data[2], data[3], data[4], data[5]);
		}
		Sleep(1000);
	}*/


	return 1;
}

