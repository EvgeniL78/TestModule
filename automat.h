#pragma once

#include <string>
#include "client.h"

enum class EAutoState
{
    none = -1,
    start = 0, // connection to broker
    end = 1, // disconnect from broker
    two,
    three
};

#define AUTOMAT_STATE_BEGIN 0
#define AUTOMAT_STATE_END   1
#define AUTOMAT_STATE_2     2

class AutomatStart;

class Automat
{
public:
    Automat(int s)
    : state_(s)
    {}
    virtual ~Automat() {}

    int GetState() { return state_; }
    void SetState(int s)
    {
        state_ = s;
        
        switch(state_)
        {
            case AUTOMAT_STATE_BEGIN:
             break;

            case AUTOMAT_STATE_END:
             delete this;
             return;

            case AUTOMAT_STATE_2:
            default:
             break;
        }
    }

private:
    int state_ = -1;
    Client* client_ = nullptr;
};

class AutomatStart : public Automat
{
public:
    AutomatStart();
    ~AutomatStart();
};