/**
 * Testing the RODOS function PRINTF
 * with the pins specified by UART_DEBUG in platform-parameter.h
 */


#include "rodos.h"

GPIO_PIN GREEN  = GPIO_060;
GPIO_PIN ORANGE = GPIO_061;
GPIO_PIN RED    = GPIO_062;
GPIO_PIN BLUE   = GPIO_063;

HAL_GPIO green(GREEN);
HAL_GPIO orange(ORANGE);
HAL_GPIO red(RED);
HAL_GPIO blue(BLUE);


class PRINTF_Test : StaticThread<> {
public:
	PRINTF_Test(const char* name) : StaticThread(name) { }

private:
	void init() {
		green.init(true, 1, 0);
		orange.init(true, 1, 0);
		red.init(true, 1, 0);
		blue.init(true, 1, 0);
	}

	void run() {
		uint32_t pinVal = 1;

		TIME_LOOP(2 * SECONDS, 5 * SECONDS) {

			PRINTF("%s says \"Hello world!\"\n", getName());

			green.setPins(pinVal);
			blue.setPins(pinVal);

			pinVal ^= 1;

			red.setPins(pinVal);
			orange.setPins(pinVal);

		}
	}
} printf_test("PRINTF-Test-Thread");
