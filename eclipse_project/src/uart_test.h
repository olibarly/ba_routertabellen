#ifndef UART_TEST
#define UART_TEST

#include "rodos.h"

class UART_Thread : StaticThread<> {
public:
	UART_Thread(char name[2], char msg[32], HAL_UART* uart, int interval);

private:
	char name[2];
	char msg[32];
	HAL_UART* uart;
	int interval;

	void sendMsg();
	void rcvAndPrint();

	void init();
	void run();
};


#endif
