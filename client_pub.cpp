#include "client.h"
#include "include/MQTTAsync.h"

using namespace std;

void onSendFailure(void* context, MQTTAsync_failureData* response)
{
	printf("Message send failed token %d error code %d\n", response->token, response->code);
}

void onSend(void* context, MQTTAsync_successData* response)
{
	printf("Message with token value %d delivery confirmed\n", response->token);
}

void Client::Publish(const std::string_view topic, const std::string_view msg)
{
	MQTTAsync client = (MQTTAsync)GetClient();
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;

	opts.onSuccess = onSend;
	opts.onFailure = onSendFailure;
	opts.context = client;
	pubmsg.qos = GetQOS();
	pubmsg.retained = 0;
	pubmsg.payload = (void*)msg.data();
	pubmsg.payloadlen = (int)msg.length();
	int rc{};
	if ((rc = MQTTAsync_sendMessage(client, topic.data(), &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start sendMessage, return code %d\n", rc);
		throw false;
	}
	printf("Waiting for publication on topic %s\n", topic);			
}
