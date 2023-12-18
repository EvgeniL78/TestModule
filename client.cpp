#include "client.h"
#include <unistd.h>
#include <cstdint>

#include "include/MQTTAsync.h"
#include "service.h"

using namespace std;

/// Call back functions

void connLost(void*, char* cause);
void onSubscribe(void*, MQTTAsync_successData*);
void onSubscribeFailure(void*, MQTTAsync_failureData* response);
int msgArrvd(void*, char* topicName, int topicLen, MQTTAsync_message* message);
void (*setState)(string&, string&);

namespace
{
	/// Broker address

    #define ADDRESS     "tcp://mqtt.eclipseprojects.io:1883"

	/// Client init values

    #define QOS         1
    #define TIMEOUT     10000L

    MQTTAsync client = nullptr;

	/// List of subscribtion topics

    initializer_list<std::string> subsrItems;

	/// Client flags of working states

    bool disc_finished{false};
    bool subscribed{false};
    bool finished{false};
}

/////////////////////////////
/// Connection
/////////////////////////////

/// @brief Call backed if connection failed
/// @param  
/// @param response - result
void onConnectFailure(void*, MQTTAsync_failureData* response)
{
	printLog(ELogType::base, "Connect failed, code: " + to_string(response->code));
	finished = true;
}

/// @brief Call backed if connection succeeded
/// @param context - client
/// @param  
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
		finished = true;
		return;
	}
}

/// @brief Connects to broker
/// @param client_id - client id
/// @param subscr - topics for subscription
/// @param f - call back func pointer; func will be called after client receives message from broker
Client::Client(string client_id, initializer_list<std::string> subscr, void (*f)(string&, string&))
{
    subsrItems = subscr;
	setState = f;

	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;

    try
    {
        int rc{};
        if ((rc = MQTTAsync_create(&client, ADDRESS, client_id.data(), MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTASYNC_SUCCESS)
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

/// @brief Checks if client stopped
/// @return true - client stopped
bool Client::IsFinished()
{
    return (finished == true);
}

/////////////////////////////
/// Disconnection
/////////////////////////////

/// @brief Reconnects if connection was lost
/// @param  
/// @param cause - reason
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
		finished = true;
	}
}

/// @brief Call backed if disconnection failed
/// @param  
/// @param response - reason
void onDisconnectFailure(void*, MQTTAsync_failureData* response)
{
	printLog(ELogType::plain, "Disconnect failed, code: " + to_string(response->code));
	disc_finished = true;
}

/// @brief Call backed if disconnection succeeded
/// @param  
/// @param response - reason
void onDisconnect(void*, MQTTAsync_successData* response)
{
	printLog(ELogType::plain, "Successful disconnection");
	disc_finished = true;
}

/// @brief Disconnects from broker
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

/// @brief Call backed if subscription succeeded
/// @param  
/// @param  
void onSubscribe(void*, MQTTAsync_successData*)
{
	printLog(ELogType::plain, "Subscribe succeeded");
	subscribed = true;
}

/// @brief Call backed if subscription failed
/// @param  
/// @param response - reason
void onSubscribeFailure(void*, MQTTAsync_failureData* response)
{
	printLog(ELogType::base, "Subscribe failed, code: " + to_string(response->code));
}

/// @brief Call backed when message was received from broker
/// @param  
/// @param topicName - topic name
/// @param topicLen - topic string length
/// @param message - message info
/// @return result
int msgArrvd(void*, char* topicName, int topicLen, MQTTAsync_message* message)
{
	string topic(topicName);
	string msg((char*)message->payload);
	printLog(ELogType::base, "Message arrived. Topic: " + topic + "; message: " + msg);

	if (setState)
		setState(topic, msg);

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

/////////////////////////////
/// Publishing
/////////////////////////////

/// @brief Call backed if sending failed
/// @param  
/// @param response - reason
void onSendFailure(void*, MQTTAsync_failureData* response)
{
	printLog(ELogType::base, "Message send failed token " + to_string(response->token) +" error code " + to_string(response->code));
}

/// @brief Call backed if sending successed
/// @param  
/// @param response - reason
void onSend(void*, MQTTAsync_successData* response)
{
	printLog(ELogType::plain, "Message with token value " + to_string(response->token) +" delivery confirmed");
}

/// @brief Publishes message to topic
/// @param topic 
/// @param msg - message
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
