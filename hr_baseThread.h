/**
 * @file hr_base.h
 * @date 2022/11/16
 * @author Oliver Gerst
 *
 * @brief Main part of Hypercube Routing, responsible for doing all calculations and managing auxiliary threads.
 */

#ifndef SRC_HYPERCUBEROUTING_HR_BASETHREAD_H_
#define SRC_HYPERCUBEROUTING_HR_BASETHREAD_H_

#include "rodos.h"
#include <map>
#include <list>

#include "hr_receiveThread.h"
#include "hr_aliveMsgThread.h"
#include "hr_nodeStateMsgThread.h"
#include "types_and_structs.h"


/**
 * @class HypercubeRoutingBaseThread
 *
 * Sends new Messages and processes received messages.
 * Keeps track of other nodes through the routing table and stores their state.
 */
class HypercubeRoutingBaseThread : StaticThread<> {
public:
	/**
	 * Constructor: creates the Base Thread for Hypercube Routing
	 *
	 * @param uartSemaphore Semaphore to safely use the given UART Gateways
	 * @param internalMsgBuffer Buffer, that messages that are sent to this node are written to, to be handled internally
	 * @param uartGateways All UART Gateways of the device that shall be handled by this thread
	 * @param aliveMsgThread Pointer to the Alive Msg Thread
	 * @param nodeStateThread Pointer to the Node State Thread
	 * @param nodeStateBuffer Buffer, that the current state of this node will be written to
	 * @param msgFifo FIFO, that the Receiver Thread writes the received messages to
	 */
	HypercubeRoutingBaseThread(Semaphore* uartSemaphore, CommBuffer<char>* internalMsgBuffer, HAL_UART(* uartGateways)[2], HypercubeRoutingAliveMsgThread* aliveMsgThread, HypercubeRoutingNodeStateThread* nodeStateThread, CommBuffer<NodeState>* nodeStateBuffer, Fifo<MsgData, 10>* msgFifo, HAL_GPIO* statusLED);
	/**
	 * Destructor
	 */
	virtual ~HypercubeRoutingBaseThread();
	
	/**
	 * Sends a message to the specified address.
	 *
	 * @param targetAddress The address that the message is to be sent to.
	 * @param msgBody The body (only data part of msg / msg without header) of the msg that is to be sent.
	 * @param msgBodySize The size of the message body.
	 */
	void sendToAddress(binId targetAddress, const void* msgBody, size_t msgBodySize);

protected:
	HAL_GPIO* statusLED; /// Status LED for visual representation of thread activity, mainly used for debugging purposes

	CommBuffer<char>* internalMsgBuffer; /// Buffer, that messages that are sent to this node are written to, to be handled internally

	Semaphore* uartSemaphore; ///Semaphore to safely use the given UART Gateways
	HAL_UART(* uartGateways)[2]; /// All UART Gateways of the device that shall be handled by this thread

	bool idInitialized; /// Specifies if an binary Identifier has been determined for this device
	binId binaryIdentifier; /// The binary Identifier of this device

	HypercubeRoutingAliveMsgThread* aliveMsgThread; /// Pointer to the Alive Message Thread

	HypercubeRoutingNodeStateThread* nodeStateThread; /// Pointer to the Node State Thread
	NodeState state; /// The current state of this node
	CommBuffer<NodeState>* nodeStateBuffer; /// Buffer, that stores the current state of this node

	Fifo<MsgData, 10>* msgFifo; /// FIFO, that stores the messages received by the Receiver Thread

	std::map<MessageIdentifier, std::map<binId, floodingMsgCounter>> expectedFloodingMsgCounters; ///

	std::map<binId, RoutingTableEntry> routingTable; /// Table, that stores a RoutingTableEntry for each known node for a certain amount of time
	std::map<binId, AdjacentNode> adjacentNodes; /// Table, that stores an AdjacentNode for each known node, that is directly adjacent to this node
	std::map<NodeState, std::map<binId, AdjacentNode>> adjacentNodesPerState; /// Table, that stores adjacent nodes sorted by their state

	std::map<binId, IdRequestResponseMsg> idRequestResponsesPerNode; /// Table, that stores the responses to the Id Requests of this node


	/**
	 * Adds a new entry or updates an existing entry in the Routing Table
	 *
	 * @param binaryId Binary Identifier for which the Entry is to be updated
	 * @param routingTableEntry The new Entry
	 */
	void updateTable(binId binaryId, RoutingTableEntry routingTableEntry);
	/**
	 * Removes all Entries from the Routing Table, whose Time of Deletion has been reached
	 */
	void removeOutdatedTableEntries();
	/**
	 * Determines the UART Gateway through which a message with a certain target address has to be sent.
	 * Also creates the header, consisting of the addressing, and its length for the message.
	 *
	 * @param targetAddress The target address of the message
	 * @param nextHopUartGateway Pointer, through which the according UART Gateway is returned
	 * @param addressing Pointer, through which the header(addressing) is returned
	 * @param Pointer, through which the length of the header is returned
	 */
	void calculateNextHop(const binId targetAddress, HAL_UART* nextHopUartGateway);

	/**
	 * Sends a message through the specified gateway.
	 *
	 * @param MsgData Struct containing all relevant data for sending a specific message through a specific gateway
	 */
	void send(MsgData* msgData);
	/**
	 * Sends a message through the specified gateway.
	 *
	 * @param uart The gateway through which the message is to be sent.
	 * @param msg The message to be sent.
	 * @param size The size of the message.
	 */
	void send(HAL_UART& uart, const void* msg, size_t size);

	/**
	 * Forwards a flooding message to all adjacent nodes except the one the message was received from.
	 *
	 * @param msg The message to be forwarded.
	 * @param msgSize The size of the message, that is to be forwarded.
	 * @param sourceId The binary Identifier, that the message has been received from.
	 */
	void forwardFloodingMsg(void* msg, size_t msgSize, binId sourceId);

	/**
	 * Creates a message from a header and a body.
	 *
	 * @param addressing The header (addressing) of the message.
	 * @param addressingLength The length of the header.
	 * @param msgBody The body of the message.
	 * @param msgBodySize The size of the message body.
	 */
	Msg createMsg(binId targetId, const void* msgBody, size_t msgBodySize);

	/**
	 * Extracts [targetAddress] and [msgBody] from [msg].
	 * Returns true if [msg] valid, false otherwise.
	 */
	bool decodeRcvMsg(void* msg, binId& targetAddress, void*& msgBody, size_t msgSize);

	/**
	 * Handles and processes a message.
	 *
	 * @param uart The gateway the message has been received from.
	 * @param msg The received message.
	 * @param targetAddress The target address of the received message.
	 * @param msgBody The body of the message.
	 * @param size The size of the complete received message.
	 */
	void handleRcvMsg(HAL_UART* uart, void* msg, const binId targetAddress, void* msgBody, size_t size);

	/**
	 * Handles Alive Messages.
	 * Updates the Routing Table with the information included in the message.
	 * Forwards the message to adjacent nodes except through the receiving gateway.
	 *
	 * @param uart The gateway from whih the message has been received.
	 * @param msg The received message.
	 */
	void handleAliveMsg(HAL_UART* uart, void* msg);

	/**
	 * Handles Id Requests.
	 * Sends response containing all possible IDs (from the point of view of this node).
	 *
	 * @param uart The gateway through which the message was received.
	 */
	void handleIdRequestMsg(HAL_UART* uart);

	/**
	 * Handles Id Request Responses.
	 * Adds the received Response to [idRequestResponsesPerNode].
	 *
	 * @param uart The gateway through which the message has been received.
	 * @param msg The received message.
	 */
	void handleIdRequestResponseMsg(HAL_UART* uart, void* msg);

	/**
	 * Handles Node State Messages.
	 * Puts AdjacentNode Entry in according table ([safeAdjacentNodes], [unsafeAdjacentNodes], [stronglyUnsafeAdjacentNodes], [faultyAdjacentNodes]).
	 *
	 * @param uartGateway The gateway through which the message has been received.
	 * @param msg The received message.
	 *
	 * Takes the node state broadcast message [msgBody] and the gateway [uartGateway] via which it was received.
	 * Moves AdjacenNode Entry to correct map ([safeAdjacentNodes], [unsafeAdjacentNodes], [stronglyUnsafeAdjacentNodes], [faultyAdjacentNodes]).
	 */
	void handleNodeStateMsg(HAL_UART* uartGateway, void* msg);

	/**
	 * Calculates the state of this node according to the states of all adjacent nodes.
	 * Sends node state message to adjacent nodes if this nodes state has changed.
	 */
	void calculateNodeState();

	/**
	 * Sends the current state of this node to all adjacent nodes through a Node State Message
	 */
	void sendNodeStateMsg();

	/**
	 * Searches for a fitting binary identifier for this node in [idRequestResponsesPerNode].
	 * A fitting binary identifier is one, that is included in all received responses to the Id Requests.
	 * If no responses have been received and [idRequestResponsesPerNode] is empty, 0 is assigned as this nodes binary identifier.
	 */
	void evaluatePossibleIds();

	void init();
	void run();
};


#endif /* SRC_HYPERCUBEROUTING_HR_BASETHREAD_H_ */
