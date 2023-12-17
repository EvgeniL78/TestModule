#include "client.h"
#include <unistd.h>
#include <cstdint>

#include "include/MQTTAsync.h"
#include "service.h"

using namespace std;

// Call back functions

void connLost(void*, char* cause);
void onSubscribe(void*, MQTTAsync_successData*);
void onSubscribeFailure(void*, MQTTAsync_failureData* response);
int msgArrvd(void*, char* topicName, int topicLen, MQTTAsync_message* message);
void (*setState)(const char*, int);

namespace 
{
    #define ADDRESS     "tcp://mqtt.eclipseprojects.io:1883"
    #define CLIENTID	"ExampleClient_main"

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
	printLog(ELogType::base, "Connect failed, code: " + to_string(response->code));
	finished = 1;
}

void onConnect(void* context, MQTTAsync_successData*)
{
	printLog(ELogType::base, "Successful connection");

	// Subscribing init

	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;

	opts.onSuccess = onSubscribe;
	opts.onFailure = onSubscribeFailure;
	opts.context = client;
	auto subscr {
		[&](const string_view& topic) {
			printLog(ELogType::plain, "Subscribing to topic " + string(topic));
			int rc{};			   
			if ((rc = MQTTAsync_subscribe(client, topic.data(), QOS, &opts)) != MQTTASYNC_SUCCESS)
			{
				printLog(ELogType::base, "Failed to start subscribe, code: " + to_string(rc));
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
		return;
	}
}

// Connections to broker
Client::Client(initializer_list<std::string> subscr, void (*f)(const char*, int))
{
    subsrItems = subscr;
	setState = f;

	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;

    try
    {
        int rc{};
        if ((rc = MQTTAsync_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTASYNC_SUCCESS)
        {
			printLog(ELogType::base, "Failed to create client, code: " + to_string(rc));
            throw 0;
        }

        if ((rc = MQTTAsync_setCallbacks(client, client, connLost, msgArrvd, NULL)) != MQTTASYNC_SUCCESS)
        {
			printLog(ELogType::base, "Failed to set callback, code: " + to_string(rc));
            throw 1;
        }

        conn_opts.keepAliveInterval = 20;
        conn_opts.cleansession = 1;
        conn_opts.onSuccess = onConnect;
        conn_opts.onFailure = onConnectFailure;
        conn_opts.context = client;
        if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
        {
			printLog(ELogType::base, "Failed to start connect, code: " + to_string(rc));
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

// Reconnects if connection was lost
void connLost(void*, char* cause)
{
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;

	printLog(ELogType::plain, "Connection lost");
	if (cause)
		printLog(ELogType::plain, "	cause: " + string(cause));

	printLog(ELogType::plain, "Reconnecting");
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	int rc{};
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
	{
		printLog(ELogType::base, "Failed to start connect, code: " + to_string(rc));
		finished = 1;
	}
}

void onDisconnectFailure(void*, MQTTAsync_failureData* response)
{
	printLog(ELogType::plain, "Disconnect failed, code: " + to_string(response->code));
	disc_finished = 1;
}

void onDisconnect(void*, MQTTAsync_successData* response)
{
	printLog(ELogType::plain, "Successful disconnection");
	disc_finished = 1;
}

// Disconnections
Client::~Client()
{
    if (client)
    {
        MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
        disc_opts.onSuccess = onDisconnect;
        disc_opts.onFailure = onDisconnectFailure;
        int rc{};
        if ((rc = MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS)
			printLog(ELogType::base, "Failed to start disconnect, code: " + to_string(rc));
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
	printLog(ELogType::plain, "Subscribe succeeded");
	subscribed = 1;
}

void onSubscribeFailure(void*, MQTTAsync_failureData* response)
{
	printLog(ELogType::base, "Subscribe failed, code: " + to_string(response->code));
}

int msgArrvd(void*, char* topicName, int topicLen, MQTTAsync_message* message)
{
	printLog(ELogType::base, "Message arrived. Topic: " + string(topicName) + "; message: " + string((char*)message->payload));

	int res{};
	sscanf((char*)message->payload, "%d", &res);
	if (setState)
		setState(topicName, res);

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

/////////////////////////////
/// Publishing
/////////////////////////////

void onSendFailure(void* context, MQTTAsync_failureData* response)
{
	printLog(ELogType::base, "Message send failed token " + to_string(response->token) +" error code " + to_string(response->code));
}

void onSend(void* context, MQTTAsync_successData* response)
{
	printLog(ELogType::plain, "Message with token value " + to_string(response->token) +" delivery confirmed");
}

void Client::Publish(const char* topic, const std::string msg)
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
	if ((rc = MQTTAsync_sendMessage(client, topic, &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
	{
		printLog(ELogType::base, "Failed to start publishing, code: " + to_string(rc));
		throw false;
	}
	printLog(ELogType::plain, "Waiting for publication on topic " + string(topic) + ". Message: " + msg);
}


