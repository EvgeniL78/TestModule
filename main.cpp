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

#define AUTOMAT_STATE_START 0
#define AUTOMAT_STATE_STOP  1
#define AUTOMAT_STATE_2     2
#define AUTOMAT_STATE_OTHER 3

map<int, shared_ptr<Automat>> autoStates = {
  { AUTOMAT_STATE_START, make_shared<AutomatStart>() },
  { AUTOMAT_STATE_STOP, make_shared<AutomatStop>() },
  { AUTOMAT_STATE_2, make_shared<AutomatState2>() },
  { AUTOMAT_STATE_OTHER, make_shared<AutomatStateOther>() }
};
shared_ptr<Automat> currAuto;

/// Cheanges curr automat state
void setAutomatToState(int to_state)
{
  switch(to_state)
  {
    case AUTOMAT_STATE_START:
    case AUTOMAT_STATE_STOP:
    case AUTOMAT_STATE_2:
      break;

    case AUTOMAT_STATE_OTHER:
    default:
      to_state = AUTOMAT_STATE_OTHER;
      break;
  }
  
  currAuto = autoStates[to_state];
  printLog(ELogType::base, currAuto->GetStateMsg());
}

// Called from mqtt client
void setAutomatToState(const char* topic, int to_state)
{
  string t(topic);
  if (!t.compare(TOPIC_STATE) || 
      !t.compare(TOPIC_GO_TO_STATE))
    setAutomatToState(to_state);
}

int main(int argc, char* argv[])
{
  setAutomatToState(AUTOMAT_STATE_START);

  // Arg parser

    string app;
    switch (cmdArgsParser(argc, argv, app))
    {
      case EArgParams::log_none:
        printLog(ELogType::base, "Log: silent mode");
        setPrintMode(EPrintMode::log_none);
        break;
      case EArgParams::log_base:
        printLog(ELogType::base, "Log: base mode");
        setPrintMode(EPrintMode::log_base);
        break;
      case EArgParams::log_all:
        printLog(ELogType::base, "Log: all mode");
        setPrintMode(EPrintMode::log_all);
        break;

      case EArgParams::run_app:
        printLog(ELogType::base, "Run app: " + app);
        runApp(app);
      default:
        setAutomatToState(AUTOMAT_STATE_STOP);
        return 0;
    }

    // MQTT client

    Client client({TOPIC_STATE, TOPIC_GO_TO_STATE}, setAutomatToState);
    if (client.IsFinished())
    {
      printLog(ELogType::base, "MQTT client is absent");
      setAutomatToState(AUTOMAT_STATE_STOP);
      return 1;
    }

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
      {
        printLog(ELogType::plain, "[Q] key was pressed - quit");
        setAutomatToState(AUTOMAT_STATE_STOP);
      }

      this_thread::sleep_for(timespan);
    }
    while (is_same_v<decltype(currAuto), AutomatStop>);

 	return 0;
}