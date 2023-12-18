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

#define CLIENTID "ExampleClient_main"

using namespace std;

namespace
{
  #define AUTOMAT_STATE_START 0
  #define AUTOMAT_STATE_STOP  1
  #define AUTOMAT_STATE_2     2
  #define AUTOMAT_STATE_ANOTHER 3

  map<int, shared_ptr<Automat>> autoStates = {
    { AUTOMAT_STATE_START, make_shared<AutomatStart>() },
    { AUTOMAT_STATE_STOP, make_shared<AutomatStop>() },
    { AUTOMAT_STATE_2, make_shared<AutomatState2>() },
    { AUTOMAT_STATE_ANOTHER, make_shared<AutomatStateAnother>() }
  };
  shared_ptr<Automat> currAuto;

  bool publishTime{false};
}

// Returns current automan state
int getAutomatState()
{
  for (const auto&[key, mk_shr_ptr] : autoStates)
  {
    if (currAuto == mk_shr_ptr)
      return key;
  }
  return -1;
}

/// Changes curr automat state
void setAutomatToState(int to_state)
{
  auto currState = getAutomatState();
  if ((currState >= AUTOMAT_STATE_START
      && to_state <= AUTOMAT_STATE_START) ||
      (currState == AUTOMAT_STATE_STOP))
  {
    printLog(ELogType::base, "Error in new state value");
    return;
  }
  if (currState == to_state)
    return;

  switch(to_state)
  {
    case AUTOMAT_STATE_START:
    case AUTOMAT_STATE_STOP:
    case AUTOMAT_STATE_2:
      break;

    case AUTOMAT_STATE_ANOTHER:
    default:
      to_state = AUTOMAT_STATE_ANOTHER;
      break;
  }
  
  currAuto = autoStates[to_state];
  printLog(ELogType::base, currAuto->GetStateMsg());
}

// Called from mqtt client
void clientRecived(const char* topic, string msg)
{
  string t(topic);

  if (!t.compare(TOPIC_GO_TO_STATE))
  {
    int to_state{};
	  sscanf(msg.data(), "%d", &to_state);
    setAutomatToState(to_state);
  }

  if (!t.compare(TOPIC_GET_TIME))
    publishTime = true;
}

int waitAppFinished();

int main(int argc, char* argv[])
{
  setAutomatToState(AUTOMAT_STATE_START);

  // Arg parser

    string app;

    switch (cmdArgsParser(argc, argv, app))
    {
      case EArgParams::log_none:
        setPrintMode(EPrintMode::log_none);
        break;
      case EArgParams::log_base:
        setPrintMode(EPrintMode::log_base);
        printLog(ELogType::base, "Log mode: base");
        break;
      case EArgParams::log_all:
        setPrintMode(EPrintMode::log_all);
        printLog(ELogType::base, "Log mode: all");
        break;

      case EArgParams::run_app:
        setAutomatToState(AUTOMAT_STATE_STOP);
        printLog(ELogType::base, "Run app: " + app);
        runApp(app);
        return waitAppFinished();
        break;

      default:
        setAutomatToState(AUTOMAT_STATE_STOP);
        return 0;
    }

    // MQTT client

    Client client(CLIENTID, {TOPIC_GET_TIME, TOPIC_GO_TO_STATE}, clientRecived); // list of subscriptions
    if (client.IsFinished())
    {
      printLog(ELogType::base, "MQTT client failed");
      setAutomatToState(AUTOMAT_STATE_STOP);
      return 1;
    }

    // Main thread

    chrono::milliseconds timespan500ms(500);
    this_thread::sleep_for(timespan500ms);
    auto begin = chrono::high_resolution_clock::now();
    auto dur3sec = chrono::high_resolution_clock::now();

    printLog(ELogType::plain, "\nPress Q<Enter> to quit\n");
    AutomatStop* p = nullptr;
    do
    {
      // publish

      auto now = chrono::high_resolution_clock::now();
      if (chrono::duration_cast<chrono::seconds>(now - dur3sec).count() >= 3)
      {
        dur3sec = now;

        client.Publish(TOPIC_STATE, to_string(getAutomatState())); // state status
        int64_t fromStart = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - begin).count();
        client.Publish(TOPIC_TIME_ELAPSED, to_string(fromStart)); // time from start

        if (publishTime)
        {
          publishTime = false;
          client.Publish(TOPIC_TIME, getCurrTime()); // current time
        }
      }

      // checking exit

      if (kbHitQ())
      {
        printLog(ELogType::plain, "\n[Q] key was pressed - quit");
        setAutomatToState(AUTOMAT_STATE_STOP);
      }

      this_thread::sleep_for(timespan500ms);

      p = dynamic_cast<AutomatStop*>(currAuto.get());
    }
    while (!p);

 	return 0;
}

int waitAppFinished()
{
  chrono::milliseconds timespan500ms(500);

  printLog(ELogType::plain, "\nPress Q<Enter> to quit\n");
  do
  {
    if (appFinished())
      break;

    if (kbHitQ())
    {
      printLog(ELogType::plain, "\n[Q] key was pressed - quit");
      return 1;
    }

    this_thread::sleep_for(timespan500ms);
  }
  while (true);
  
  return 0;
}