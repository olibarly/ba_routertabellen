#ifndef MULTI_BOARD_UART
#define MULTI_BOARD_UART

#include "rodos.h"
#include <map>
#include <list>

class MultiBoardUART : StaticThread<> {
public:
	MultiBoardUART(uint8_t binId); //allows up to 256 different IDs, with 256 being used as a "broadcast" address

private:
	HAL_UART uartA;
	HAL_UART uartB;

	uint8_t binaryIdentifier;

	HAL_GPIO* sendLED;
	HAL_GPIO* rcvLED;
	HAL_GPIO* rcvSuccess;
	HAL_GPIO* statusLED;

	std::map<uint8_t, HAL_UART*> routingTable;
	std::list<uint8_t> reachableNextHops;

	bool checkBinIdValid(uint8_t binId);

	void updateTable(HAL_UART* uart, uint8_t binId);

	void send(HAL_UART* uart, const void* msg);
	void sendAliveMsg();

	size_t receive(HAL_UART* uart, void* rcvBuffer, const size_t maxLen = 100);
	void decodeAndHandleRcvMsg(HAL_UART* uart, void* msg, const uint8_t* targetAddress, uint8_t* nextHopAddress, HAL_UART* nextHopGateway);

	void init();
	void run();
};


#endif /* MULTI_BOARD_UART */
