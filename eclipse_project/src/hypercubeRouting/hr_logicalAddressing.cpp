/*
 * hypercube_routing.cpp
 *
 *  Created on: Oct 18, 2022
 *      Author: oliver
 */

#include "../hypercubeRouting/hr_logicalAddressing.h"
#include <stdlib.h>

void HRLogicalAddressing::handleRcvMsg(HAL_UART* uart, void* msg, const binId targetAddress, void* msgBody, size_t size) {
	if (targetAddress == BROADCAST_ADDRESS) {
		broadcastId* bcId = msgBody;
		floodingMsgCounter* counter = msgBody + sizeof(broadcastId);
		binId* nodeId = msgBody + sizeof(broadcastId) + sizeof(floodingMsgCounter);

		std::map<broadcastId, std::map<binId, floodingMsgCounter>>::iterator it1 = expectedFloodingMsgCounters.find(bcId);
		if (it1 != expectedFloodingMsgCounters.end()) {
			std::map<binId, floodingMsgCounter>::iterator it2 = it1->second.find(binId);
			if (it2 != it1->second.end()) {
				if (it2->second != counter)	return; // msg does not have the expected counter value and will therefore be ignored
			}
		}

		// if msg has expected counter value or no value is present in the map, update next expected counter value
		expectedFloodingMsgCounters[bcId][nodeId] = counter + 1;s

		switch(static_cast<BroadcastIdentifier*>(msg)[0]) {
		case ALIVE:
			handleAliveMsg(uart, msg);
			break;
		case NODE_STATE:
			calculateNodeState(uart, msgBody);
			break;
		}

		forwardFloodingMsg(msg, size, uart);

	} else if (targetAddress) {
		// handle internally
	} else if (targetAddress != binaryIdentifier) {
		HAL_UART* nextHopGateway;
		binId nextHopAddress;

		std::map<binId, RoutingTableEntry>::iterator it = routingTable.find(nextHopAddress);
		if (it != routingTable.end()) { // Address already exists in routing table
			nextHopGateway = it->second.uartGateway;
			it->second.resetTTL(); // reset TTL
		} else { // Address does not exist in routing table
			binId nextHopAddress;
			size_t length;
			calculateAddressing(targetAddress, nextHopGateway, &nextHopAddress, &length);
			routingTable.insert({nextHopAddress, RoutingTableEntry(nextHopAddress, nextHopGateway)});
		}

		send(*nextHopGateway, msgBody, size);
	} else {
	}
}

void HRLogicalAddressing::calculateAddressing(const binId targetAddress, HAL_UART* nextHopUartGateway, binId* addressing, size_t* addressingLength) {
	*addressingLength = sizeof(binId);
	*addressing = targetAddress;

	binId diff = targetAddress ^ binaryIdentifier;


	// Find all dimensions where [targetAddress] and [binaryIdentifier] differ
	int hammingDistance = 0; // amount of bits that differ, therefore length of [diffDims]
	int* diffDims; // dimensions where IDs differ
	for (uint i = 0; i <= sizeof(typeof(diff)) * 8; i++) {
		if (diff & 1) {
			diffDims[hammingDistance] = 1 << i; // matches [dimOfLink] in AdjacentNode; e.g. 0b00000100 for third dimension
			hammingDistance++;
		}
		diff >>= 1;
	}

	// Find best adjacent node to route message to
	// check safe adjacent Nodes on shortest path (with dimOfLink matching to one element in diffDims)
	for (std::map<binId, AdjacentNode>::iterator it = safeAdjacentNodes.begin(); it != safeAdjacentNodes.end(); ++it) {
		for (int i = 0; i < hammingDistance; i++) {
			if (it->second.dimOfLink == diffDims[i]) {
				nextHopUartGateway = it->second.uartGateway;
				return;
			}
		}
	}
	// check unsafe adjacent nodes on shortest path (with dimOfLink matching to one element in diffDims)
	for (std::map<binId, AdjacentNode>::iterator it = unsafeAdjacentNodes.begin(); it != unsafeAdjacentNodes.end(); ++it) {
		for (int i = 0; i < hammingDistance; i++) {
			if (it->second.dimOfLink == diffDims[i]) {
				nextHopUartGateway = it->second.uartGateway;
				return;
			}
		}
	}
	// if distance to target Node <= 2 or current node strongly unsafe
	// check strongly unsafe nodes on shortest path (with dimOfLink matching to one element in diffDims)
	if (hammingDistance <= 2 || state == STRONGLY_UNSAFE) {
		for (std::map<binId, AdjacentNode>::iterator it = stronglyUnsafeAdjacentNodes.begin(); it != stronglyUnsafeAdjacentNodes.end(); ++it) {
			for (int i = 0; i < hammingDistance; i++) {
				if (it->second.dimOfLink == diffDims[i]) {
					nextHopUartGateway = it->second.uartGateway;
					return;
				}
			}
		}
	}
	// check safe nodes not on shortest path
	for (std::map<binId, AdjacentNode>::iterator it = safeAdjacentNodes.begin(); it != safeAdjacentNodes.end(); ++it) {
		for (int i = 0; i < hammingDistance; i++) {
			if (it->second.dimOfLink != diffDims[i]) {
				nextHopUartGateway = it->second.uartGateway;
				return;
			}
		}
	}
	// check unsafe nodes not on shortest path
	for (std::map<binId, AdjacentNode>::iterator it = unsafeAdjacentNodes.begin(); it != unsafeAdjacentNodes.end(); ++it) {
		for (int i = 0; i < hammingDistance; i++) {
			if (it->second.dimOfLink != diffDims[i]) {
				nextHopUartGateway = it->second.uartGateway;
				return;
			}
		}
	}
}

void HRLogicalAddressing::calculateNodeState(HAL_UART* uartGateway, void* msgBody) {
	binId nodeId = *static_cast<binId*>(msgBody + sizeof(broadcastId));
	NodeState newState = *static_cast<NodeState*>(msgBody + sizeof(broadcastId) + sizeof(binId));

	NodeState oldState;
	std::map<binId, AdjacentNode>::iterator adjacentNodeIt = adjacentNodes.find(nodeId);
	if (adjacentNodeIt != adjacentNodes.end()) {
		oldState = adjacentNodeIt->second.state;
		if (oldState != newState) {
			AdjacentNode newAdjacentNode = adjacentNodeIt->second;
			newAdjacentNode.state = newState;
			adjacentNodesPerState[newState][nodeId] = newAdjacentNode;
			adjacentNodesPerState[oldState].erase(adjacentNodeIt);
		}
	} else {
		adjacentNodesPerState[newState][nodeId] = AdjacentNode(binaryIdentifier, nodeId, uartGateway, newState);
	}

	int unsafeCount = unsafeAdjacentNodes.size() + stronglyUnsafeAdjacentNodes.size();
	int faultyCount = faultyOrNonExistingNodes.size();
	int safeCount = safeAdjacentNodes.size();

	NodeState oldOwnState = state;
	if (safeCount == 0) state = STRONGLY_UNSAFE;
	else if (faultyCount >= 2 || faultyCount + unsafeCount >= 3) state = UNSAFE;
	else state = SAFE;

	if (state != oldOwnState) sendNodeStateMsg();
}

void HRLogicalAddressing::sendNodeStateMsg() {
	nodeStateThread.resume();
}
