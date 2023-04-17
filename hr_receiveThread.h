/**
 * @file hr_receiveThread.h
 * @date 2022/12/05
 * @author Oliver Gerst
 *
 * @brief Part of Hypercube Routing, responsible for receiving and passing incoming messages on
 */

#ifndef SRC_HYPERCUBEROUTING_HR_RECEIVETHREAD_H_
#define SRC_HYPERCUBEROUTING_HR_RECEIVETHREAD_H_

#include "rodos.h"
#include "types_and_structs.h"

/**
 * @class HypercubeRoutingReceiveThread
 *
 * Constantly checks given UART Gateways for new messages and passes them on into a FIFO if any are received.
 */
class HypercubeRoutingReceiveThread : StaticThread<> {
public:
	/**
	 * Constructor: creates receiver thread with given UART Gateways, corresponding semaphore and FIFO to send received messages to.
	 *
	 * @param[in] uartSemaphore Semaphore to safely use the given UART Gateways.
	 * @param[in] uartGateways UART Gateways from which messages are to be received.
	 */
	HypercubeRoutingReceiveThread(Semaphore* uartSemaphore, HAL_UART(* uartGateways)[2], Fifo<MsgData, 10>* msgFifo, HAL_GPIO* statusLED);
	/**
	 * Destructor
	 */
	virtual ~HypercubeRoutingReceiveThread();

protected:
	HAL_GPIO* statusLED; /// Status LED for visual representation of thread activity, mainly used for debugging purposes

	Fifo<MsgData, 10>* msgFifo; /// FIFO, that received messages are being written to.
	Semaphore* uartSemaphore; /// Semaphore to safely use UART Gateways
	HAL_UART(* uartGateways)[2]; /// UART Gateways to send Alive Messages to

	/**
	 * Checks UART Gateway for an incoming message with a maximum length.
	 *
	 * @param[in] uart UART Gateway to check.
	 * @param rcvBuffer Pointer used for returning the received message.
	 * @param maxLen specifies the maximum length for received messages.
	 *
	 * @return Size of received message. 0 if nothing was received.
	 */
	size_t receive(HAL_UART* uart, void* rcvBuffer, const size_t maxLen = 100);

	void receiveMsgs(); /// Cycles through and receives messages from all UART Gateways.

private:
	void init();
	void run();
};

#endif /* SRC_HYPERCUBEROUTING_HR_RECEIVETHREAD_H_ */
