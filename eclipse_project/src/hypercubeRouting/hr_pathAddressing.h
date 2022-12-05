/*
 * hypercube_routing2.h
 *
 *  Created on: Oct 27, 2022
 *      Author: oliver
 */

#ifndef SRC_HYPERCUBEROUTING_HR_PATHADDRESSING_H_
#define SRC_HYPERCUBEROUTING_HR_PATHADDRESSING_H_


#include <list>

#include "hr_mainThread_base.h"

class HRPathAddressing : HypercubeRoutingMainThread {
private:
	std::map<binId, RoutingTableEntry> routingTable;

	void forwardMsg(const void* msg, size_t size);

	void handleRcvMsg(HAL_UART* uart, void* msg, const binId targetAddress, void* msgBody, size_t size) override;

	void calculateAddressing(const binId targetAddress, HAL_UART* nextHopUartGateway, binId* addressing, size_t* addressingLength) override;
};


#endif /* SRC_HYPERCUBEROUTING_HR_PATHADDRESSING_H_ */
