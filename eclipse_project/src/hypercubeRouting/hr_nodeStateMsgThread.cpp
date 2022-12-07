/*
 * hr_nodeStateMsgThread.cpp
 *
 *  Created on: Dec 4, 2022
 *      Author: oliver
 */

#include "hr_nodeStateMsgThread.h"
#include <stdlib.h>


HypercubeRoutingNodeStateThread::HypercubeRoutingNodeStateThread(HAL_UART(* uartGateways)[2], binId binaryIdentifier, CommBuffer<NodeState>* nodeStateBuffer) {
	this->uartGateways = uartGateways;
	this->binaryIdentifier = binaryIdentifier;
	this->nodeStateBuffer = nodeStateBuffer;
}

void HypercubeRoutingNodeStateThread::sendNodeStateMsg() {
	/**
	 * MSG Format:
	 * Header:
	 * target address (here broadcast address) [SpaceWire header Size]
	 *
	 * Body:
	 * Broadcast Message Identifier (NODE_STATE)
	 * binary Identifier of self [binId]
	 */

	size_t size = sizeof(binId) * 2 + sizeof(broadcastIdUnderlyingType) + sizeof(nodeStateUnderlyingType);
	void* msg = malloc(size);

	// Header: Target Address
	binId* header = static_cast<binId*>(msg);
	*header = BROADCAST_ADDRESS;

	// Body:
	// broadcast msg identifier
	broadcastIdUnderlyingType* body1 = static_cast<broadcastIdUnderlyingType*>(msg + sizeof(typeof(*header)));
	*body1 = NODE_STATE;
	// floodingMsg counters
	floodingMsgCounter* body2 = static_cast<broadcastIdUnderlyingType*>(msg + sizeof(typeof(*header)) + sizeof(typeof(body1)));
	*body2 = nodeStateMsgCounter;
	// own binary Identifier/Address
	binId* body3 = static_cast<binId*>(msg + sizeof(typeof(*header)) + sizeof(typeof(body1)) + sizeof(typeof(body2)));
	*body3 = binaryIdentifier;
	// node State
	binId* body4 = static_cast<binId*>(msg + sizeof(typeof(*header)) + sizeof(typeof(body1)) + sizeof(typeof(body2)) + sizeof(typeof(body3)));
	*body4 = state;


	for (HAL_UART uart : *uartGateways) uart.write(msg, size);

	free(msg);

	nodeStateMsgCounter++;
}

void HypercubeRoutingNodeStateThread::run() {
	while(1) {
		if (nodeStateBuffer->getOnlyIfNewData(state)) {
			sendNodeStateMsg();
		}

		AT(END_OF_TIME); // suspend forever or until resumed
	}
}
