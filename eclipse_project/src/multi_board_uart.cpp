#include "multi_board_uart.h"

#include "rodos.h"
#include "LED.h"

HAL_GPIO* sendLED = &greenLED;
HAL_GPIO* rcvLED = &redLED;
HAL_GPIO* statusLED = &blueLED;

MultiBoardUART::MultiBoardUART(HAL_UART* uart, char* name /*= "AnonymThread"*/) : StaticThread(name) {
	this->uart = uart;
}

void MultiBoardUART::send(char* msg) {
	sendLED->setPins(1);

	uart->write(msg, strlen(msg));
}

void MultiBoardUART::receive() {
	if (uart->isDataReady()) {
		rcvLED->setPins(1);

		char* readBuffer = "";
		int readLen = uart->read(readBuffer, 100);
	}
}

void MultiBoardUART::init() {
	sendLED->init(true, 1, 0);
	rcvLED->init(true, 1, 0);
	statusLED->init(true, 1, 1);
	uart->init();
}

void MultiBoardUART::run() {
	int sendIntervalCounter = 0;
	int64_t sendInterval = 2*SECONDS;
	int64_t loopInterval = 50*MILLISECONDS;

	uint32_t statusPinVal = 0;

	TIME_LOOP(1*SECONDS, loopInterval) {
		sendLED->setPins(0);
		rcvLED->setPins(0);
		statusLED->setPins(statusPinVal);

		if (sendIntervalCounter >= sendInterval / loopInterval) {
			sendIntervalCounter = 0;
			send("test");
		}

		receive();

		statusPinVal ^= 1;
		sendIntervalCounter++;

	}
}
