#if defined(_WRS_KERNEL)
#include <OsWrapper.h>
#endif
#include <iostream>
#include <type_traits>
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
shared_ptr<Automat> currAuto;

void setAutomatToState(const char* topic, int to_state)
{
  string t(topic);
  if (!t.compare(TOPIC_STATE) || 
      !t.compare(TOPIC_GO_TO_STATE))
  {
    switch(to_state)
    {
      case AUTOMAT_STATE_BEGIN:
      case AUTOMAT_STATE_END:
      case AUTOMAT_STATE_2:
        break;

      case AUTOMAT_STATE_OTHER:
      default:
        to_state = AUTOMAT_STATE_OTHER;
        break;
    }
   
    currAuto = autoStates[to_state];
   	cout << currAuto->GetStateMsg() << endl;
  }
}

int main(int argc, char* argv[])
{
  setAutomatToState(TOPIC_GO_TO_STATE, AUTOMAT_STATE_BEGIN);

  // Arg parser

    string app;
    switch (cmdArgsParser(argc, argv, app))
    {
      case EArgParams::log_none:
        setPrintMode(EPrintMode::log_none);
        break;
      case EArgParams::log_base:
        setPrintMode(EPrintMode::log_base);
        break;
      case EArgParams::log_all:
        setPrintMode(EPrintMode::log_all);
        break;

      case EArgParams::run_app:
        runApp(app);
      default:
        setAutomatToState(TOPIC_GO_TO_STATE, AUTOMAT_STATE_END);
        return 0;
    }

    // Mqtt client

    Client client({TOPIC_STATE, TOPIC_GO_TO_STATE}, setAutomatToState);

    // Main thread

    chrono::milliseconds timespan(500);
    this_thread::sleep_for(timespan);
    auto begin = std::chrono::high_resolution_clock::now();

    printLog(ELogType::plain, "\nPress Q<Enter> to quit\n\n");
    do
    {
      // publish time from start

      int64_t fromStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin).count();
      string msg = to_string(fromStart);
      client.Publish(TOPIC_TIME_ELAPSED, msg);

      // publish current time

      msg = getCurrTime();
      client.Publish(TOPIC_TIME, msg);

      //

      if (kbHitQ())
        setAutomatToState(TOPIC_GO_TO_STATE, AUTOMAT_STATE_END);

      this_thread::sleep_for(timespan);
    }
    while (is_same_v<decltype(currAuto), AutomatEnd>);

 	return 0;
}