#include "multi_board_uart.h"

#include "rodos.h"
#include "../misc/LED.h"

#include <map>
#include <list>
#include <cassert>


/** Definition of STATIC member [reserved_addresses]*/
const std::map<const char*, binId> reserved_addresses  = {
		{"ALIVE_MSG_BROADCAST_ADDRESS", ALIVE_MSG_BROADCAST_ADDRESS}
	};


HAL_UART uartIDX1(UART_IDX1); // working: UART_IDX1, UARTIDX2, UART_IDX6
HAL_UART uartIDX6(UART_IDX6);


bool strEqual(const char* a, const char* b) {
	if (strlen(a) == strlen(b)) {
		for (int i = 0; a[i]; i++) {
			if (a[i] != b[i]) return false; // different content
		}
		return true; // strings are equal
	}
	return false; // different length
}


MultiBoardUART::MultiBoardUART(binId binaryId) : StaticThread(), uartGateways{uartIDX1, uartIDX6}{
	assert(checkBinIdValid(binaryId));
	binaryIdentifier = binaryId;

	sendLED = &greenLED;
	rcvLED = &redLED;
	statusLED = &blueLED;
}

bool MultiBoardUART::checkBinIdValid(binId binaryId) {

	for (std::map<const char*, binId>::const_iterator it = reserved_addresses.begin(); it != reserved_addresses.end(); ++it) {
		if (binaryId == it->second) return false;
	}

	return true;
}


void MultiBoardUART::updateTable(binId binaryId, HAL_UART& uart) {
	RoutingTableEntry newEntry;
	newEntry.binaryId = binaryId;
	newEntry.uartGateway = &uart;
	newEntry.ttlSeconds = 120;

	routingTable[binaryId] = newEntry;
}

void MultiBoardUART::send(HAL_UART& uart, const void* msg, size_t size) {
	sendLED->setPins(1);

	uart.write(msg, size); // strlen + 1 to ensure null terminator is also sent
}

size_t MultiBoardUART::receive(HAL_UART& uart, void* rcvBuffer, const size_t maxLen /* = 100*/) {
	if (uart.isDataReady()) {
		rcvLED->setPins(1);

		size_t readLen = uart.read(rcvBuffer, maxLen);

		return readLen;
	}
	return 0;
}

void MultiBoardUART::decodeRcvMsg(binId& targetAddress, void* msg) {
	targetAddress = *static_cast<binId*>(msg);
	msg = static_cast<binId*>(msg) + 1;
}

void MultiBoardUART::init() {
	sendLED->init(true, 1, 0);
	rcvLED->init(true, 1, 0);
	statusLED->init(true, 1, 1);

	for (HAL_UART uart : uartGateways) uart.init();
}

void MultiBoardUART::run() {
	int sendIntervalCounter = 0;
	int sendAliveIntervalCounter = 0;

	int64_t sendAliveInterval = 5*SECONDS;
	int64_t loopInterval = 50*MILLISECONDS;

	uint32_t statusPinVal = 0;


	sendAliveMsg();

	TIME_LOOP(0*SECONDS, loopInterval) {
		sendLED->setPins(0);
		rcvLED->setPins(0);
		statusLED->setPins(statusPinVal);

		std::list<std::map<binId, RoutingTableEntry>::iterator> elementsToRemove;

		for (auto it = routingTable.begin(); it != routingTable.cend(); ++it) {
			it->second.ttlSeconds -= loopInterval;
			if (it->second.ttlSeconds <= 0) elementsToRemove.push_back(it);
		}
		for (auto e : elementsToRemove) routingTable.erase(e);


		if (sendAliveIntervalCounter >= sendAliveInterval / loopInterval) {
			sendAliveIntervalCounter = 0;
			sendAliveMsg();
		}

		for (HAL_UART uart : uartGateways) {
			char rcvBufferA[100];
			size_t rcvSizeA = receive(uart, rcvBufferA);
			if (rcvSizeA != 0) {
				binId targetAddress;
				decodeRcvMsg(targetAddress, rcvBufferA);

				binId nextHopAddress;
				HAL_UART* nextHopGateway;
				handleRcvMsg(uart, rcvBufferA, targetAddress, rcvSizeA, nextHopAddress, nextHopGateway);
			}
		}

		statusPinVal ^= 1;
		sendIntervalCounter++;
		sendAliveInterval++;
	}
}
