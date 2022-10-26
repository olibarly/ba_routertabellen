/*
 * hypercube_routing.cpp
 *
 *  Created on: Oct 18, 2022
 *      Author: oliver
 */

#include "hypercube_routing.h"
#include <stdlib.h>

void HypercubeRouting::sendAliveMsg() {

	/**
	 * MSG Format:
	 * Header:
	 * target address (here alive msg broadcast address) [SpaceWire header Size]
	 *
	 * Body:
	 * binary Identifier of self [binId]
	 */

	size_t size = sizeof(binId) * 2;
	void* msg = malloc(size);
	// Header: Target Address
	binId* header = static_cast<binId*>(msg);
	*header = ALIVE_MSG_BROADCAST_ADDRESS;
	//Body: own binary Identifier/Address
	binId* body1 = static_cast<binId*>(msg + sizeof(typeof(*header)));
	*body1 = binaryIdentifier;

	for (HAL_UART uart : uartGateways) send(uart, msg, size);

	free(msg);
}

void HypercubeRouting::sendToAddress(binId targetAddress, const void* msg, size_t size) {
	HAL_UART* nextHopGateway;
	binId nextHopAddress;

	calculateNextHopFromTargetAddress(targetAddress, nextHopGateway, nextHopAddress);
	send(*nextHopGateway, msg, size);
}

void HypercubeRouting::handleRcvMsg(HAL_UART& uart, void* msg, const binId targetAddress, size_t size, binId& nextHopAddress, HAL_UART* nextHopGateway) {
	if (targetAddress == ALIVE_MSG_BROADCAST_ADDRESS) {
		updateTable(static_cast<binId*>(msg)[0], uart);
	} else if (targetAddress != binaryIdentifier) {
		calculateNextHopFromTargetAddress(targetAddress, nextHopGateway, nextHopAddress);
		send(*nextHopGateway, msg, size);
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
	for (std::list<uint8_t>::iterator it = neighborIds.begin(); it != neighborIds.end(); ++it) {
		if ((*it >> bitIndex) & 1) {
			nextHopAddress = *it;
			nextHopUartGateway = static_cast<RoutingTableEntry>(routingTable[*it]).uartGateway;
			break;
		}
	}
}
