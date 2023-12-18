#include <iostream>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <chrono>
#include <filesystem>
#include <cstdlib>
#include <thread>
#include <atomic>

#include "service.h"

using namespace std;
namespace fs = std::filesystem;

namespace
{
    /// Mode of writing messages to console
    EPrintMode printMode = EPrintMode::log_all;

    /// Results of running another app in other thread
    atomic_int appRes(-1);
}

/// @brief Parseints of commang line arguments
/// @param count of agruments
/// @param parameters of agruments
/// @param app - application for running
/// @return type of act for curr app
EArgParams cmdArgsParser(uint argc, char* argv[], string& app)
{
    switch (argc)
    {
        case 2:
            if (!string(argv[1]).compare("-s"))
                return EArgParams::log_none;
            break;

        case 3:
            if (!string(argv[1]).compare("-d"))
            {
                if (!string(argv[2]).compare("0"))
                    return EArgParams::log_base;
                if (!string(argv[2]).compare("1"))
                    return EArgParams::log_all;
            }            
            if (!string(argv[1]).compare("-p"))
            {
                app = string(argv[2]);
                if (fs::exists(app)) 
                    return EArgParams::run_app;
            }
            break;
    }

  return EArgParams::error;
}

/// @brief Checks key code
/// @return true if keq Q was pressed
bool kbHitQ()
{
  static const int STDIN = 0;
  static bool initialized = false;

  if (!initialized) {
    termios term;
    tcgetattr(STDIN, &term);
    term.c_lflag &= ~ICANON;
    tcsetattr(STDIN, TCSANOW, &term);
    setbuf(stdin, NULL);
    initialized = true;
  }

  int bytes_waiting;
  ioctl(STDIN, FIONREAD, &bytes_waiting);
  if (bytes_waiting)
  {
    char ch{};
    cin >> ch;
    return (ch == 'q' || ch == 'Q');
  }
  return false;
}

/// @brief returns timestamp as string (HH:MM:SS)
/// @return timestamp
string getCurrTime()
{
    using chrono::system_clock;
    auto currentTime = chrono::system_clock::now();
    char buffer[80]{};
    time_t tt;
    tt = system_clock::to_time_t (currentTime);
    auto timeinfo = localtime (&tt);
    strftime (buffer, 80, "%H:%M:%S", timeinfo);
    return std::string(buffer);
}

/// @brief Runs another app in thread
/// @name file name
void runApp(std::string& name)
{
    thread th([name] {
            int res = system(name.data());
            appRes.store(res);
         });
    th.detach();
}

/// @brief Checks if another app is working or not
/// @return - true is app finished
bool appFinished()
{
    return (appRes.load() > -1);
}

/// @brief Sets mode for writing log messages to console
/// @param m - mode
void setPrintMode(EPrintMode m)
{
    printMode = m;
}

/// @brief Writes msg to console
/// @param t - msg type
/// @param s - msg
void printLog(ELogType t, std::string s)
{
    if (printMode == EPrintMode::log_none)
        return;
    
    if (printMode == EPrintMode::log_base &&
        t == ELogType::plain)
        return;
    
    cout << s << endl;
}
