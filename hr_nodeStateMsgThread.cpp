/*
 * hr_nodeStateMsgThread.cpp
 *
 *  Created on: Dec 4, 2022
 *      Author: oliver
 */

#include "hr_nodeStateMsgThread.h"
#include <stdlib.h>


HypercubeRoutingNodeStateThread::HypercubeRoutingNodeStateThread(Semaphore* uartSemaphore, HAL_UART(* uartGateways)[2], CommBuffer<NodeState>* nodeStateBuffer, HAL_GPIO* statusLED) : StaticThread("NodeStateThread") {
	this->statusLED = statusLED;

	this->uartSemaphore = uartSemaphore;
	this->uartGateways = uartGateways;
	this->nodeStateBuffer = nodeStateBuffer;
}

HypercubeRoutingNodeStateThread::~HypercubeRoutingNodeStateThread() {}

void HypercubeRoutingNodeStateThread::setBinaryIdAndResume(binId binaryId) {
	this->binaryIdentifier = binaryId;
	resume();
}

void HypercubeRoutingNodeStateThread::sendNodeStateMsg() {
	PROTECT_IN_SCOPE(*uartSemaphore);

	NodeStateMsg nodeStateMsg = NodeStateMsg(binaryIdentifier, state);
	size_t size = sizeof(nodeStateMsg);
	void* msg = &nodeStateMsg;

	for (HAL_UART uart : *uartGateways) uart.write(msg, size);
}

void HypercubeRoutingNodeStateThread::init() {}

void HypercubeRoutingNodeStateThread::run() {
	PRINTF("NodeState: run start\n");

	AT(END_OF_TIME); // suspend until id is set and thread is resumed
	sendNodeStateMsg();

	PRINTF("NodeState: msg sent\n");

	while(1) {
		if (nodeStateBuffer->getOnlyIfNewData(state)) {
			statusLED->setPins(1);
			sendNodeStateMsg();
			PRINTF("NodeState: msg sent\n");
			statusLED->setPins(0);
		}

		AT(NOW() + 5*SECONDS); // suspend for 5 seconds or until resumed
	}
}
