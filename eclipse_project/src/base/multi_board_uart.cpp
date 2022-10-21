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


MultiBoardUART::MultiBoardUART(binId binaryId) : StaticThread(), uartA(uartIDX1), uartB(uartIDX6) {
	assert(checkBinIdValid(binaryId));
	binaryIdentifier = binaryId;

	sendLED = &greenLED;
	rcvLED = &redLED;f
	statusLED = &blueLED;
}

bool MultiBoardUART::checkBinIdValid(binId binaryId) {

	for (std::map<const char*, binId>::const_iterator it = reserved_addresses.begin(); it != reserved_addresses.end(); ++it) {
		if (binaryId == it->second) return false;
	}

	return true;
}


void MultiBoardUART::updateTable(HAL_UART* uart, binId binaryId) {
	routingTable[binaryId] = uart;
}

void MultiBoardUART::send(HAL_UART* uart, const void* msg) {
	sendLED->setPins(1);

	uart->write(msg, strlen(static_cast<const char*>(msg))); // strlen + 1 to ensure null terminator is also sent
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

void MultiBoardUART::decodeRcvMsg(void* msg, const binId* targetAddress) {
	targetAddress = static_cast<binId*>(msg);
	msg = static_cast<binId*>(msg) + 1;
}

void MultiBoardUART::init() {
	sendLED->init(true, 1, 0);
	rcvLED->init(true, 1, 0);
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
