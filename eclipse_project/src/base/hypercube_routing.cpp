/*
 * hypercube_routing.cpp
 *
 *  Created on: Oct 18, 2022
 *      Author: oliver
 */

#include "hypercube_routing.h"

void HypercubeRouting::sendAliveMsg() {
	char msg[2];
	msg[0] = ALIVE_MSG_BROADCAST_ADDRESS;
	msg[1] = binaryIdentifier;
	send(&uartA, msg);
	send(&uartB, msg);
}

void HypercubeRouting::sendToAddress(binId* targetAddress, const void *msg) {
	HAL_UART* nextHopGateway;
	binId* nextHopAddress;

	calculateNextHopFromTargetAddress(targetAddress, nextHopGateway, nextHopAddress);
	send(nextHopGateway, msg);
}

void HypercubeRouting::handleRcvMsg(HAL_UART *uart, void *msg, const binId *targetAddress, binId *nextHopAddress, HAL_UART *nextHopGateway) {
	if (*targetAddress == ALIVE_MSG_BROADCAST_ADDRESS) {
		updateTable(uart, static_cast<binId*>(msg)[1]);
	} else if (*targetAddress != binaryIdentifier) {
		calculateNextHopFromTargetAddress(targetAddress, nextHopGateway, nextHopAddress);
		send(nextHopGateway, msg);
	} else {
	}
}

void HypercubeRouting::calculateNextHopFromTargetAddress(const binId* targetAddress, HAL_UART* nextHopUartGateway, binId* nextHopAddress) {
	binId diff = *targetAddress ^ binaryIdentifier;

	// Find first bit difference from back (LSB)
	int bitIndex = 0;
	while (diff && !(diff & 1)) {
		diff >>= 1;
		bitIndex++;
	}

	// Find nextHop that matches that difference
	for (std::list<uint8_t>::iterator it = reachableNextHops.begin(); it != reachableNextHops.end(); ++it) {
		if ((*it >> bitIndex) & 1) {
			nextHopAddress = &*it;
			nextHopUartGateway = routingTable[*it];
			break;
		}
	}
}
