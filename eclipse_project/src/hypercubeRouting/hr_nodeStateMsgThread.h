/*
 * hr_nodeStateMsgThread.h
 *
 *  Created on: Dec 4, 2022
 *      Author: oliver
 */

#ifndef SRC_HYPERCUBEROUTING_HR_NODESTATEMSGTHREAD_H_
#define SRC_HYPERCUBEROUTING_HR_NODESTATEMSGTHREAD_H_

#include "types_and_structs.h"
#include "rodos.h"


class HypercubeRoutingNodeStateThread : StaticThread<> {
public:
	HypercubeRoutingNodeStateThread(HAL_UART(* uartGateways)[2], binId id, CommBuffer<NodeState>* commBuff);

	void resume() {
		StaticThread::resume();
	}

protected:
	HAL_UART(* uartGateways)[2];
	binId binaryIdentifier;
	CommBuffer<NodeState>* nodeStateBuffer;
	NodeState state;

	void sendNodeStateMsg();

private:
	void init();
	void run();
};


#endif /* SRC_HYPERCUBEROUTING_HR_NODESTATEMSGTHREAD_H_ */
