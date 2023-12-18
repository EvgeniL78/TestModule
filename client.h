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
    Client(std::string client_id, std::initializer_list<std::string> subscr, void (*f)(std::string&, std::string&) = nullptr);
    ~Client();

    bool IsFinished();
    void Publish(const char* topic, const std::string msg);
};
