#include <memory>
#include <initializer_list>

#include "automat.h"
#include "client.h"

using namespace std;

unique_ptr<Client> client;

AutomatStart::AutomatStart()
{
    initializer_list<string> l{TOPIC_STATE, TOPIC_GO_TO_STATE};
    client = make_unique<Client>(l);
}

bool AutomatStart::IsWorking()
{
    return client->IsFinished();
}
 
string AutomatStart::GetStateMsg() 
{
    return IsWorking() ? "AutomatStart" : "AutomatEnd"; 
};

