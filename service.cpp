#include <iostream>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <chrono>
#include <filesystem>
#include <cstdlib>

#include "service.h"

using namespace std;
namespace fs = std::filesystem;

namespace
{
    EPrintMode printMode = EPrintMode::log_all;
}

/// \brief parseints of commang line arguments
/// \param count of agruments
/// \param parameters of agruments
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

/// \brief check key code
/// \return true if keq Q was pressed
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

/// \brief returns timestamp as string (HH:MM:SS)
/// \return timestamp
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

void runApp(std::string& name)
{
    system(name.data());
}

void setPrintMode(EPrintMode m)
{
    printMode = m;
}

void printLog(ELogType t, std::string s)
{
    if (printMode == EPrintMode::log_none)
        return;
    
    if (printMode == EPrintMode::log_base &&
        t == ELogType::plain)
        return;
    
    cout << s << endl;
}