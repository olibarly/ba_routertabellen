/*
 * hr_receiveThread.cpp
 *
 *  Created on: Dec 5, 2022
 *      Author: oliver
 */

#include "hr_receiveThread.h"

HypercubeRoutingReceiveThread::HypercubeRoutingReceiveThread(Semaphore* uartSemaphore, HAL_UART(* uartGateways)[2], Fifo<MsgData, 10>* msgFifo, HAL_GPIO* statusLED) : StaticThread("ReceiveThread") {
	this->statusLED = statusLED;

	this->uartSemaphore = uartSemaphore;
	this->msgFifo = msgFifo;
	this->uartGateways = uartGateways;
}

HypercubeRoutingReceiveThread::~HypercubeRoutingReceiveThread() {}


size_t HypercubeRoutingReceiveThread::receive(HAL_UART* uart, void* rcvBuffer, const size_t maxLen /* = 100*/) {
	PROTECT_IN_SCOPE(*uartSemaphore);

	if (uart->isDataReady()) {
		statusLED->setPins(1);
		size_t readLen = uart->read(rcvBuffer, maxLen);
		statusLED->setPins(0);

		return readLen;
	}
	else return 0;
}

void HypercubeRoutingReceiveThread::receiveMsgs() {
	for (HAL_UART uart : *uartGateways) {
		void* rcvBuffer;
		size_t rcvSize = receive(&uart, rcvBuffer);
		if (rcvSize > 0) {
			msgFifo->put(MsgData(&uart, rcvBuffer, rcvSize));
			PRINTF("Receiver: received: %u, %x\n", rcvSize, rcvBuffer);
		}
	}
}

void HypercubeRoutingReceiveThread::init() {}

void HypercubeRoutingReceiveThread::run() {
	PRINTF("Receiver: run start\n");

	while(1) {
		receiveMsgs();
	}
}
