/*
 * hr_receiveThread.cpp
 *
 *  Created on: Dec 5, 2022
 *      Author: oliver
 */

#include "hr_receiveThread.h"

HypercubeRoutingReceiveThread::HypercubeRoutingReceiveThread(HAL_UART(* uartGateways)[2], Fifo<RcvMsg, 10>* msgFifo) {
	this->msgFifo = msgFifo;
	this->uartGateways = uartGateways;
}

HypercubeRoutingReceiveThread::~HypercubeRoutingReceiveThread() {}


size_t HypercubeRoutingReceiveThread::receive(HAL_UART* uart, void* rcvBuffer, const size_t maxLen /* = 100*/) {
	if (uart->isDataReady()) return uart->read(rcvBuffer, maxLen);
	else return 0;
}

void HypercubeRoutingReceiveThread::receiveMsgs() {
	for (HAL_UART uart : *uartGateways) {
		void* rcvBuffer;
		size_t rcvSize = receive(&uart, rcvBuffer);
		if (rcvSize > 0) {
			msgFifo->put(RcvMsg(&uart, rcvBuffer, rcvSize));
		}
	}
}

void HypercubeRoutingReceiveThread::init() {}

void HypercubeRoutingReceiveThread::run() {
	while(1) receiveMsgs();
}
