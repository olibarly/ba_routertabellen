/*
 * hypercube_routing.h
 *
 *  Created on: Oct 18, 2022
 *      Author: oliver
 */

#ifndef SRC_HYPERCUBEROUTING_HR_LOGICALADDRESSING_H_
#define SRC_HYPERCUBEROUTING_HR_LOGICALADDRESSING_H_



#include "../hypercubeRouting/hr_base.h"

class HRLogicalAddressing : HypercubeRouting {
private:
	/**
	 * @param
	 * [size] includes header (targetAddress) and msg (body)
	 */
	void handleRcvMsg(HAL_UART* uart, void* msg, const binId targetAddress, void* msgBody, size_t size) override;

	void calculateAddressing(const binId targetAddress, HAL_UART* nextHopUartGateway, binId* addressing, size_t* addressingLength) override;
	/**
	 * Takes the node state broadcast message [msgBody] and the gateway [uartGateway] via which it was received.
	 * Moves AdjacenNode Entry to correct map ([safeAdjacentNodes], [unsafeAdjacentNodes], [stronglyUnsafeAdjacentNodes], [faultyAdjacentNodes]).
	 * Sets [state] of self according to new neighbor states.
	 */
	void calculateNodeState(HAL_UART* uartGateway, void* msgBody);
	void sendNodeStateMsg();
};



#endif /* SRC_HYPERCUBEROUTING_HR_LOGICALADDRESSING_H_ */
