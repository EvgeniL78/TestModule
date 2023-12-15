#include "client.h"
#include "include/MQTTAsync.h"

using namespace std;

void onSubscribe(void*, MQTTAsync_successData*)
{
	printf("Subscribe succeeded\n");
	subscribed = 1;
}

void onSubscribeFailure(void*, MQTTAsync_failureData* response)
{
	printf("Subscribe failed, rc %d\n", response->code);
}

int msgArrvd(void*, char* topicName, int topicLen, MQTTAsync_message* message)
{
    printf("Message arrived\n");
    printf("   topic: %s\n", topicName);
    printf("   message: %.*s\n", message->payloadlen, (char*)message->payload);
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

