#pragma once

#include <string>
#include <initializer_list>

#define TOPIC_STATE      	"State"
#define TOPIC_GO_TO_STATE	"GoToState"
#define TOPIC_TIME_ELAPSED	"TimeElapsed"
#define TOPIC_TIME			"Time"

class Client
{
public:
    Client(std::initializer_list<std::string> subscr);
    ~Client();

    bool IsFinished();
    void Publish(const std::string_view topic, const std::string_view msg);
};