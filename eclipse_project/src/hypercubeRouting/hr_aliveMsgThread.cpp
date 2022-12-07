/*
 * hr_aliveMsgThread.cpp
 *
 *  Created on: Dec 4, 2022
 *      Author: oliver
 */

#include "hr_aliveMsgThread.h"
#include <stdlib.h>


HypercubeRoutingAliveMsgThread::HypercubeRoutingAliveMsgThread(HAL_UART(* uartGateways)[2], binId binaryIdentifier) {
	this->uartGateways = uartGateways;
	this->binaryIdentifier = binaryIdentifier;
}

void HypercubeRoutingAliveMsgThread::sendAliveMsg() {
	// TODO: Add Msg Counter for Flooding Msg Identification
	/**
	 * MSG Format:
	 * Header:
	 * target address (here broadcast address) [SpaceWire header Size]
	 *
	 * Body:
	 * Broadcast Message Identifier (ALIVE)
	 * Alive Message Counter
	 * binary Identifier of self [binId]
	 */

	size_t size = sizeof(binId) * 2 + sizeof(broadcastId);
	void* msg = malloc(size);
	// Header: Target Address
	binId* header = static_cast<binId*>(msg);
	*header = BROADCAST_ADDRESS;

	// Body:
	// Broadcast Msg Id
	broadcastId* body1 = static_cast<broadcastId*>(msg + sizeof(typeof(*header)));
	*body1 = ALIVE;
	// AliveMsg Counter
	floodingMsgCounter* body2 = static_cast<broadcastId*>(msg + sizeof(typeof(*header)) + sizeof(typeof(body1)));
	*body2 = aliveMsgCounter;
	// own binary Identifier/Address
	binId* body3 = static_cast<binId*>(msg + sizeof(typeof(*header)) + sizeof(typeof(body1)) + sizeof(typeof(body2)));
	*body3 = binaryIdentifier;

	for (HAL_UART uart : *uartGateways) uart.write(msg, size);

	free(msg);

	aliveMsgCounter++;
}

void HypercubeRoutingAliveMsgThread::run() {
	TIME_LOOP(0, 5*SECONDS) {
		sendAliveMsg();
	}
}
