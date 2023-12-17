#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <chrono>

#include "service.h"

using namespace std;

/// \brief parseints of commang line arguments
/// \param count of agruments
/// \param parameters of agruments
int cmdArgsParser(uint argc, char** argv, string& app, int& code)
{
  switch (argc)
  {
    case 1:
      error = ERROR_ARGS_NOT_ENOUGH;
      break;
    case 2:
      if (string(argv[1]) == "-s")
        flag = EFlags::kServer;
      else
        error = ERROR_ARGS_SERVER;
      break;

    case 3:
      if (string(argv[1]) == "-c") {
        // Path to files for sending

        string file(argv[2]);
        if (!fs::exists(file)) {
          error = ERROR_ARGS_CLIENT;
          break;
        }

        arg_file = file;
        flag = EFlags::kClient;
      }
      else error = ERROR_ARGS_CLIENT;
      break;

    default:
      error = ERROR_ARGS;
      break;
  }
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
  return (bytes_waiting == 'q' || bytes_waiting == 'Q');
}

/// \brief returns timestamp as string (HH:MM:SS)
/// \return timestamp
string getCurrTime()
{
    using chrono::system_clock;
    auto currentTime = chrono::system_clock::now();
    char buffer[80]{};

    //auto transformed = currentTime.time_since_epoch().count() / 1000000;
    //auto millis = transformed % 1000;

    time_t tt;
    tt = system_clock::to_time_t (currentTime);
    auto timeinfo = localtime (&tt);
    strftime (buffer, 80, "%H:%M:%S", timeinfo);
    //char buffer2[100]{};
    //sprintf(buffer2, "%s:%03d", buffer, (int)millis);

    return std::string(buffer);
}

void runApp(std::string& name)
{

}