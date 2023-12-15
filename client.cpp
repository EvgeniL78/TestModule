#include "client.h"
#include <unistd.h>
#include "include/MQTTAsync.h"

using namespace std;

extern void onSubscribe(void*, MQTTAsync_successData*);
extern void onSubscribeFailure(void*, MQTTAsync_failureData* response);
extern int msgArrvd(void*, char* topicName, int topicLen, MQTTAsync_message* message);

namespace 
{
    #define ADDRESS     "tcp://mqtt.eclipseprojects.io:1883"
    #define CLIENTID	"ExampleClientSub1"

    #define QOS         1
    #define TIMEOUT     10000L

    MQTTAsync client = nullptr;
    initializer_list<std::string> subsrItems;
}

int disc_finished = 0;
int subscribed = 0;
int finished = 0;

/// Connection

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
			printf("Subscribing to topic %s\nfor client %s using QoS%d\n", topic, CLIENTID, QOS);
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

/// Disconnection

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

///

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
        }
    }
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

void* Client::GetClient()
{
    return client;
}

int Client::GetQOS()
{
    return QOS;
}

