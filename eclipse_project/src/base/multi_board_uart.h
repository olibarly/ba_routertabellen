#ifndef MULTI_BOARD_UART
#define MULTI_BOARD_UART

#include "rodos.h"
#include <map>
#include <list>


typedef uint8_t binId;
#define ALIVE_MSG_BROADCAST_ADDRESS		0b11111111
#define FLOODING_MSG_BROADCAST_ADDRESS	0b11111110


struct UART_Gateway {
	uint8_t id;
	HAL_UART hal_uart;
};

struct RoutingTableEntry {
	virtual ~RoutingTableEntry() {};

	binId binaryId;
	HAL_UART* uartGateway;
	uint8_t ttlSeconds;
};


class MultiBoardUART : StaticThread<> {
public:
	MultiBoardUART(binId binaryId); //allows up to 256 different IDs, with 256 being used as a "broadcast" address

protected:
	static const std::map<const char*, binId> reserved_addresses;

	HAL_UART uartGateways[2];

	binId binaryIdentifier;

	std::map<binId, RoutingTableEntry> routingTable;
	std::map<binId, HAL_UART*> neighborIds;

	bool checkBinIdValid(binId BinaryId);

	void updateTable(binId binaryId, HAL_UART* uart);
	virtual void calculateNextHopFromTargetAddress(const binId targetAddress, HAL_UART* nextHopUartGateway, binId nextHopAddress);

	void send(HAL_UART& uart, const void* msg, size_t size);
	void sendAliveMsg();
	virtual void sendToAddress(binId targetAddress, const void* msg, size_t msgSize);

	size_t receive(HAL_UART& uart, void* rcvBuffer, const size_t maxLen = 100);
	void decodeRcvMsg(binId& targetAddress, void* msg);
	/**
	* @param size includes header (targetAddress) and msgBody (body)
	*/
	virtual void handleRcvMsg(HAL_UART* uart, void* msg, const binId targetAddress, void* msgBody, size_t size);

private:
	HAL_GPIO* sendLED;
	HAL_GPIO* rcvLED;
	HAL_GPIO* statusLED;
	void init();
	void run();
};


#endif /* MULTI_BOARD_UART */
