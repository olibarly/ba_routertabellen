/*
 * types_and_structs.h
 *
 *  Created on: Dec 4, 2022
 *      Author: oliver
 */

#ifndef SRC_HYPERCUBEROUTING_TYPES_AND_STRUCTS_H_
#define SRC_HYPERCUBEROUTING_TYPES_AND_STRUCTS_H_

#include "rodos.h"


typedef uint8_t binId;
typedef uint8_t broadcastIdUnderlyingType;
typedef uint8_t floodingMsgCounter;
typedef uint8_t nodeStateUnderlyingType;
#define BROADCAST_ADDRESS		0b11111111

enum BroadcastIdentifier : broadcastIdUnderlyingType {
	ALIVE,
	NODE_STATE
};

enum NodeState : nodeStateUnderlyingType {
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

struct RcvMsg {
	RcvMsg();
	RcvMsg(HAL_UART* u, void* m, size_t s) {
		uart = u;
		msg = m;
		size = s;
	}

	HAL_UART* uart;
	void* msg;
	size_t size;
};


#endif /* SRC_HYPERCUBEROUTING_TYPES_AND_STRUCTS_H_ */
