/**
 * Testing the RODOS function PRINTF
 * with the pins specified by UART_DEBUG in platform-parameter.h
 */

#include "printf_test.h"

#include "rodos.h"
#include "../misc/LED.h"


PRINTF_Test::PRINTF_Test(const char* name) : StaticThread(name) { }

void PRINTF_Test::init() {
	greenLED.init(true, 1, 0);
	orangeLED.init(true, 1, 0);
	redLED.init(true, 1, 0);
	blueLED.init(true, 1, 0);
}

void PRINTF_Test::run() {
	uint32_t pinVal = 1;

	TIME_LOOP(2 * SECONDS, 5 * SECONDS) {
		PRINTF("%s says \"Hello world!\"\n", getName());

		greenLED.setPins(pinVal);
		blueLED.setPins(pinVal);

		pinVal ^= 1;

		redLED.setPins(pinVal);
		orangeLED.setPins(pinVal);

	}
}
