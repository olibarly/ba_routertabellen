/**
 * Testing a UART connection on a single board.
 */

#include "uart_test.h"

#include "rodos.h"

HAL_UART cmd(UART_DEBUG);

UART_Thread::UART_Thread(char name[2], char msg[32], HAL_UART* uart, int interval) : StaticThread<>(name) {
	strcpy(this->name, name);
	strcpy(this->msg, msg);
	this->uart = uart;
	this->interval = interval;
}

void UART_Thread::sendMsg() {
	size_t sentBytes = uart->write(msg, strlen(msg));
	size_t flushedBytes = uart->flush();
	PRINTF("%x, %x\n", sentBytes, flushedBytes);
}

void UART_Thread::rcvAndPrint() {
	//if (uartSelf->isDataReady()) {
		char readBuffer[32];
		uart->read(readBuffer, 32);

		PRINTF(name);
		PRINTF(" received:\n");
		PRINTF(readBuffer);
		PRINTF("\n");
	//}
}

void UART_Thread::init() {
	uart->init();
}

void UART_Thread::run() {
	int intervalCounter = 0;
	TIME_LOOP(interval * 500*MILLISECONDS, 2000*MILLISECONDS) {
		rcvAndPrint();
		if (intervalCounter == interval) {
			intervalCounter = 0;
			sendMsg();
		}
		intervalCounter++;
	}
}

