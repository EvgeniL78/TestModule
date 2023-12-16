#pragma once

#include <string>

#define AUTOMAT_STATE_BEGIN 0
#define AUTOMAT_STATE_END   1
#define AUTOMAT_STATE_2     2
#define AUTOMAT_STATE_3     3

class Automat
{
public:
    Automat(){}

    virtual bool IsWorking() { return true; }
    virtual std::string GetStateMsg() = 0;
};

class AutomatStart: public Automat
{
public:
    AutomatStart();
    virtual bool IsWorking() override;
    virtual std::string GetStateMsg() override;
};

class AutomatState2: public Automat
{
public:
    virtual std::string GetStateMsg() override { return "AutomatState2"; };
};

class AutomatState3: public Automat
{
public:
    virtual std::string GetStateMsg() override { return "AutomatState3"; };
};