#include "multi_board_uart.h"

#include "rodos.h"
#include "LED.h"

#include <map>
#include <list>
#include <cassert>

#define ALIVE_MSG_BROADCAST_ADDRESS (reserved_addresses["ALIVE_MSG_BROADCAST_ADDRESS"])

std::map<const char*, uint8_t> reserved_addresses = {
		{"ALIVE_MSG_BROADCAST_ADDRESS", 0b11111111}
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


MultiBoardUART::MultiBoardUART(uint8_t binId) : StaticThread(), uartA(uartIDX1), uartB(uartIDX6) {
	assert(checkBinIdValid(binId));
	binaryIdentifier = binId;

	sendLED = &greenLED;
	rcvLED = &redLED;
	rcvSuccess = &orangeLED;
	statusLED = &blueLED;
}

bool MultiBoardUART::checkBinIdValid(uint8_t binId) {

	for (std::map<const char*, uint8_t>::iterator it = reserved_addresses.begin(); it != reserved_addresses.end(); ++it) {
		if (binId == it->second) return false;
	}

	return true;
}


void MultiBoardUART::updateTable(HAL_UART* uart, uint8_t binId) {
	routingTable[binId] = uart;
}

void MultiBoardUART::send(HAL_UART* uart, const void* msg) {
	sendLED->setPins(1);

	uart->write(msg, strlen(static_cast<const char*>(msg))); // strlen + 1 to ensure null terminator is also sent
}

void MultiBoardUART::sendAliveMsg() {
	char msg[2];
	msg[0] = ALIVE_MSG_BROADCAST_ADDRESS;
	msg[1] = binaryIdentifier;
	send(&uartA, msg);
	send(&uartB, msg);
}

size_t MultiBoardUART::receive(HAL_UART* uart, void* rcvBuffer, const size_t maxLen /* = 100*/) {
	if (uart->isDataReady()) {
		rcvLED->setPins(1);

		size_t readLen = uart->read(rcvBuffer, maxLen);

		//const char* expectedString = "test";
		//if (strEqual(rcvBuffer, expectedString)) rcvSuccess->setPins(1);

		return readLen;
	}
	return -1;
}

void MultiBoardUART::decodeAndHandleRcvMsg(HAL_UART* uart, void* msg, const uint8_t* targetAddress, uint8_t* nextHopAddress, HAL_UART* nextHopGateway) {
	targetAddress = static_cast<uint8_t*>(msg);

	if (*targetAddress == ALIVE_MSG_BROADCAST_ADDRESS) {
		updateTable(uart, static_cast<uint8_t*>(msg)[1]);
	}
	else if (*targetAddress != binaryIdentifier) {
		uint8_t diff = *targetAddress ^ binaryIdentifier;

		// Find first bit difference from back (LSB)
		int bitIndex = 0;
		while (diff && !(diff & 1)) {
			diff >>= 1;
			bitIndex++;
		}

		// Find nextHop that matches that difference
		for (std::list<uint8_t>::iterator it = reachableNextHops.begin(); it != reachableNextHops.end(); ++it){
			if ((*it >> bitIndex) & 1) {
				nextHopAddress = &*it;
				nextHopGateway = routingTable[*it];

				send(nextHopGateway, msg);
				break;
			}
		}
	} else {}
}

void MultiBoardUART::init() {
	sendLED->init(true, 1, 0);
	rcvLED->init(true, 1, 0);
	rcvSuccess->init(true, 1, 0);
	statusLED->init(true, 1, 1);

	uartA.init(115200);
	uartB.init();
}

void MultiBoardUART::run() {
	int sendIntervalCounter = 0;
	int sendAliveIntervalCounter = 0;

	int64_t sendInterval = 2*SECONDS;
	int64_t sendAliveInterval = 5*SECONDS;
	int64_t loopInterval = 50*MILLISECONDS;

	uint32_t statusPinVal = 0;


	sendAliveMsg();

	TIME_LOOP(1*SECONDS, loopInterval) {
		sendLED->setPins(0);
		rcvLED->setPins(0);
		rcvSuccess->setPins(0);
		statusLED->setPins(statusPinVal);

		if (sendIntervalCounter >= sendInterval / loopInterval) {
			sendIntervalCounter = 0;
			send(&uartA, "test");
		}
		if (sendAliveIntervalCounter >= sendAliveInterval / loopInterval) {
			sendAliveIntervalCounter = 0;
			sendAliveMsg();
		}

		char rcvBufferA[100];
		receive(&uartA, rcvBufferA);
		//decodeRcvMsg(uartA, rcvBufferA);
		char rcvBufferB[100];
		receive(&uartB, rcvBufferB);
		//decodeRcvMsg(uartB, rcvBufferB);

		statusPinVal ^= 1;
		sendIntervalCounter++;
		sendAliveInterval++;
	}
}
