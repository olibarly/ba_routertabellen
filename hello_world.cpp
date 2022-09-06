#include "rodos.h"

class HelloWorldThread : public StaticThread<> {
public:
    HelloWorldThread() : StaticThread<>("HelloWorldThread") {}

    void run() override {
        PRINTF("Hello World\n");
    }

} helloWorldThread;