/*
 * hr_base.cpp
 *
 *  Created on: Nov 16, 2022
 *      Author: oliver
 */



#include "../hypercubeRouting/hr_base.h"

#include "rodos.h"
#include "../misc/LED.h"

#include <map>
#include <list>
#include <cassert>
#include <stdlib.h>


HAL_UART uartIDX1(UART_IDX1); // working: UART_IDX1, UARTIDX2, UART_IDX6
HAL_UART uartIDX6(UART_IDX6);


HypercubeRouting::HypercubeRouting(binId binaryId) : StaticThread(), uartGateways{uartIDX1, uartIDX6}{
	assert(checkBinIdValid(binaryId));
	binaryIdentifier = binaryId;

	sendLED = &greenLED;
	rcvLED = &redLED;
	statusLED = &blueLED;

	// TODO: Fault Tolerance: initialize NodeState Maps (issue #21)
}

bool HypercubeRouting::checkBinIdValid(binId binaryId) {
	return binaryId != BROADCAST_ADDRESS;
}


void HypercubeRouting::updateTable(binId binaryId, RoutingTableEntry routingTableEntry) {
	routingTable[binaryId] = routingTableEntry;
}

void HypercubeRouting::send(HAL_UART& uart, const void* msg, size_t size) {
	sendLED->setPins(1);

	uart.write(msg, size); // strlen + 1 to ensure null terminator is also sent
}

void HypercubeRouting::sendAliveMsg() {
	/**
	 * MSG Format:
	 * Header:
	 * target address (here broadcast address) [SpaceWire header Size]
	 *
	 * Body:
	 * Broadcast Message Identifier (ALIVE)
	 * binary Identifier of self [binId]
	 */

	size_t size = sizeof(binId) * 2 + sizeof(broadcastId);
	void* msg = malloc(size);
	// Header: Target Address
	binId* header = static_cast<binId*>(msg);
	*header = BROADCAST_ADDRESS;
	// Body:
	broadcastId* body1 = static_cast<broadcastId*>(msg + sizeof(typeof(*header)));
	*body1 = ALIVE;
	// own binary Identifier/Address
	binId* body2 = static_cast<binId*>(msg + sizeof(typeof(*header)) + sizeof(typeof(body1)));
	*body2 = binaryIdentifier;

	for (HAL_UART uart : uartGateways) send(uart, msg, size);

	free(msg);
}

void HypercubeRouting::sendToAddress(binId targetAddress, const void* msgBody, size_t msgSize) {
	// check if address is in routing table
	std::map<binId, RoutingTableEntry>::iterator routingTableMapEntry = routingTable.find(targetAddress);
	RoutingTableEntry routingData;

	// if not: calculate addressing, updateTable
	if (routingTableMapEntry == routingTable.end()) {
		HAL_UART* nextHopUartGateway;
		binId* addressing;
		size_t addressingLength;
		calculateAddressing(targetAddress, nextHopUartGateway, addressing, &addressingLength);
		updateTable(targetAddress, RoutingTableEntry(targetAddress, addressing, addressingLength, nextHopUartGateway));
	} else { //otherwise: just use data from table
		routingData = routingTableMapEntry->second;
	}

	size_t size = sizeof(binId) * routingData.addressingLength + msgSize;

	void* msg = malloc(size);

	binId* addressing = static_cast<binId*>(msg);
	memcpy(addressing, routingData.addressing, routingData.addressingLength);

	void* body = msg + sizeof(typeof(*addressing)) * routingData.addressingLength;
	memcpy(body, msg, msgSize);

	send(*routingData.uartGateway, msg, size);

	free(msg);
}

size_t HypercubeRouting::receive(HAL_UART& uart, void* rcvBuffer, const size_t maxLen /* = 100*/) {
	if (uart.isDataReady()) {
		rcvLED->setPins(1);

		size_t readLen = uart.read(rcvBuffer, maxLen);

		return readLen;
	}
	return 0;
}

void HypercubeRouting::decodeRcvMsg(void* msg, binId& targetAddress, void* msgBody) {
	targetAddress = *static_cast<binId*>(msg);
	msgBody = static_cast<binId*>(msg) + 1;
}

void HypercubeRouting::handleAliveMsg(HAL_UART* uart, void* msgBody) {
	binId sourceAddress = static_cast<binId*>(msgBody)[0];
	// calculation of addressing not necessery, since source node is adjacent to this node
	updateTable(sourceAddress, RoutingTableEntry(sourceAddress, uart));
}

void HypercubeRouting::init() {
	sendLED->init(true, 1, 0);
	rcvLED->init(true, 1, 0);
	statusLED->init(true, 1, 1);

	for (HAL_UART uart : uartGateways) uart.init();
}

void HypercubeRouting::run() {
	int sendIntervalCounter = 0;
	int sendAliveIntervalCounter = 0;

	int64_t sendAliveInterval = 5*SECONDS;
	int64_t loopInterval = 50*MILLISECONDS;

	uint32_t statusPinVal = 0;


	sendAliveMsg();

	TIME_LOOP(0*SECONDS, loopInterval) {
		sendLED->setPins(0);
		rcvLED->setPins(0);
		statusLED->setPins(statusPinVal);

		std::list<std::map<binId, RoutingTableEntry>::iterator> elementsToRemove;

		for (auto it = routingTable.begin(); it != routingTable.cend(); ++it) {
			it->second.ttlSeconds -= loopInterval;
			if (it->second.ttlSeconds <= 0) elementsToRemove.push_back(it);
		}
		for (auto e : elementsToRemove) routingTable.erase(e);


		if (sendAliveIntervalCounter >= sendAliveInterval / loopInterval) {
			sendAliveIntervalCounter = 0;
			sendAliveMsg();
		}

		for (HAL_UART uart : uartGateways) {
			void* rcvBuffer;
			size_t rcvSize = receive(uart, rcvBuffer);
			if (rcvSize != 0) {
				binId targetAddress;
				void* msgBody;
				decodeRcvMsg(rcvBuffer, targetAddress, msgBody);

				handleRcvMsg(&uart, rcvBuffer, targetAddress, msgBody, rcvSize);
			}
		}

		statusPinVal ^= 1;
		sendIntervalCounter++;
		sendAliveInterval++;
	}
}

