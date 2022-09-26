#ifndef MULTI_BOARD_UART
#define MULTI_BOARD_UART

#include "rodos.h"

class MultiBoardUART : StaticThread<> {
public:
	MultiBoardUART(HAL_UART* uart, char* name = "AnonymThread");

private:
	HAL_UART* uart;

	void send(char * msg);
	void receive();

	void init();
	void run();
};


#endif /* MULTI_BOARD_UART */
