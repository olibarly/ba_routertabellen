/**
 * @file hr_idRequestThread.cpp
 * @date 2023/01/18
 * @author Oliver Gerst
 *
 * @brief Part of Hypercube Routing, responsible for requesting a binary Identifier.
 * 		  Will be suspended after a certain amount of time.
 */

#ifndef SRC_HYPERCUBEROUTING_HR_IDREQUESTTHREAD_H_
#define SRC_HYPERCUBEROUTING_HR_IDREQUESTTHREAD_H_


#include "rodos.h"
#include "types_and_structs.h"

/**
 * @class HypercubeRoutingIdRequestThread
 *
 * Periodically sends ID Request Messages for a certain amount of time.
 * Will be suspended afterwards.
 * Structure of ID Request Messages can be seen in [IdRequestMsg].
 */
class HypercubeRoutingIdRequestThread : StaticThread<> {
public:
	/**
	 * Constructor: creates id request thread with given UART Gateways and corresponding semaphore.
	 *
	 * @param[in] uartSemaphore Semaphore to safely use the given UART Gateways.
	 * @param[in] uartGateways UART Gateways to send Id Requests to.
	 */
	HypercubeRoutingIdRequestThread(Semaphore* uartSemaphore, HAL_UART(* uartGateways)[2], HAL_GPIO* statusLED);
	/**
	 * Destructor
	 */
	virtual ~HypercubeRoutingIdRequestThread();

protected:
	HAL_GPIO* statusLED;/// Status LED for visual representation of thread activity, mainly used for debugging purposes

	Semaphore* uartSemaphore; /// Semaphore to safely use UART Gateways
	HAL_UART(* uartGateways)[2];/// UART Gateways to send Alive Messages to

	void sendRequest(); /// Sends an Id Request to each UART Gateway

	void init();
	void run();
};


#endif /* SRC_HYPERCUBEROUTING_HR_IDREQUESTTHREAD_H_ */
