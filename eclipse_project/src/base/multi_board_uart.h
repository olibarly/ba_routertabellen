#ifndef MULTI_BOARD_UART
#define MULTI_BOARD_UART

#include "rodos.h"
#include <map>
#include <list>


typedef uint8_t binId;
#define ALIVE_MSG_BROADCAST_ADDRESS 0b11111111


class MultiBoardUART : StaticThread<> {
public:
	MultiBoardUART(binId binaryId); //allows up to 256 different IDs, with 256 being used as a "broadcast" address

protected:
	static const std::map<const char*, binId>reserved_addresses;

	HAL_UART uartA;
	HAL_UART uartB;

	binId binaryIdentifier;

	std::map<binId, HAL_UART*> routingTable;
	std::list<binId> reachableNextHops;

	bool checkBinIdValid(binId BinaryId);

	void updateTable(HAL_UART* uart, binId binaryId);

	void send(HAL_UART* uart, const void* msg, size_t size);
	virtual void sendAliveMsg();
	virtual void sendToAddress(binId* targetAddress, const void* msg, size_t size);

	size_t receive(HAL_UART* uart, void* rcvBuffer, const size_t maxLen = 100);
	void decodeRcvMsg(void* msg, const binId* targetAddress);
	virtual void handleRcvMsg(HAL_UART* uart, void* msg, const binId* targetAddress, size_t size, binId* nextHopAddress, HAL_UART* nextHopGateway);

private:
	HAL_GPIO* sendLED;
	HAL_GPIO* rcvLED;
	HAL_GPIO* statusLED;
	void init();
	void run();
};


#endif /* MULTI_BOARD_UART */
