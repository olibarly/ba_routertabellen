/**
 * @file hr_application.cpp
 *
 * @date 2022/12/08
 * @author Oliver Gerst
 *
 * @brief creates all necessary variables and threads for hypercube routing and groups all threads inside an application
 */


#include "rodos.h"
#include "hr_aliveMsgThread.h"
#include "hr_nodeStateMsgThread.h"
#include "hr_receiveThread.h"
#include "hr_baseThread.h"
#include "hr_idRequestThread.h"

// initialize Application
Application routingApp("RoutingApplication");

// initialize LEDs
GPIO_PIN GREEN  = GPIO_060;
GPIO_PIN ORANGE = GPIO_061;
GPIO_PIN RED    = GPIO_062;
GPIO_PIN BLUE   = GPIO_063;

HAL_GPIO greenLED = HAL_GPIO(GREEN);
HAL_GPIO orangeLED = HAL_GPIO(ORANGE);
HAL_GPIO redLED = HAL_GPIO(RED);
HAL_GPIO blueLED = HAL_GPIO(BLUE);

// initialize Variables, UART, Fifo, Buffer:
Semaphore uartSemaphore; // TODO add one semaphore per gateway

HAL_UART uartIDX1(UART_IDX1); // working: UART_IDX1, UARTIDX2, UART_IDX6
HAL_UART uartIDX6(UART_IDX6);
HAL_UART uartGateways[2] = {uartIDX1, uartIDX6};

CommBuffer<char> internalMsgBuffer;

Fifo<MsgData, 10> msgFifo;
CommBuffer<NodeState> nodeStateBuffer;



// runs before threads start, initiates UART ports
static class UARTInitiator : public RODOS::Initiator {
	void init() {
		uartIDX1.init();
		uartIDX6.init();
	}
} uartInitiator;

// runs before threads start, initiates LED pins
static class LEDInitiator : public RODOS::Initiator {
	void init() {
		greenLED.init(true, 1, 1);
		orangeLED.init(true, 1, 1);
		redLED.init(true, 1, 1);
		blueLED.init(true, 1, 1);
	}
} ledInitiator;


// initialize Threads:
// TODO: check order (issue #33)
HypercubeRoutingReceiveThread rcvThread = HypercubeRoutingReceiveThread(&uartSemaphore, &uartGateways, &msgFifo, &redLED);
HypercubeRoutingAliveMsgThread aliveMsgThread = HypercubeRoutingAliveMsgThread(&uartSemaphore, &uartGateways, &orangeLED);
HypercubeRoutingNodeStateThread nodeStateThread = HypercubeRoutingNodeStateThread(&uartSemaphore, &uartGateways, &nodeStateBuffer, &greenLED);
HypercubeRoutingBaseThread baseThread = HypercubeRoutingBaseThread(&uartSemaphore, &internalMsgBuffer, &uartGateways, &aliveMsgThread, &nodeStateThread, &nodeStateBuffer, &msgFifo, &blueLED);
HypercubeRoutingIdRequestThread idRequestThread = HypercubeRoutingIdRequestThread(&uartSemaphore, &uartGateways, &orangeLED);
