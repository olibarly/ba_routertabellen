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
		switch(static_cast<BroadcastIdentifier*>(msg)[0]) {
		case ALIVE:
			handleAliveMsg(uart, msg);
			break;
		case NODE_STATE:
			calculateNodeState(uart, msgBody);
			break;
		}
	} else if (targetAddress) {

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

	/**
	 * MSG Format:
	 * Header:
	 * target address (here broadcast address) [SpaceWire header Size]
	 *
	 * Body:
	 * Broadcast Message Identifier (NODE_STATE)
	 * binary Identifier of self [binId]
	 */

	size_t size = sizeof(binId) * 2 + sizeof(broadcastId) + sizeof(nodeStateUnderlying);
	void* msg = malloc(size);

	// Header: Target Address
	binId* header = static_cast<binId*>(msg);
	*header = BROADCAST_ADDRESS;

	// Body:
	// broadcast msg identifier
	broadcastId* body1 = static_cast<broadcastId*>(msg + sizeof(typeof(*header)));
	*body1 = NODE_STATE;
	// own binary Identifier/Address
	binId* body2 = static_cast<binId*>(msg + sizeof(typeof(*header)) + sizeof(typeof(body1)));
	*body2 = binaryIdentifier;
	// node State
	binId* body3 = static_cast<binId*>(msg + sizeof(typeof(*header)) + sizeof(typeof(body1)) + sizeof(typeof(body2)));
	*body3 = state;


	for (HAL_UART uart : uartGateways) send(uart, msg, size);

	free(msg);

}
