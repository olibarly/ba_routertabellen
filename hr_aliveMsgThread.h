/**
 * @file hr_AliveMsgThread.h
 * @date 2022/12/04
 * @author Oliver Gerst
 *
 * @brief Part of Hypercube Routing, responsible for sending alive messages.
 */

#ifndef SRC_HYPERCUBEROUTING_HR_ALIVEMSGTHREAD_H_
#define SRC_HYPERCUBEROUTING_HR_ALIVEMSGTHREAD_H_

#include "rodos.h"
#include "types_and_structs.h"

/**
 * @class HypercubeRoutingAliveMsgThread
 *
 * Periodically sends Alive Messages through the provided UART Gateways.
 * Is initially suspended, will be resumed after binaryIdentifier is given.
 * Structure of Alive Messages can be seen in [AliveMsg].
 */
class HypercubeRoutingAliveMsgThread : StaticThread<> {
public:
	/**
	 * Constructor: creates alive message thread with given UART Gateways and corresponding semaphore.
	 *
	 * @param[in] uartSemaphore Semaphore to safely use the given UART Gateways.
	 * @param[in] uartGateways UART Gateways to send Alive Messages to.
	 */
	HypercubeRoutingAliveMsgThread(Semaphore* uartSemaphore, HAL_UART(* uartGateways)[2], HAL_GPIO* statusLED);
	/**
	 * Destructor
	 */
	virtual ~HypercubeRoutingAliveMsgThread();

	/**
	 * Resumes Thread.
	 */
	void resume() {
		StaticThread::resume();
	}

	/**
	 * Assigns binary identifier and resumes the thread.
	 * @param[in] binaryId Binary Identifier that will be used in Alive Messages.
	 */
	void setBinaryIdAndResume(binId binaryId);

protected:
	HAL_GPIO* statusLED; /// Status LED for visual representation of thread activity, mainly used for debugging purposes

	Semaphore* uartSemaphore; /// Semaphore to safely use UART Gateways
	HAL_UART(* uartGateways)[2]; /// UART Gateways to send Alive Messages to
	binId binaryIdentifier; /// Binary Identifier used in Alive Messages
	floodingMsgCounter aliveMsgCounter = 0; /// Counter that identifies each Alive Msg

	void sendAliveMsg(); /// Sends Alive Message to each Gateway and increments the counter

private:
	void init();
	void run();
};


#endif /* SRC_HYPERCUBEROUTING_HR_ALIVEMSGTHREAD_H_ */
