#pragma once

#include <string>

class Automat
{
public:
    virtual std::string GetStateMsg() = 0;
};

class AutomatStart: public Automat
{
public:
    virtual std::string GetStateMsg() override { return "AutomatStart"; };
};

class AutomatEnd: public Automat
{
public:
    virtual std::string GetStateMsg() override { return "AutomatEnd"; };
};

class AutomatState2: public Automat
{
public:
    virtual std::string GetStateMsg() override { return "AutomatState2"; };
};

class AutomatStateOther: public Automat
{
public:
    virtual std::string GetStateMsg() override { return "AutomatStatOther"; };
};