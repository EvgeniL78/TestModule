#pragma once

#include <string>
#include <initializer_list>

class Client
{
public:
    Client(std::initializer_list<std::string> subscr);
    ~Client();

    void Publish(const std::string_view topic, const std::string_view msg);
};