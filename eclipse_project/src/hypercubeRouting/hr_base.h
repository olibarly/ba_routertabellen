/*
 * hr_base.h
 *
 *  Created on: Nov 16, 2022
 *      Author: oliver
 */

#ifndef SRC_HYPERCUBEROUTING_HR_BASE_H_
#define SRC_HYPERCUBEROUTING_HR_BASE_H_

#include "rodos.h"
#include <map>
#include <list>


typedef uint8_t binId;
typedef uint8_t broadcastId;
typedef uint8_t nodeStateUnderlying;
#define BROADCAST_ADDRESS		0b11111111

enum BroadcastIdentifier : broadcastId {
	ALIVE,
	NODE_STATE
};

enum NodeState : nodeStateUnderlying {
	SAFE,
	FAULTY,
	UNSAFE,
	STRONGLY_UNSAFE
};

struct UART_Gateway {
	uint8_t id;
	HAL_UART hal_uart;
};

struct RoutingTableEntry {
	RoutingTableEntry();
	RoutingTableEntry(binId t, HAL_UART* u) {
		RoutingTableEntry(t, &t, 1, u);
	}
	RoutingTableEntry(binId t, binId* a, size_t l, HAL_UART* u, uint8_t ttl = 60) {
		nodeAddress = t;
		a = a;
		l = l;
		uartGateway = u;
		ttlSeconds = ttl;
		state = SAFE;
	}
	virtual ~RoutingTableEntry() {};

	void resetTTL() {
		ttlSeconds = 60;
	}

	binId nodeAddress;
	binId* addressing;
	size_t addressingLength;
	HAL_UART* uartGateway;
	uint8_t ttlSeconds;
	NodeState state;
};

struct AdjacentNode {
	AdjacentNode();
	AdjacentNode(binId ownAddress, binId adjacentNodeAddress, HAL_UART* gateway, NodeState state) {
		nodeAddress = adjacentNodeAddress;
		uartGateway = gateway;
		dimOfLink = ownAddress ^ adjacentNodeAddress;
		this->state = state;
	}

	binId nodeAddress;
	HAL_UART* uartGateway;
	binId dimOfLink; // specifies the dimension of the connection between node and adjacentNode, equals ID_Of_Node XOR ID_Of_AdjacentNode, e.g. 0b00000100 for a link in third dimension
	NodeState state;
};


class HypercubeRouting : StaticThread<> {
public:
	HypercubeRouting(binId binaryId); //allows up to 256 different IDs, with 256 being used as a "broadcast" address

protected:
	HAL_UART uartGateways[2];

	binId binaryIdentifier;
	NodeState state;

	std::map<binId, RoutingTableEntry> routingTable;
	std::map<binId, AdjacentNode> adjacentNodes;
	std::map<binId, AdjacentNode> safeAdjacentNodes;
	std::map<binId, AdjacentNode> unsafeAdjacentNodes;
	std::map<binId, AdjacentNode> stronglyUnsafeAdjacentNodes;
	std::map<binId, HAL_UART*> faultyOrNonExistingNodes;
	std::map<NodeState, std::map<binId, AdjacentNode>> adjacentNodesPerState;

	bool checkBinIdValid(binId BinaryId);

	void updateTable(binId binaryId, RoutingTableEntry routingTableEntry);
	/**
	 * Given the target address [targetAddress] the function determines the UART Gateway [nextHopUartGateway] over which the message will be forwarded.
	 * Additionally it creates the header [addressing], depending on the addressing method, for said message and gives the header's length [addressingLength].
	 */
	virtual void calculateAddressing(const binId targetAddress, HAL_UART* nextHopUartGateway, binId* addressing, size_t* addressingLength);

	void send(HAL_UART& uart, const void* msg, size_t size);
	void sendAliveMsg();
	void sendToAddress(binId targetAddress, const void* msgBody, size_t msgBodySize);

	size_t receive(HAL_UART& uart, void* rcvBuffer, const size_t maxLen = 100);
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



#endif /* SRC_HYPERCUBEROUTING_HR_BASE_H_ */
