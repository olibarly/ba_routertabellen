/*
 * link_state_routing.cpp
 *
 *  Created on: Oct 24, 2022
 *      Author: oliver
 */


#include "link_state_routing.h"
#include <stdlib.h>

void LinkStateRouting::sendAliveMsg() {
	/**
	 * MSG-Format:
	 * Header: target address (AliveMsgBroadcastAddress)
	 * Body: flooding ID -- own binary ID -- neighbors binary IDs
	 */

	size_t size = sizeof(binId) * (2 + neighborIds.size()) + sizeof(uint16_t);
	void* msg = malloc(size);

	binId* header = static_cast<binId*>(msg);
	*header = ALIVE_MSG_BROADCAST_ADDRESS;

	uint16_t* body1 = static_cast<uint16_t*>(msg + sizeof(typeof(*header)));
	srand(binaryIdentifier);
	*body1 = rand() % 65535; // random ID bewtween 0 and 65,535 (max value of unsigned 16bit integer)

	binId* body2 = static_cast<binId*>(msg + sizeof(typeof(*header)) + sizeof(typeof(*body1)));
	*body2 = binaryIdentifier;

	binId* body3 = static_cast<binId*>(msg + sizeof(typeof(*header)) + sizeof(typeof(*body1)) + sizeof(typeof(*body2)));
	for (std::list<binId>::iterator it = neighborIds.begin(); it != neighborIds.end(); ++it) {
		*body3 = *it;
		body3++;
	}

	for (HAL_UART uart : uartGateways) send(uart, msg, size);

	free(msg);
}

void LinkStateRouting::sendToAddress(binId targetAddress, const void* msg, size_t size) {}

void LinkStateRouting::handleRcvMsg(HAL_UART& uart, void* msg, const binId targetAddress, size_t size, binId& nextHopAddress, HAL_UART* nextHopGateway) {}
