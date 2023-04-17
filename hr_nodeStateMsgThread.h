/**
 * @file hr_nodeStateMsgThread.h
 * @date 2022/12/04
 * @author Oliver Gerst
 *
 * @brief Part of Hypercube Routing, responsible for sending node state messages.
 */

#ifndef SRC_HYPERCUBEROUTING_HR_NODESTATEMSGTHREAD_H_
#define SRC_HYPERCUBEROUTING_HR_NODESTATEMSGTHREAD_H_

#include "types_and_structs.h"
#include "rodos.h"

/**
 * @class HypercubeRoutingNodeStateThread
 *
 * Sends node state messages through the provided UART Gateways.
 * Messages are sent whenever the base thread calculates and publishes a new state and every 5 seconds.
 * Structure of Node State Message can be seen in [NodeStateMsg].
 */
class HypercubeRoutingNodeStateThread : StaticThread<> {
public:
	/**
	 * Constructor: creates node state thread with given UART Gateways, corresponding semaphore and buffer for node states
	 * @param[in] uartSemaphore Semaphore to safely use the given UART Gateways.
	 * @param[in] uartGateways UART Gateways to send Node State Messages to.
	 * @param[in] commBuffer Buffer, that new node states are being written to.
	 */
	HypercubeRoutingNodeStateThread(Semaphore* uartSemaphore, HAL_UART(* uartGateways)[2], CommBuffer<NodeState>* commBuff, HAL_GPIO* statusLED);
	/**
	 * Destructor
	 */
	virtual ~HypercubeRoutingNodeStateThread();

	/**
	 * Resumes Thread
	 */
	void resume() {
		StaticThread::resume();
	}

	/**
	 * Assigns binary identifier and resumes the thread.
	 * @param[in] binaryId Binary Identifier that will be used in Node State Messages.
	 */
	void setBinaryIdAndResume(binId binaryId);

protected:
	HAL_GPIO* statusLED; /// Status LED for visual representation of thread activity, mainly used for debugging purposes

	Semaphore* uartSemaphore; /// Semaphore to safely use UART Gateways
	HAL_UART(* uartGateways)[2]; /// UART Gateways to send Alive Messages to
	binId binaryIdentifier; /// Binary Identifier used in Node State Messages
	CommBuffer<NodeState>* nodeStateBuffer; /// Buffer, that stores most recent node state
	NodeState state = STRONGLY_UNSAFE; /// Current node state TODO: Fault Tolerance: initialize NodeState Maps (issue #21)

	void sendNodeStateMsg(); /// Sends Node State Message to each Gateway and increments the counter

private:
	void init();
	void run();
};


#endif /* SRC_HYPERCUBEROUTING_HR_NODESTATEMSGTHREAD_H_ */
