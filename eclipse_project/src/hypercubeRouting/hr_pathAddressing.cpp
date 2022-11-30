/*
 * hypercube_routing2.cpp
 *
 *  Created on: Oct 27, 2022
 *      Author: oliver
 */


#include "../hypercubeRouting/hr_pathAddressing.h"

#include "hal/hal_uart.cpp"
#include <stdlib.h>


void HRPathAddressing::forwardMsg(const void* msg, size_t size) {
	binId targetAddress = *static_cast<const binId*>(msg);

	send(*adjacentNodes[targetAddress].uartGateway, msg, size);
}

void HRPathAddressing::handleRcvMsg(HAL_UART* uart, void* msg, const binId targetAddress, void* msgBody, size_t size) {
	if (targetAddress == BROADCAST_ADDRESS) {
		handleAliveMsg(uart, msg);
	} else if (targetAddress == binaryIdentifier) {
		// forward Msg to topic (?) to further handle internally
	} else {
		forwardMsg(msgBody, size);
	}
}

void HRPathAddressing::calculateAddressing(const binId targetAddress, HAL_UART* nextHopUartGateway, binId* addressing, size_t* addressingLength) {
	binId diff = targetAddress ^ binaryIdentifier;
	binId address = binaryIdentifier;

	*addressingLength = 0;

	while(diff != 0) {
		// Find first bit difference from back (LSB)
		int bitIndex = 0;
		while (diff && !(diff & 1)) {
			diff >>= 1;
			bitIndex++;
		}

		// calculate next Address from last Address and first bit difference from back
		binId bitMask = 1 << bitIndex;
		address = address ^ bitMask;

		// add next Address to List
		addressing[(*addressingLength) / sizeof(binId)] = address;
		*addressingLength += sizeof(binId);

		// calculate new difference
		diff = targetAddress ^ address;
	}

	// Find nextHopGateway that matches the first Address
}
