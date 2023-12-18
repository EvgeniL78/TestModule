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

#define CLIENTID "ExampleClient_tester"

using namespace std;


int main(int argc, char* argv[])
{
    setPrintMode(EPrintMode::log_base);
    printLog(ELogType::base, "Log mode: base");

    // MQTT client

    Client client(CLIENTID, {TOPIC_STATE, TOPIC_TIME, TOPIC_TIME_ELAPSED}, nullptr);
    if (client.IsFinished())
    {
      printLog(ELogType::base, "MQTT client failed");
      return 1;
    }

    // Main thread

    chrono::milliseconds timespan500ms(500);
    this_thread::sleep_for(timespan500ms);
    auto begin = chrono::high_resolution_clock::now();
    auto dur3sec = chrono::high_resolution_clock::now();
    auto dur5sec = chrono::high_resolution_clock::now();
    bool state_2_3 = false;

    printLog(ELogType::plain, "\nPress Q<Enter> to quit\n");
    do
    {
      // publish

      auto now = chrono::high_resolution_clock::now();
      if (chrono::duration_cast<chrono::seconds>(now - dur3sec).count() >= 3)
      {
        dur3sec = now;
        state_2_3 = !state_2_3; 
        client.Publish(TOPIC_GO_TO_STATE, state_2_3 ? "2" : "3"); // change automat state by GoToState topic
      }
      if (chrono::duration_cast<chrono::seconds>(now - dur5sec).count() >= 5)
      {
        dur5sec = now;
        client.Publish(TOPIC_GET_TIME, "."); // request time
      }      

      // checking exit

      if (kbHitQ())
      {
        printLog(ELogType::plain, "\n[Q] key was pressed - quit");
        client.Publish(TOPIC_GO_TO_STATE, "1"); // stop automat
        this_thread::sleep_for(timespan500ms);
        break;
      }

      this_thread::sleep_for(timespan500ms);
    }
    while (true);

 	return 0;
}