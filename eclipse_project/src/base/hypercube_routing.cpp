/*
 * hypercube_routing.cpp
 *
 *  Created on: Oct 18, 2022
 *      Author: oliver
 */

#include "hypercube_routing.h"

void HypercubeRouting::sendToAddress(binId targetAddress, const void* msg, size_t msgSize) {
	HAL_UART* nextHopGateway;
	binId nextHopAddress;

	calculateNextHopFromTargetAddress(targetAddress, nextHopGateway, nextHopAddress);
	send(*nextHopGateway, msg, msgSize);
}

void HypercubeRouting::handleRcvMsg(HAL_UART* uart, void* msg, const binId targetAddress, void* msgBody, size_t size) {
	if (targetAddress == ALIVE_MSG_BROADCAST_ADDRESS) {
		updateTable(static_cast<binId*>(msgBody)[0], uart);
	} else if (targetAddress != binaryIdentifier) {
		HAL_UART* nextHopGateway;
		binId nextHopAddress;
		calculateNextHopFromTargetAddress(targetAddress, nextHopGateway, nextHopAddress);
		send(*nextHopGateway, msgBody, size);
	} else {
	}
}

void HypercubeRouting::calculateNextHopFromTargetAddress(const binId targetAddress, HAL_UART* nextHopUartGateway, binId nextHopAddress) {
	binId diff = targetAddress ^ binaryIdentifier;

	// Find first bit difference from back (LSB)
	int bitIndex = 0;
	while (diff && !(diff & 1)) {
		diff >>= 1;
		bitIndex++;
	}

	// Find nextHop that matches that difference
	for (std::map<binId, HAL_UART*>::iterator it = neighborIds.begin(); it != neighborIds.end(); ++it) {
		if ((it->first >> bitIndex) & 1) {
			nextHopAddress = it->first;
			nextHopUartGateway = static_cast<RoutingTableEntry>(routingTable[it->first]).uartGateway;
			break;
		}
	}
}
