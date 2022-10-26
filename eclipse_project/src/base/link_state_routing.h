/*
 * link_state_routing.h
 *
 *  Created on: Oct 24, 2022
 *      Author: oliver
 */

#ifndef SRC_BASE_LINK_STATE_ROUTING_H_
#define SRC_BASE_LINK_STATE_ROUTING_H_

#include "multi_board_uart.h"

struct LinkStateRoutingTableEntry : RoutingTableEntry {
	uint8_t hopDistance;
};

class LinkStateRouting : MultiBoardUART {
private:
	std::map<binId, std::list<binId>> neigborsPerId; // information gathered through AliveMsgs; Neighbors of each known node
	std::map<uint8_t, uint8_t> floodingIds; // flooding Msg ID : TTL in seconds

	void sendAliveMsg() override;
	void sendToAddress(binId targetAddress, const void* msg, size_t size) override;

	void handleRcvMsg(HAL_UART& uart, void* msg, const binId targetAddress, size_t size, binId& nextHopAddress, HAL_UART* nextHopGateway) override;

};


#endif /* SRC_BASE_LINK_STATE_ROUTING_H_ */
