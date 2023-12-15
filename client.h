#pragma once

#include <string>
#include <initializer_list>

class Client
{
public:
    Client(std::initializer_list<std::string> subscr);
    ~Client();

    void Subscribe(const std::string_view topic);
    void Publish(const std::string_view topic, const std::string_view msg);

private:
    void* GetClient();
    int GetQOS();
};

// extern void onSubscribe(void*, MQTTAsync_successData*);
// extern void onSubscribeFailure(void*, MQTTAsync_failureData* response);

extern int disc_finished;
extern int subscribed;
extern int finished;