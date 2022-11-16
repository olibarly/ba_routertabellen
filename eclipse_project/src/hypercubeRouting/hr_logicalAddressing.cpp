/*
 * hypercube_routing.cpp
 *
 *  Created on: Oct 18, 2022
 *      Author: oliver
 */

#include "../hypercubeRouting/hr_logicalAddressing.h"

void HypercubeRouting::sendToAddress(binId targetAddress, const void* msgBody, size_t msgBodySize) {
	HAL_UART* nextHopGateway;
	binId nextHopAddress;
	size_t addressingLength;

	calculateAddressing(targetAddress, nextHopGateway, &nextHopAddress, &addressingLength);
	send(*nextHopGateway, msgBody, msgBodySize);
}

void HypercubeRouting::handleRcvMsg(HAL_UART* uart, void* msg, const binId targetAddress, void* msgBody, size_t size) {
	if (targetAddress == ALIVE_MSG_BROADCAST_ADDRESS) {
		updateTable(static_cast<binId*>(msgBody)[0], uart);
	} else if (targetAddress != binaryIdentifier) {
		HAL_UART* nextHopGateway;
		binId nextHopAddress;

		std::map<binId, RoutingTableEntry>::iterator it = routingTable.find(nextHopAddress);
		if (it != routingTable.end()) { // Address already exists in routing table
			nextHopGateway = it->second.uartGateway;
			it->second.resetTTL(); // reset TTL
		} else { // Address does not exist in routing table
			binId nextHopAddress;
			size_t length;
			calculateAddressing(targetAddress, nextHopGateway, &nextHopAddress, &length);
			routingTable.insert({nextHopAddress, RoutingTableEntry(nextHopAddress, nextHopGateway)});
		}

		send(*nextHopGateway, msgBody, size);
	} else {
	}
}

void HypercubeRouting::calculateAddressing(const binId targetAddress, HAL_UART* nextHopUartGateway, binId* addressing, size_t* addressingLength) {
	*addressingLength = sizeof(binId);

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
			*addressing = it->first;
			nextHopUartGateway = static_cast<RoutingTableEntry>(routingTable[it->first]).uartGateway;
			break;
		}
	}
}
