/*
 * hr_base.h
 *
 *  Created on: Nov 16, 2022
 *      Author: oliver
 */

#ifndef SRC_HYPERCUBEROUTING_HR_MAINTHREAD_BASE_H_
#define SRC_HYPERCUBEROUTING_HR_MAINTHREAD_BASE_H_

#include "rodos.h"
#include <map>
#include <list>

#include "hr_receiveThread.h"
#include "hr_aliveMsgThread.h"
#include "hr_nodeStateMsgThread.h"
#include "types_and_structs.h"



class HypercubeRoutingMainThread : StaticThread<> {
public:
	HypercubeRoutingMainThread(binId binaryId); //allows up to 256 different IDs, with 256 being used as a "broadcast" address

protected:
	HypercubeRoutingNodeStateThread nodeStateThread;

	HAL_UART uartGateways[2];

	binId binaryIdentifier;
	NodeState state;
	CommBuffer<NodeState> nodeStateBuffer;

	Fifo<RcvMsg, 10> msgFifo;

	std::map<binId, RoutingTableEntry> routingTable;
	std::map<binId, AdjacentNode> adjacentNodes;
	std::map<binId, AdjacentNode> safeAdjacentNodes;
	std::map<binId, AdjacentNode> unsafeAdjacentNodes;
	std::map<binId, AdjacentNode> stronglyUnsafeAdjacentNodes;
	std::map<binId, HAL_UART*> faultyOrNonExistingNodes;
	std::map<NodeState, std::map<binId, AdjacentNode>> adjacentNodesPerState;

	bool checkBinIdValid(binId BinaryId);

	void updateTable(binId binaryId, RoutingTableEntry routingTableEntry);
	void removeOutdatedTableEntries();
	/**
	 * Given the target address [targetAddress] the function determines the UART Gateway [nextHopUartGateway] over which the message will be forwarded.
	 * Additionally it creates the header [addressing], depending on the addressing method, for said message and gives the header's length [addressingLength].
	 */
	virtual void calculateAddressing(const binId targetAddress, HAL_UART* nextHopUartGateway, binId* addressing, size_t* addressingLength);

	void send(HAL_UART& uart, const void* msg, size_t size);
	void sendToAddress(binId targetAddress, const void* msgBody, size_t msgBodySize);

	void decodeRcvMsg(void* msg, binId& targetAddress, void* msgBody);
	/**
	* @param size includes header (nodeAddress) and msgBody (body)
	*/
	virtual void handleRcvMsg(HAL_UART* uart, void* msg, const binId targetAddress, void* msgBody, size_t size);
	void handleAliveMsg(HAL_UART* uart, void* msgBody);

private:
	HAL_GPIO* sendLED;
	HAL_GPIO* rcvLED;
	HAL_GPIO* statusLED;
	void init();
	void run();
};


#endif /* SRC_HYPERCUBEROUTING_HR_MAINTHREAD_BASE_H_ */
