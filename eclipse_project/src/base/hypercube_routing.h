/*
 * hypercube_routing.h
 *
 *  Created on: Oct 18, 2022
 *      Author: oliver
 */

#ifndef SRC_BASE_HYPERCUBE_ROUTING_H_
#define SRC_BASE_HYPERCUBE_ROUTING_H_



#include "multi_board_uart.h"

class HypercubeRouting : MultiBoardUART {
private:
	void sendAliveMsg() override;
	void sendToAddress(binId targetAddress, const void* msg, size_t size) override;
	/**
	 * @param
	 * [size] includes header (targetAddress) and msg (body)
	 */
	void handleRcvMsg(HAL_UART& uart, void* msg, const binId targetAddress, size_t size, binId& nextHopAddress, HAL_UART* nextHopGateway) override;

	void calculateNextHopFromTargetAddress(const binId targetAddress, HAL_UART* nextHopUartGateway, binId nextHopAddress);
};



#endif /* SRC_BASE_HYPERCUBE_ROUTING_H_ */
