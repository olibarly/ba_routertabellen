#include "../tests/printf_test.h"
#include "../tests/uart_test.h"
//#include "../base/hypercube_routing.h"


/* PRINTF Test */
//PRINTF_Test printf_test("PRINTF-Test-Thread");

/* UART Test */
/*
HAL_UART uartA(UART_IDX1);
HAL_UART uartB(UART_IDX4);

char nameA[]{"A"};
char nameB[]{"B"};

char msgA[]{"This is a message from Thread A"};
char msgB[]{"This is a message from Thread B"};

UART_Thread threadA(nameA, msgA, &uartA, 1);

UART_Thread threadB(nameB, msgB, &uartB, 2);
*/


/* Multi Board UART Static Routing*/
//MultiBoardUART thread(0b00, true); // Board 1
//MultiBoardUART thread(0b01, true); // Board 2
//MultiBoardUART thread(0b10, true); // Board 3
