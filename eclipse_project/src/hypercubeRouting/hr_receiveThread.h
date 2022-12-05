/*
 * hr_receiveThread.h
 *
 *  Created on: Dec 5, 2022
 *      Author: oliver
 */

#ifndef SRC_HYPERCUBEROUTING_HR_RECEIVETHREAD_H_
#define SRC_HYPERCUBEROUTING_HR_RECEIVETHREAD_H_

#include "rodos.h"
#include "types_and_structs.h"

class HypercubeRoutingReceiveThread : StaticThread<> {
public:
	HypercubeRoutingReceiveThread(HAL_UART(* uartGateways)[2], Fifo<RcvMsg, 10>* msgFifo);

protected:
	Fifo<RcvMsg, 10>* msgFifo;
	HAL_UART(* uartGateways)[2];

	size_t receive(HAL_UART* uart, void* rcvBuffer, const size_t maxLen = 100);
	void receiveMsgs();

private:
	void init();
	void run();
};

#endif /* SRC_HYPERCUBEROUTING_HR_RECEIVETHREAD_H_ */
