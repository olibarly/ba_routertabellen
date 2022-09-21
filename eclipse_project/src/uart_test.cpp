/**
 * Testing a UART connection on a single board.
 */


#include "rodos.h"

HAL_UART cmd(UART_DEBUG);

HAL_UART uartA(UART_IDX1);
HAL_UART uartB(UART_IDX4);

class UART_Thread : StaticThread<> {
public:
	UART_Thread(char name[2], char msg[32], HAL_UART* uart, int interval) : StaticThread<>(name) {
		strcpy(this->name, name);
		strcpy(this->msg, msg);
		this->uart = uart;
		this->interval = interval;
	}


private:
	char name[2];
	char msg[32];
	HAL_UART* uart;
	int interval;

	void sendMsg() {
		size_t sentBytes = uart->write(msg, strlen(msg));
		size_t flushedBytes = uart->flush();
		PRINTF("%x, %x\n", sentBytes, flushedBytes);
	}

	void rcvAndPrint() {
		//if (uartSelf->isDataReady()) {
			char readBuffer[32];
			uart->read(readBuffer, 32);

			PRINTF(name);
			PRINTF(" received:\n");
			PRINTF(readBuffer);
			PRINTF("\n");
		//}
	}

	void init() {
		uart->init();
	}

	void run() {
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
};

char nameA[]{"A"};
char nameB[]{"B"};

char msgA[]{"This is a message from Thread A"};
char msgB[]{"This is a message from Thread B"};

UART_Thread threadA(nameA, msgA, &uartA, 1);

UART_Thread threadB(nameB, msgB, &uartB, 2);
