/*
 * hypercube_routing2.cpp
 *
 *  Created on: Oct 27, 2022
 *      Author: oliver
 */


#include "../hypercubeRouting/hr_pathAddressing.h"

#include "hal/hal_uart.cpp"
#include <stdlib.h>

/*
void HypercubeRouting2::sendFloodingMsg() {
	size_t size = sizeof(binId) * (2 + neighborIds.size()) + sizeof(uint8_t);
	void* msg = malloc(size);

	// Header: Target Address
	binId* header = static_cast<binId*>(msg);
	*header = FLOODING_MSG_BROADCAST_ADDRESS;

	// Body: own binary Identifier/Address
	binId* body1 = static_cast<binId*>(msg + sizeof(typeof(*header)));
	*body1 = binaryIdentifier;

	uint8_t* body2 = static_cast<uint8_t*>(msg + sizeof(typeof(*body1)) + sizeof(typeof(header)));
	*body2 = floodingMsgCounter;
	++floodingMsgCounter;

	// Body: binary IDs of adjacent nodes
	binId* body3 = static_cast<binId*>(msg + sizeof(typeof(*header)) + sizeof(typeof(*body2)) + sizeof(typeof(body1)));
	for (std::map<binId, HAL_UART*>::const_iterator it = neighborIds.cbegin(); it != neighborIds.cend(); ++it) {
		*body3 = it->first;
		++body3;
	}

	for (HAL_UART uart : uartGateways) send(uart, msg, size);

	free(msg);
}
*/

void HypercubeRouting2::sendToAddress(binId targetAddress, const void* msg, size_t msgSize) {
	HypercubeRouting2RoutingTableEntry routingData = routingTable[targetAddress];
	size_t sizeAll = sizeof(binId) * routingData.binIdChain.size() + msgSize;

	void* msgAll = malloc(sizeAll);

	binId* addressChain = static_cast<binId*>(msgAll);
	for (binId address : routingData.binIdChain) {
		*addressChain = address;
		++addressChain;
	}

	void* body = msgAll + sizeof(typeof(*addressChain)) * routingData.binIdChain.size();
	memcpy(body, msg, msgSize);

	send(*routingData.uartGateway, msgAll, sizeAll);

	free(msgAll);
}

void HypercubeRouting2::forwardMsg(const void* msg, size_t size) {
	binId targetAddress = *static_cast<const binId*>(msg);

	send(*neighborIds[targetAddress], msg, size);
}

/*
 * void HypercubeRouting2::forwardFloodingMsg(HAL_UART* receivingGateway, const void* msg, size_t size) {
	for (HAL_UART uart : uartGateways) {
		if (&uart != receivingGateway) send(uart, msg, size); // Don't know if inequality check is really working here, but since we're only working with pointers, receiving Gateway should point to HAL_UART stored in uartGateways
	}
}
*/

void HypercubeRouting2::handleRcvMsg(HAL_UART* uart, void* msg, const binId targetAddress, void* msgBody, size_t size) {
	if (targetAddress == ALIVE_MSG_BROADCAST_ADDRESS) {
		// extract source address; refresh ttl in corresponding routing table entry
		binId sourceAddress = *static_cast<binId*>(msgBody);
		routingTable[sourceAddress].ttlSeconds = 60;
	} /*else if (targetAddress == FLOODING_MSG_BROADCAST_ADDRESS) {
		// check if sourceID - floodingID combo already known; respectively drop or forward Msg (but not via receiving gateway)
		binId sourceId = *static_cast<binId*>(msgBody);
		uint8_t floodingId = *static_cast<uint8_t*>(msgBody + sizeof(typeof(sourceId)));

		for (std::list<FloodingMsgData>::const_iterator it = floodingmsgs.cbegin(); it != floodingmsgs.cend(); ++it) {
			if (it->nodeId == sourceId && it->floodingId == floodingId) return; // if known -> do nothing
		}
		// else if not known -> add data to table, then forward to all, except receiving gateway
		floodingmsgs.push_back(FloodingMsgData(sourceId, floodingId));
		// TODO: Flooding Msg Data handling
		forwardFloodingMsg(uart, msg, size);
	}*/ else if (targetAddress == binaryIdentifier) {
		// forward Msg to topic (?) to further handle internally
	} else {
		forwardMsg(msgBody, size);
	}
}

void HypercubeRouting2::calculateHopChainFromTargetAddress(const binId targetAddress, std::list<binId>& binIdChain) {
	binId diff = targetAddress ^ binaryIdentifier;
	binId address = binaryIdentifier;

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
		binIdChain.push_back(address);

		// calculate new difference
		diff = targetAddress ^ address;
	}

	// Find nextHop that matches that the first Address
}
