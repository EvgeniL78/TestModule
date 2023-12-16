#include "client.h"
#include <unistd.h>
#include "include/MQTTAsync.h"

using namespace std;

void connLost(void*, char* cause);
void onSubscribe(void*, MQTTAsync_successData*);
void onSubscribeFailure(void*, MQTTAsync_failureData* response);
int msgArrvd(void*, char* topicName, int topicLen, MQTTAsync_message* message);

namespace 
{
    #define ADDRESS     "tcp://mqtt.eclipseprojects.io:1883"
    #define CLIENTID	"ExampleClientSub1"

    #define QOS         1
    #define TIMEOUT     10000L

    MQTTAsync client = nullptr;
    initializer_list<std::string> subsrItems;

    int disc_finished = 0;
    int subscribed = 0;
    int finished = 0;
}

/////////////////////////////
/// Connection
/////////////////////////////

void onConnectFailure(void*, MQTTAsync_failureData* response)
{
	printf("Connect failed, rc %d\n", response->code);
	finished = 1;
}

void onConnect(void* context, MQTTAsync_successData*)
{
	printf("Successful connection\n");

	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;

	opts.onSuccess = onSubscribe;
	opts.onFailure = onSubscribeFailure;
	opts.context = client;
	auto subscr {
		[&](const string_view& topic) {
			printf("Subscribing to topic %s\nfor client %s using QoS%d\n", topic.data(), CLIENTID, QOS);
			int rc{};			   
			if ((rc = MQTTAsync_subscribe(client, topic.data(), QOS, &opts)) != MQTTASYNC_SUCCESS)
			{
				printf("Failed to start subscribe, return code %d\n", rc);
				throw false;
			}
		}
	};

	try {
        for (auto& i : subsrItems)
		    subscr(i);
	}
	catch (bool& b) {
		finished = 1;
	}
}

Client::Client(initializer_list<std::string> subscr)
{
    subsrItems = move(subscr);
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;

    try
    {
        int rc{};
        if ((rc = MQTTAsync_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL))
                != MQTTASYNC_SUCCESS)
        {
            printf("Failed to create client, return code %d\n", rc);
            throw 0;
        }

        if ((rc = MQTTAsync_setCallbacks(client, client, connLost, msgArrvd, NULL)) != MQTTASYNC_SUCCESS)
        {
            printf("Failed to set callbacks, return code %d\n", rc);
            throw 1;
        }

        conn_opts.keepAliveInterval = 20;
        conn_opts.cleansession = 1;
        conn_opts.onSuccess = onConnect;
        conn_opts.onFailure = onConnectFailure;
        conn_opts.context = client;
        if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
        {
            printf("Failed to start connect, return code %d\n", rc);
            throw 1;
        }
    }
    catch(int& err)
    {
        if (err == 1)
        {
	        MQTTAsync_destroy(&client);
            client = nullptr;
            return;
        }
    }

	while (!subscribed && !finished)
	{
		usleep(TIMEOUT);
	}    
}


bool Client::IsFinished()
{
    return (finished == 1);
}

/////////////////////////////
/// Disconnection
/////////////////////////////

void connLost(void*, char* cause)
{
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;

	printf("\nConnection lost\n");
	if (cause)
		printf("     cause: %s\n", cause);

	printf("Reconnecting\n");
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	int rc{};
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start connect, return code %d\n", rc);
		finished = 1;
	}
}

void onDisconnectFailure(void*, MQTTAsync_failureData* response)
{
	printf("Disconnect failed, rc %d\n", response->code);
	disc_finished = 1;
}

void onDisconnect(void*, MQTTAsync_successData* response)
{
	printf("Successful disconnection\n");
	disc_finished = 1;
}

Client::~Client()
{
    if (client)
    {
        MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
        disc_opts.onSuccess = onDisconnect;
        disc_opts.onFailure = onDisconnectFailure;
        int rc{};
        if ((rc = MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS)
            printf("Failed to start disconnect, return code %d\n", rc);
        else
        {
            while (!disc_finished)
                usleep(TIMEOUT);
        }

	    MQTTAsync_destroy(&client);
        client = nullptr;
    }
}

/////////////////////////////
/// Subscription
/////////////////////////////

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

/////////////////////////////
/// Publishing
/////////////////////////////

void onSendFailure(void* context, MQTTAsync_failureData* response)
{
	printf("Message send failed token %d error code %d\n", response->token, response->code);
}

void onSend(void* context, MQTTAsync_successData* response)
{
	printf("Message with token value %d delivery confirmed\n", response->token);
}

void Client::Publish(const std::string& topic, const std::string& msg)
{
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;

	opts.onSuccess = onSend;
	opts.onFailure = onSendFailure;
	opts.context = client;
	pubmsg.qos = QOS;
	pubmsg.retained = 0;
	pubmsg.payload = (void*)msg.data();
	pubmsg.payloadlen = (int)msg.length();
	int rc{};
	if ((rc = MQTTAsync_sendMessage(client, topic.data(), &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start sendMessage, return code %d\n", rc);
		throw false;
	}
	printf("Waiting for publication on topic %s\n", topic.data());			
}


