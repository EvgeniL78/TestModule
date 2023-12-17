#if defined(_WRS_KERNEL)
#include <OsWrapper.h>
#endif
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <memory>
#include <map>

#include "service.h"
#include "client.h"
#include "automat.h"

using namespace std;

#define AUTOMAT_STATE_BEGIN 0
#define AUTOMAT_STATE_END   1
#define AUTOMAT_STATE_2     2
#define AUTOMAT_STATE_OTHER 3

map<int, shared_ptr<Automat>> autoStates = {
  { AUTOMAT_STATE_BEGIN, make_shared<AutomatStart>() },
  { AUTOMAT_STATE_END, make_shared<AutomatEnd>() },
  { AUTOMAT_STATE_2, make_shared<AutomatState2>() },
  { AUTOMAT_STATE_OTHER, make_shared<AutomatStateOther>() }
};
shared_ptr<Automat> currAuto = autoStates[AUTOMAT_STATE_BEGIN];

bool endOfWork = false;

void printCurrAutomatState()
{
	cout << currAuto->GetStateMsg() << endl;
}

void setAutomatToState(const char* topic, int to_state)
{
  string t(topic);
  if (!t.compare(TOPIC_STATE) || 
      !t.compare(TOPIC_GO_TO_STATE))
  {
    switch(to_state)
    {
      case AUTOMAT_STATE_END:
      case AUTOMAT_STATE_2:
      break;

      case AUTOMAT_STATE_OTHER:
      default:
        to_state = AUTOMAT_STATE_OTHER;
      break;
    }
   
    currAuto = autoStates[to_state];
    printCurrAutomatState();

    if (to_state == AUTOMAT_STATE_END)
      endOfWork = true;
  }
}

int main(int argc, char* argv[])
{	
	printCurrAutomatState();

  // arg parser

  // Mqtt client

	Client client({TOPIC_STATE, TOPIC_GO_TO_STATE}, setAutomatToState);

  // Main thread

	printf("\nPress Q<Enter> to quit\n\n");

  chrono::milliseconds timespan(500);
  this_thread::sleep_for(timespan);
  auto begin = std::chrono::high_resolution_clock::now();

	do
	{
    // publish time from start

    int64_t fromStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin).count();
    string msg = to_string(fromStart);
    client.Publish(TOPIC_TIME_ELAPSED, msg);

    // publish current time

    msg = getCurrTime();
    client.Publish(TOPIC_TIME, msg);

    this_thread::sleep_for(timespan);
	}
  while (
    !kbHitQ() || // Q
    !endOfWork || // to end state
    !client.IsFinished()); // client was broke

  if (!endOfWork) // if Q was pressed or client was broke
  {
    setAutomatToState(TOPIC_GO_TO_STATE, 1);
	  printCurrAutomatState();
  }

 	return 0;
}