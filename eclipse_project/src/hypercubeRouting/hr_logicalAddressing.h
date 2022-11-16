/*
 * hypercube_routing.h
 *
 *  Created on: Oct 18, 2022
 *      Author: oliver
 */

#ifndef SRC_HYPERCUBEROUTING_HR_LOGICALADDRESSING_H_
#define SRC_HYPERCUBEROUTING_HR_LOGICALADDRESSING_H_



#include "../hypercubeRouting/hr_base.h"

class HypercubeRouting : MultiBoardUART {
private:
	void sendToAddress(binId targetAddress, const void* msgBody, size_t msgBodySize) override;
	/**
	 * @param
	 * [size] includes header (targetAddress) and msg (body)
	 */
	void handleRcvMsg(HAL_UART* uart, void* msg, const binId targetAddress, void* msgBody, size_t size) override;

	void calculateNextHopFromTargetAddress(const binId targetAddress, HAL_UART* nextHopUartGateway, binId nextHopAddress) override;
};



#endif /* SRC_HYPERCUBEROUTING_HR_LOGICALADDRESSING_H_ */
