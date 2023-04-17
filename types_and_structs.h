/*
 * types_and_structs.h
 *
 *  Created on: Dec 4, 2022
 *      Author: oliver
 */

#ifndef SRC_HYPERCUBEROUTING_TYPES_AND_STRUCTS_H_
#define SRC_HYPERCUBEROUTING_TYPES_AND_STRUCTS_H_

#include "rodos.h"


typedef uint8_t binId; //allows up to 256 different IDs, with 256 being used as a "broadcast" address
typedef uint8_t msgIdUnderlyingType;
typedef uint8_t floodingMsgCounter;
typedef uint8_t nodeStateUnderlyingType;

#define BROADCAST_ADDRESS	0b11111111
#define PORTS_PER_NODE		2
#define MSG_BODY_MAX_LENGTH	32 // Bytes

const int BIT_SIZE_OF_ID = sizeof(binId) * 8; // equals maximum amount of dimensions

enum MessageIdentifier : msgIdUnderlyingType {
	ALIVE,
	NODE_STATE,
	ID_REQUEST,
	ID_REQUEST_RESPONSE
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
	RoutingTableEntry() {}
	RoutingTableEntry(binId t, HAL_UART* u, int64_t toD = NOW() + 60*SECONDS) {
		nodeAddress = t;
		uartGateway = u;
		timeOfDeletion = toD;
		state = SAFE;
	}
	virtual ~RoutingTableEntry() {}

	void resetToD() {
		timeOfDeletion = NOW() + 60*SECONDS;
	}

	binId nodeAddress;
	HAL_UART* uartGateway;
	int64_t timeOfDeletion;
	NodeState state;
};

struct AdjacentNode {
	AdjacentNode() {}
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

struct MsgData {
	MsgData() {}
	MsgData(HAL_UART* u, void* m, size_t s) {
		uart = u;
		msg = m;
		size = s;
	}

	HAL_UART* uart;
	void* msg;
	size_t size;
};

/**
 * !!Should not be used!!
 *
 * This struct only exists,
 * so that LogicalAddressingMsg and PathAddressingMsg can inherit from it
 */
struct Msg {};

struct LogicalAddressingMsg : Msg{
	binId targetId;
	uint8_t msgBody[MSG_BODY_MAX_LENGTH];
};

struct PathAddressingMsg : Msg{
	binId targetId[BIT_SIZE_OF_ID];
	uint8_t msgBody[MSG_BODY_MAX_LENGTH];
};

struct BroadcastMsg {
	binId targetId;
};

struct AliveMsg {
	AliveMsg() {}
	AliveMsg(floodingMsgCounter msgCounter, binId nodeId) {
		this->msgCounter = msgCounter;
		this->nodeId = nodeId;
	}

	binId targetId = BROADCAST_ADDRESS;
	MessageIdentifier broadcastId = ALIVE;
	floodingMsgCounter msgCounter;
	binId nodeId;
};

struct NodeStateMsg {
	NodeStateMsg() {}
	NodeStateMsg(binId nodeId, NodeState state) {
		this->nodeId = nodeId;
		this->state = state;
	}

	binId targetId = BROADCAST_ADDRESS;
	MessageIdentifier broadcastId = NODE_STATE;
	binId nodeId;
	NodeState state;
};

struct IdRequestMsg {
	binId targetId = BROADCAST_ADDRESS;
	MessageIdentifier broadcastId = ID_REQUEST;
};

struct IdRequestResponseMsg {
	IdRequestResponseMsg() {}
	IdRequestResponseMsg(binId sourceId, uint8_t possibleIdCount, binId* possibleIds) {
		this->sourceId = sourceId;
		this->possibleIdCount = possibleIdCount;
		for (int i = 0; i < possibleIdCount; i++) {
			this->possibleIds[i] = possibleIds[i];
		}
	}

	binId targetId = BROADCAST_ADDRESS;
	MessageIdentifier broadcastId = ID_REQUEST;
	binId sourceId;
	uint8_t possibleIdCount;
	binId possibleIds[BIT_SIZE_OF_ID]; // size = maximum amount of possible IDs = maximum amount of dimensions = size of [binId] in Bytes * 8 bits per Byte
};


#endif /* SRC_HYPERCUBEROUTING_TYPES_AND_STRUCTS_H_ */
