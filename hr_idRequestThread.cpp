/**
 * @file hr_idRequestThread.cpp
 * @date 2023/01/18
 * @author Oliver Gerst
 *
 * @brief Part of Hypercube Routing, responsible for requesting a binary Identifier.
 * 		  Will be suspended after a certain amount of time.
 */

#include "hr_idRequestThread.h"
#include "rodos.h"

HypercubeRoutingIdRequestThread::HypercubeRoutingIdRequestThread(Semaphore* uartSemaphore, HAL_UART(* uartGateways)[2], HAL_GPIO* statusLED) : StaticThread("IdRequestThread") {
	this->statusLED = statusLED;

	this->uartSemaphore = uartSemaphore;
	this->uartGateways = uartGateways;
}

HypercubeRoutingIdRequestThread::~HypercubeRoutingIdRequestThread() {}


void HypercubeRoutingIdRequestThread::sendRequest() {
	PROTECT_IN_SCOPE(*uartSemaphore);

	IdRequestMsg msg;
	for (HAL_UART uart : *uartGateways) uart.write(&msg, sizeof(msg));
}

void HypercubeRoutingIdRequestThread::init() {}

void HypercubeRoutingIdRequestThread::run() {
	PRINTF("IDRequest: run start\n");

	// Send ID Request 10 times in 30 seconds (issue 32)
	for (int i = 0; i < 10; i++) {
		statusLED->setPins(~statusLED->readPins());
		sendRequest();
		PRINTF("IDRequest: request sent\n");
		AT(NOW() + 3*SECONDS);
	}

	AT(END_OF_TIME);

}
