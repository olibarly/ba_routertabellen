/*
 * hr_base.cpp
 *
 *  Created on: Nov 16, 2022
 *      Author: oliver
 */



#include "rodos.h"
#include "../misc/LED.h"

#include <map>
#include <list>
#include <cassert>
#include <stdlib.h>
#include "hr_mainThread_base.h"


HAL_UART uartIDX1(UART_IDX1); // working: UART_IDX1, UARTIDX2, UART_IDX6
HAL_UART uartIDX6(UART_IDX6);


HypercubeRoutingMainThread::HypercubeRoutingMainThread(binId binaryId) : StaticThread(), uartGateways{uartIDX1, uartIDX6}, nodeStateThread(&uartGateways, binaryIdentifier, &nodeStateBuffer) {
	assert(checkBinIdValid(binaryId));
	binaryIdentifier = binaryId;

	sendLED = &greenLED;
	rcvLED = &redLED;
	statusLED = &blueLED;

	//initialize auxiliary Threads
	HypercubeRoutingReceiveThread rcvThread(&uartGateways, &msgFifo);
	HypercubeRoutingAliveMsgThread aliveMsgThread(&uartGateways, binaryIdentifier);

	// TODO: Fault Tolerance: initialize NodeState Maps (issue #21)
}

HypercubeRoutingMainThread::~HypercubeRoutingMainThread() {}

bool HypercubeRoutingMainThread::checkBinIdValid(binId binaryId) {
	return binaryId != BROADCAST_ADDRESS;
}


void HypercubeRoutingMainThread::updateTable(binId binaryId, RoutingTableEntry routingTableEntry) {
	routingTable[binaryId] = routingTableEntry;
}

void HypercubeRoutingMainThread::removeOutdatedTableEntries(int64_t cleanupInterval) {

	// TODO: needs refactoring

	std::list<std::map<binId, RoutingTableEntry>::iterator> elementsToRemove;

	for (auto it = routingTable.begin(); it != routingTable.cend(); ++it) {
		it->second.ttlSeconds -= cleanupInterval;
		if (it->second.ttlSeconds <= 0) elementsToRemove.push_back(it);
	}
	for (auto e : elementsToRemove) routingTable.erase(e);
}

void HypercubeRoutingMainThread::send(HAL_UART& uart, const void* msg, size_t size) {
	sendLED->setPins(1);

	uart.write(msg, size); // strlen + 1 to ensure null terminator is also sent
}

void HypercubeRoutingMainThread::sendToAddress(binId targetAddress, const void* msgBody, size_t msgSize) {
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

void HypercubeRoutingMainThread::forwardFloodingMsg(void* msg, size_t msgSize, binId sourceId) {
	for (std::map<binId, AdjacentNode>::iterator it = adjacentNodes.begin(); it != adjacentNodes.end(); ++it) {
		if (sourceId != it->first) {
			send(*(it->second.uartGateway), msg, msgSize);
		}
	}
}



bool HypercubeRoutingMainThread::decodeRcvMsg(void* msg, binId& targetAddress, void* msgBody, size_t msgSize) {
	// TODO: check if Msg valid

	// check if msg has body
	if (msgSize <= sizeof(binId)) return false;

	// extract header and body
	targetAddress = *static_cast<binId*>(msg);
	msgBody = static_cast<binId*>(msg) + 1;

	return true;
}

void HypercubeRoutingMainThread::handleAliveMsg(HAL_UART* uart, void* msgBody) {
	binId sourceAddress = static_cast<binId*>(msgBody)[0];
	// calculation of addressing not necessery, since source node is adjacent to this node
	updateTable(sourceAddress, RoutingTableEntry(sourceAddress, uart));
}

void HypercubeRoutingMainThread::init() {
	sendLED->init(true, 1, 0);
	rcvLED->init(true, 1, 0);
	statusLED->init(true, 1, 1);

	for (HAL_UART uart : uartGateways) uart.init();
}

void HypercubeRoutingMainThread::run() {
	int64_t nextCleanUp = NOW();

	while(1) {
		size_t elementCount = msgFifo.getElementCount();
		for (int i = 0; i < elementCount; i++) {
			RcvMsg rcvMsg;
			msgFifo.get(rcvMsg);

			binId targetAddress;
			void* msgBody;
			if (decodeRcvMsg(rcvMsg.msg, targetAddress, msgBody, rcvMsg.size)) {
				handleRcvMsg(rcvMsg.uart, rcvMsg.msg, targetAddress, msgBody, rcvMsg.size);
			}
		}

		if (NOW() >= nextCleanUp) {
			int64_t  cleanupInterval = 5*SECONDS;
			removeOutdatedTableEntries(cleanupInterval); //cleanup
			nextCleanUp += cleanupInterval; // next cleanup in 5sec
			AT(nextCleanUp); // Sleep until next cleanup or until resumed
		}
	}
}
