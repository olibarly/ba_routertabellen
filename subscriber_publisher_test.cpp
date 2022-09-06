#include "rodos.h"
#include <iostream>
#include <string>

using namespace std;

Topic<float> testTopic(-1, "test");

class PublisherThread : public StaticThread<> {
public:

    PublisherThread() : StaticThread<>("Publisher") {}

    float number;
    string input;


    void run() override {
        PRINTF("Enter number:\n");
        while (true) {
            getline(cin, input);
            number = stof(input);
            testTopic.publish(number);
        }
    }
} publisherThread;

class SubscriberThread : public SubscriberReceiver<float> {
public:
    SubscriberThread() : SubscriberReceiver<float>(testTopic, "Subscriber") {}

    void put(float &data) {
        PRINTF("%f\n", data);
    }
} subscriberThread;