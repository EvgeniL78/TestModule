#pragma once

#include <string>
#include <initializer_list>

#define TOPIC_STATE      	"State"
#define TOPIC_GO_TO_STATE	"GoToState"
#define TOPIC_TIME_ELAPSED	"TimeElapsed"
#define TOPIC_TIME			"Time"
#define TOPIC_GET_TIME      "GetTime"

class Client
{
public:
    Client(std::initializer_list<std::string> subscr, void (*f)(const char*, std::string));
    ~Client();

    bool IsFinished();
    void Publish(const char* topic, const std::string msg);
};
