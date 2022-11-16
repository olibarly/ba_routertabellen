/*
 * hypercube_routing2.cpp
 *
 *  Created on: Oct 27, 2022
 *      Author: oliver
 */


#include "../hypercubeRouting/hr_pathAddressing.h"

#include "hal/hal_uart.cpp"
#include <stdlib.h>


void HypercubeRouting2::sendToAddress(binId targetAddress, const void* msg, size_t msgSize) {
	RoutingTableEntry routingData = routingTable[targetAddress];
	size_t sizeAll = sizeof(binId) * routingData.addressingLength + msgSize;

	void* msgAll = malloc(sizeAll);

	binId* addressChain = static_cast<binId*>(msgAll);
	memcpy(addressChain, routingData.addressing, routingData.addressingLength);

	void* body = msgAll + sizeof(typeof(*addressChain)) * routingData.addressingLength;
	memcpy(body, msg, msgSize);

	send(*routingData.uartGateway, msgAll, sizeAll);

	free(msgAll);
}

void HypercubeRouting2::forwardMsg(const void* msg, size_t size) {
	binId targetAddress = *static_cast<const binId*>(msg);

	send(*neighborIds[targetAddress], msg, size);
}

void HypercubeRouting2::handleRcvMsg(HAL_UART* uart, void* msg, const binId targetAddress, void* msgBody, size_t size) {
	if (targetAddress == ALIVE_MSG_BROADCAST_ADDRESS) {
		// extract source address; refresh ttl in corresponding routing table entry
		binId sourceAddress = *static_cast<binId*>(msgBody);
		routingTable[sourceAddress].ttlSeconds = 60;
	} else if (targetAddress == binaryIdentifier) {
		// forward Msg to topic (?) to further handle internally
	} else {
		forwardMsg(msgBody, size);
	}
}

void HypercubeRouting2::calculateAddressing(const binId targetAddress, HAL_UART* nextHopUartGateway, binId* addressing, size_t* addressingLength) {
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
