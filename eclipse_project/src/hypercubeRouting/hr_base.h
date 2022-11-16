/*
 * hr_base.h
 *
 *  Created on: Nov 16, 2022
 *      Author: oliver
 */

#ifndef SRC_HYPERCUBEROUTING_HR_BASE_H_
#define SRC_HYPERCUBEROUTING_HR_BASE_H_

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
	RoutingTableEntry();
	RoutingTableEntry(binId t, HAL_UART* u) {
		RoutingTableEntry(t, u, 60);
	}
	RoutingTableEntry(binId t, HAL_UART* u, uint8_t ttl) {
		RoutingTableEntry(t, &t, 1, u, ttl);
	}
	RoutingTableEntry(binId t, binId* a, size_t l, HAL_UART* u, uint8_t ttl) {
		targetAddress = t;
		a = a;
		l = l;
		uartGateway = u;
		ttlSeconds = ttl;
	}
	virtual ~RoutingTableEntry() {};

	void resetTTL() {
		ttlSeconds = 60;
	}

	binId targetAddress;
	binId* addressing;
	size_t addressingLength;
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
	virtual void calculateAddressing(const binId targetAddress, HAL_UART* nextHopUartGateway, binId* addressing, size_t* addressingLength);

	void send(HAL_UART& uart, const void* msg, size_t size);
	void sendAliveMsg();
	virtual void sendToAddress(binId targetAddress, const void* msgBody, size_t msgBodySize);

	size_t receive(HAL_UART& uart, void* rcvBuffer, const size_t maxLen = 100);
	void decodeRcvMsg(void* msg, binId& targetAddress, void* msgBody);
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



#endif /* SRC_HYPERCUBEROUTING_HR_BASE_H_ */
