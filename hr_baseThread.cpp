/*
 * hr_base.cpp
 *
 *  Created on: Nov 16, 2022
 *      Author: oliver
 */



#include "hr_baseThread.h"

#include "rodos.h"

#include <map>
#include <list>
#include <vector>
#include <stdlib.h>



HypercubeRoutingBaseThread::HypercubeRoutingBaseThread(Semaphore* uartSemaphore, CommBuffer<char>* internalMsgBuffer, HAL_UART(* uartGateways)[2], HypercubeRoutingAliveMsgThread* aliveMsgThread, HypercubeRoutingNodeStateThread* nodeStateThread, CommBuffer<NodeState>* nodeStateBuffer, Fifo<MsgData, 10>* msgFifo, HAL_GPIO* statusLED) : StaticThread("BaseThread") {
	this->statusLED = statusLED;

	this->internalMsgBuffer = internalMsgBuffer;

	idInitialized = false;

	this->uartSemaphore = uartSemaphore;
	this->uartGateways = uartGateways;
	this->aliveMsgThread = aliveMsgThread;
	this->nodeStateThread = nodeStateThread;
	this->nodeStateBuffer = nodeStateBuffer;
	this->msgFifo = msgFifo;
}

HypercubeRoutingBaseThread::~HypercubeRoutingBaseThread() {}


void HypercubeRoutingBaseThread::updateTable(binId binaryId, RoutingTableEntry routingTableEntry) {
	routingTable[binaryId] = routingTableEntry;
}

void HypercubeRoutingBaseThread::removeOutdatedTableEntries() {

	std::list<std::map<binId, RoutingTableEntry>::iterator> elementsToRemove;

	//gathers all elements that need to be removed
	for (auto it = routingTable.begin(); it != routingTable.cend(); ++it) {
		if (it->second.timeOfDeletion >= NOW()) elementsToRemove.push_back(it);
	}

	//removes the found elements
	for (auto e : elementsToRemove) routingTable.erase(e);
}

void HypercubeRoutingBaseThread::calculateNextHop(const binId targetAddress, HAL_UART* nextHopUartGateway) {
	binId diff = targetAddress ^ binaryIdentifier;


	// Find all dimensions where [targetAddress] and [binaryIdentifier] differ:
	int hammingDistance = 0; // amount of bits that differ, therefore length of [diffDims]
	int diffDims[BIT_SIZE_OF_ID]; // dimensions where IDs differ
	for (uint i = 0; i <= sizeof(diff) * 8; i++) {
		if (diff & 1) {
			diffDims[hammingDistance] = 1 << i; // matches [dimOfLink] in AdjacentNode; e.g. 0b00000100 for third dimension
			hammingDistance++;
		}
		diff >>= 1;
	}


	// Find best adjacent node to route message to:
	// check safe adjacent Nodes on shortest path (with dimOfLink matching one element in diffDims):
	for (std::map<binId, AdjacentNode>::iterator it = adjacentNodesPerState[SAFE].begin(); it != adjacentNodesPerState[SAFE].end(); ++it) {
		for (int i = 0; i < hammingDistance; i++) {
			if (it->second.dimOfLink == diffDims[i]) {
				*nextHopUartGateway = *it->second.uartGateway;
				return;
			}
		}
	}
	// check unsafe adjacent nodes on shortest path (with dimOfLink matching one element in diffDims):
	for (std::map<binId, AdjacentNode>::iterator it = adjacentNodesPerState[UNSAFE].begin(); it != adjacentNodesPerState[UNSAFE].end(); ++it) {
		for (int i = 0; i < hammingDistance; i++) {
			if (it->second.dimOfLink == diffDims[i]) {
				*nextHopUartGateway = *it->second.uartGateway;
				return;
			}
		}
	}
	// if distance to target Node <= 2 or current node strongly unsafe
	// check strongly unsafe nodes on shortest path (with dimOfLink matching one element in diffDims)
	if (hammingDistance <= 2 || state == STRONGLY_UNSAFE) {
		for (std::map<binId, AdjacentNode>::iterator it = adjacentNodesPerState[STRONGLY_UNSAFE].begin(); it != adjacentNodesPerState[STRONGLY_UNSAFE].end(); ++it) {
			for (int i = 0; i < hammingDistance; i++) {
				if (it->second.dimOfLink == diffDims[i]) {
					*nextHopUartGateway = *it->second.uartGateway;
					return;
				}
			}
		}
	}
	// check safe nodes not on shortest path
	for (std::map<binId, AdjacentNode>::iterator it = adjacentNodesPerState[SAFE].begin(); it != adjacentNodesPerState[SAFE].end(); ++it) {
		for (int i = 0; i < hammingDistance; i++) {
			if (it->second.dimOfLink != diffDims[i]) {
				*nextHopUartGateway = *it->second.uartGateway;
				return;
			}
		}
	}
	// check unsafe nodes not on shortest path
	for (std::map<binId, AdjacentNode>::iterator it = adjacentNodesPerState[UNSAFE].begin(); it != adjacentNodesPerState[UNSAFE].end(); ++it) {
		for (int i = 0; i < hammingDistance; i++) {
			if (it->second.dimOfLink != diffDims[i]) {
				*nextHopUartGateway = *it->second.uartGateway;
				return;
			}
		}
	}
}

void HypercubeRoutingBaseThread::send(MsgData* msgData) {
	PROTECT_IN_SCOPE(*uartSemaphore);

	statusLED->setPins(1);
	msgData->uart->write(msgData->msg, msgData->size);
	statusLED->setPins(0);
}

void HypercubeRoutingBaseThread::send(HAL_UART& uart, const void* msg, size_t size) {
	PROTECT_IN_SCOPE(*uartSemaphore);

	statusLED->setPins(1);
	uart.write(msg, size);
	statusLED->setPins(0);
}

void HypercubeRoutingBaseThread::sendToAddress(binId targetAddress, const void* msgBody, size_t msgBodySize) {
	// check if address is in routing table
	std::map<binId, RoutingTableEntry>::iterator routingTableMapEntry = routingTable.find(targetAddress);
	RoutingTableEntry routingData;

	// if not: calculate addressing, updateTable
	if (routingTableMapEntry == routingTable.end()) {
		HAL_UART* nextHopUartGateway = NULL;
		calculateNextHop(targetAddress, nextHopUartGateway);

		routingData = RoutingTableEntry(targetAddress, nextHopUartGateway);
		updateTable(targetAddress, routingData);
	} else { //otherwise: just use data from table
		routingData = routingTableMapEntry->second;
	}

	Msg msg;
	msg = createMsg(targetAddress, msgBody, msgBodySize);

	MsgData msgData(routingData.uartGateway, &msg, sizeof(msg));
	send(&msgData);
}

void HypercubeRoutingBaseThread::forwardFloodingMsg(void* msg, size_t msgSize, binId sourceId) {
	for (std::map<binId, AdjacentNode>::iterator it = adjacentNodes.begin(); it != adjacentNodes.end(); ++it) {
		if (sourceId != it->first) {
			send(*(it->second.uartGateway), msg, msgSize);
		}
	}
}

Msg HypercubeRoutingBaseThread::createMsg(binId targetId, const void* msgBody, size_t msgBodySize) {
	LogicalAddressingMsg msg;
	msg.targetId = targetId;

	if (msgBodySize <= MSG_BODY_MAX_LENGTH) {
		for (uint i = 0; i < msgBodySize; i++) {
			msg.msgBody[i] = static_cast<const uint8_t*>(msgBody)[i];
		}
	}

	return msg;
}

bool HypercubeRoutingBaseThread::decodeRcvMsg(void* msg, binId& targetAddress, void*& msgBody, size_t msgSize) {
	// TODO: check if Msg valid

	// check if msg has body
	if (msgSize <= sizeof(binId)) return false;

	// extract header and body
	targetAddress = *static_cast<binId*>(msg);
	msgBody = static_cast<binId*>(msg) + 1;

	return true;
}

void HypercubeRoutingBaseThread::handleRcvMsg(HAL_UART* uart, void* msg, const binId targetAddress, void* msgBody, size_t size) {
	if (targetAddress == BROADCAST_ADDRESS) {
		MessageIdentifier* bcId = static_cast<MessageIdentifier*>(msgBody);
		floodingMsgCounter* counter = static_cast<floodingMsgCounter*>(msgBody) + sizeof(MessageIdentifier);
		binId* nodeId = static_cast<binId*>(msgBody) + sizeof(MessageIdentifier) + sizeof(floodingMsgCounter);

		std::map<MessageIdentifier, std::map<binId, floodingMsgCounter>>::iterator it1 = expectedFloodingMsgCounters.find(*bcId);
		if (it1 != expectedFloodingMsgCounters.end()) {
			std::map<binId, floodingMsgCounter>::iterator it2 = it1->second.find(*nodeId);
			if (it2 != it1->second.end()) {
				if (it2->second != *counter)	return; // msg does not have the expected counter value and will therefore be ignored
			}
		}

		// if msg has expected counter value or no value is present in the map, update next expected counter value
		expectedFloodingMsgCounters[*bcId][*nodeId] = *counter + 1;

		switch(static_cast<MessageIdentifier*>(msg)[0]) {
		case ALIVE:
			handleAliveMsg(uart, msg);
			forwardFloodingMsg(msg, size, *nodeId);
			break;
		case NODE_STATE:
			handleNodeStateMsg(uart, msg);
			break;
		case ID_REQUEST:
			handleIdRequestMsg(uart);
			break;
		case ID_REQUEST_RESPONSE:
			if (!idInitialized)
			handleIdRequestResponseMsg(uart, msg);
			break;
		}

	} else if (targetAddress) {
		internalMsgBuffer->put(*static_cast<char*>(msg)); // handle internally
	} else if (targetAddress != binaryIdentifier) {
		HAL_UART* nextHopGateway = NULL;
		binId nextHopAddress;

		std::map<binId, RoutingTableEntry>::iterator it = routingTable.find(nextHopAddress);
		if (it != routingTable.end()) { // Address already exists in routing table
			nextHopGateway = it->second.uartGateway;
			it->second.resetToD(); // reset Time of Deletion
		} else { // Address does not exist in routing table
			calculateNextHop(targetAddress, nextHopGateway);
			updateTable(targetAddress, RoutingTableEntry(targetAddress, nextHopGateway));
		}

		send(*nextHopGateway, msgBody, size);
	} else {
	}
}

void HypercubeRoutingBaseThread::handleAliveMsg(HAL_UART* uart, void* msg) {
	AliveMsg aliveMsg = *static_cast<AliveMsg*>(msg);

	HAL_UART* nextHopUartGateway = NULL;

	calculateNextHop(aliveMsg.nodeId, nextHopUartGateway);
	updateTable(aliveMsg.nodeId, RoutingTableEntry(aliveMsg.nodeId, nextHopUartGateway));
}

void HypercubeRoutingBaseThread::handleIdRequestMsg(HAL_UART* uart) {
	binId possibleIds[BIT_SIZE_OF_ID];
	uint numberOfPossibleIds = 0;

	// TODO: check if generated Ids contain broadcast Id (issue #34)

	for (int i = 0; i < BIT_SIZE_OF_ID; i++) { // go through id bit-wise
		binId newId = binaryIdentifier ^ 1 << i;
		if (newId != BROADCAST_ADDRESS) {
			possibleIds[numberOfPossibleIds] = newId; //pretend there is no neighbor in that dimension
			for (auto it = adjacentNodes.begin(); it != adjacentNodes.end(); ++it) {
				if (it->first == possibleIds[numberOfPossibleIds]){
					numberOfPossibleIds--; //if neighbor in that dimension is found, remove the id from the list
					break; // continue with next dimension
				}
			}
			numberOfPossibleIds++; //keep the id, by incrementing the index, if no neighbor with it is found; otherwise this just restores the index / negates the decrementation from above
		}
	}

	IdRequestResponseMsg msg(binaryIdentifier, numberOfPossibleIds, possibleIds);

	send(*uart, &msg, sizeof(msg));
}

void HypercubeRoutingBaseThread::handleIdRequestResponseMsg(HAL_UART* uart, void* msg) {
	if (!idInitialized) { //ignore if id already initialized
		IdRequestResponseMsg response = *static_cast<IdRequestResponseMsg*>(msg);
		idRequestResponsesPerNode[response.sourceId] = response;
	}
}

void HypercubeRoutingBaseThread::handleNodeStateMsg(HAL_UART* uartGateway, void* msg) {
	NodeStateMsg nodeStateMsg = *static_cast<NodeStateMsg*>(msg);

	binId nodeId = nodeStateMsg.nodeId;
	NodeState newState = nodeStateMsg.state;

	NodeState oldState;
	std::map<binId, AdjacentNode>::iterator adjacentNodeIt = adjacentNodes.find(nodeId);
	if (adjacentNodeIt != adjacentNodes.end()) {
		oldState = adjacentNodeIt->second.state;

		if (oldState != newState) {
			adjacentNodeIt->second.state = newState;

			AdjacentNode newAdjacentNode = adjacentNodeIt->second;
			newAdjacentNode.state = newState;
			adjacentNodesPerState[newState][nodeId] = newAdjacentNode;
			adjacentNodesPerState[oldState].erase(adjacentNodeIt);
		} else return; // when nothing has changed, nothing needs to be done
	} else {
		adjacentNodesPerState[newState][nodeId] = AdjacentNode(binaryIdentifier, nodeId, uartGateway, newState);
	}

	calculateNodeState();
}

void HypercubeRoutingBaseThread::calculateNodeState() {
	int unsafeCount = adjacentNodesPerState[UNSAFE].size() + adjacentNodesPerState[STRONGLY_UNSAFE].size();
	int faultyCount = BIT_SIZE_OF_ID - adjacentNodes.size();
	int safeCount = adjacentNodesPerState[SAFE].size();

	NodeState oldOwnState = state;
	if (safeCount == 0) state = STRONGLY_UNSAFE;
	else if (faultyCount >= 2 || faultyCount + unsafeCount >= 3) state = UNSAFE;
	else state = SAFE;

	if (state != oldOwnState) {
		nodeStateBuffer->put(state);
		sendNodeStateMsg();
	}
}

void HypercubeRoutingBaseThread::sendNodeStateMsg() {
	nodeStateThread->resume();
}

void HypercubeRoutingBaseThread::evaluatePossibleIds() {
	PRINTF("MainThread: evaluating IDs\n");

	std::vector<binId> acceptedIds;


	if (idRequestResponsesPerNode.empty()) {
		acceptedIds.push_back(0);
	}	else {
		IdRequestResponseMsg msg = idRequestResponsesPerNode.begin()->second;

		// this searches for all ids that are present in every response. alternatively the loop could be stopped after finding one, since thats enough.
		for (int i = 0; i < msg.possibleIdCount; i++) { // loop through possible ids of first response
			binId idToCheck = msg.possibleIds[i];
			bool idFound = true;

			for (auto it = ++(idRequestResponsesPerNode.begin()); it != idRequestResponsesPerNode.end() && idFound; it++) { //loop through all other responses, break loop if id not found in possible ids of node
				idFound = false;
				for (int j = 0; j < it->second.possibleIdCount; it++) { //loop through possible Ids of response
					if (idToCheck == it->second.possibleIds[j]) {
						idFound = true;
						break; // break loop if id found
					}
				}
			}

			if(idFound) {
				acceptedIds.push_back(idToCheck);
			}
		}
	}

	binaryIdentifier = acceptedIds[0];
	idInitialized = true;

	PRINTF("MainThread: ID determined as %u\n", binaryIdentifier);
	PRINTF("MainThread: setting IDs and resuming threads\n");

	aliveMsgThread->setBinaryIdAndResume(binaryIdentifier);

	nodeStateThread->setBinaryIdAndResume(binaryIdentifier);
}

void HypercubeRoutingBaseThread::init() {}

void HypercubeRoutingBaseThread::run() {
	PRINTF("MainThread: run start\n");

	int64_t idRequestTimeout = NOW() + 30*SECONDS;

	int64_t nextCleanUp = NOW();

	while(1) {
		size_t elementCount = msgFifo->getElementCount();
		for (uint i = 0; i < elementCount; i++) {
			MsgData msgData;
			msgFifo->get(msgData);

			binId targetAddress;
			void* msgBody;
			if (decodeRcvMsg(msgData.msg, targetAddress, msgBody, msgData.size)) {
				handleRcvMsg(msgData.uart, msgData.msg, targetAddress, msgBody, msgData.size);
			}
		}

		if (!idInitialized) {
			if (NOW() > idRequestTimeout) {
				PRINTF("MainThread: evaluate IDs\n");//TODO: interrupt without handler error if PRINT is missing
				evaluatePossibleIds();
			}
		}

		if (NOW() >= nextCleanUp) {
			int64_t  cleanupInterval = 1*SECONDS;
			removeOutdatedTableEntries(); //cleanup
			nextCleanUp += cleanupInterval; // next cleanup in 1sec
		}
	}
}
