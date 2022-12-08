/*routing
 * hr_AliveMsgThread.h
 *
 *  Created on: Dec 4, 2022
 *      Author: oliver
 */

#ifndef SRC_HYPERCUBEROUTING_HR_ALIVEMSGTHREAD_H_
#define SRC_HYPERCUBEROUTING_HR_ALIVEMSGTHREAD_H_

#include "rodos.h"
#include "types_and_structs.h"

class HypercubeRoutingAliveMsgThread : StaticThread<> {
public:
	HypercubeRoutingAliveMsgThread(HAL_UART(* uartGateways)[2], binId id);
	virtual ~HypercubeRoutingAliveMsgThread();

protected:
	HAL_UART(* uartGateways)[2];
	binId binaryIdentifier;
	floodingMsgCounter aliveMsgCounter = 0;

	void sendAliveMsg();

private:
	void init();
	void run();
};


#endif /* SRC_HYPERCUBEROUTING_HR_ALIVEMSGTHREAD_H_ */
