/*
 * hypercube_routing2.h
 *
 *  Created on: Oct 27, 2022
 *      Author: oliver
 */

#ifndef SRC_BASE_HYPERCUBE_ROUTING2_H_
#define SRC_BASE_HYPERCUBE_ROUTING2_H_


#include "multi_board_uart.h"
#include <list>


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
};

class HypercubeRouting2 : MultiBoardUART {
private:
	std::map<binId, HypercubeRouting2RoutingTableEntry> routingTable;

	uint8_t floodingMsgCounter = 0;
	std::list<FloodingMsgData> floodingmsgs;

	void sendFloodingMsg();
	void sendToAddress(binId targetAddress, const void* msg, size_t msgSize) override;
	void forwardMsg(const void* msg, size_t msgSize);
	void forwardFloodingMsg(HAL_UART* receivingGateway, const void* msg, size_t size);

	void handleRcvMsg(HAL_UART* uart, void* msg, const binId targetAddress, void* msgBody, size_t size) override;

	void calculateHopChainFromTargetAddress(const binId targetAddress, std::list<binId>& binIdChain);
};


#endif /* SRC_BASE_HYPERCUBE_ROUTING2_H_ */
