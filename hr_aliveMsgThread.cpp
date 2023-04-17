/*
 * hr_aliveMsgThread.cpp
 *
 *  Created on: Dec 4, 2022
 *      Author: oliver
 */

#include "hr_aliveMsgThread.h"
#include <stdlib.h>



HypercubeRoutingAliveMsgThread::HypercubeRoutingAliveMsgThread(Semaphore* uartSemaphore, HAL_UART(* uartGateways)[2], HAL_GPIO* statusLED) : StaticThread("AliveMsgThread") {
	this->statusLED = statusLED;

	this->uartSemaphore = uartSemaphore;
	this->uartGateways = uartGateways;
}

HypercubeRoutingAliveMsgThread::~HypercubeRoutingAliveMsgThread() {}

void HypercubeRoutingAliveMsgThread::setBinaryIdAndResume(binId binaryId) {
	this->binaryIdentifier = binaryId;
	resume();
}

void HypercubeRoutingAliveMsgThread::sendAliveMsg() {
	PROTECT_IN_SCOPE(*uartSemaphore);

	AliveMsg aliveMsg = AliveMsg(aliveMsgCounter, binaryIdentifier);
	size_t size = sizeof(aliveMsg);
	void* msg = &aliveMsg;

	for (HAL_UART uart : *uartGateways) uart.write(msg, size);

	aliveMsgCounter++;
}

void HypercubeRoutingAliveMsgThread::init() {}

void HypercubeRoutingAliveMsgThread::run() {
	PRINTF("AliveMsg: run start\n");
	AT(END_OF_TIME); //sleep until id is set and thread is resumed

	TIME_LOOP(0, 5*SECONDS) {
		statusLED->setPins(~statusLED->readPins());
		sendAliveMsg();
		PRINTF("AliveMsg: msg sent\n");
	}
}
