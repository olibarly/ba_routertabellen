/*
 * hypercube_routing2.h
 *
 *  Created on: Oct 27, 2022
 *      Author: oliver
 */

#ifndef SRC_HYPERCUBEROUTING_HR_PATHADDRESSING_H_
#define SRC_HYPERCUBEROUTING_HR_PATHADDRESSING_H_


#include <list>

#include "../hypercubeRouting/hr_base.h"


struct FloodingMsgData{
	FloodingMsgData(binId n, uint8_t f) {
		nodeId = n;
		floodingId = f;
	}

	binId nodeId;
	uint8_t floodingId;
	uint8_t ttlSeconds = 15;
};

struct HypercubeRouting2RoutingTableEntry : RoutingTableEntry {
	std::list<binId> binIdChain;
	uint8_t ttlSeconds;
};

class HypercubeRouting2 : MultiBoardUART {
private:
	std::map<binId, HypercubeRouting2RoutingTableEntry> routingTable;

	uint8_t floodingMsgCounter = 0;
	std::list<FloodingMsgData> floodingmsgs;

	void sendFloodingMsg();
	void sendToAddress(binId targetAddress, const void* msgBody, size_t msgBodySize) override;
	void forwardMsg(const void* msg, size_t size);
	void forwardFloodingMsg(HAL_UART* receivingGateway, const void* msg, size_t size);

	void handleRcvMsg(HAL_UART* uart, void* msg, const binId targetAddress, void* msgBody, size_t size) override;

	void calculateHopChainFromTargetAddress(const binId targetAddress, std::list<binId>& binIdChain);
};


#endif /* SRC_HYPERCUBEROUTING_HR_PATHADDRESSING_H_ */
