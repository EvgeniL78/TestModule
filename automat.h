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

class AutomatStop: public Automat
{
public:
    virtual std::string GetStateMsg() override { return "AutomatStop"; };
};

class AutomatState2: public Automat
{
public:
    virtual std::string GetStateMsg() override { return "AutomatState2"; };
};

class AutomatStateAnother: public Automat
{
public:
    virtual std::string GetStateMsg() override { return "AutomatStateAnother"; };
};
