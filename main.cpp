#if defined(_WRS_KERNEL)
#include <OsWrapper.h>
#endif
#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <memory>
#include <map>

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

/// \brief check key code
/// \return true if keq Q was pressed
bool KbHitQ()
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

void PrintCurrAutomatState()
{
	cout << currAuto->GetStateMsg() << endl;
}

void SetState(const char* topic, int to_state)
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
    PrintCurrAutomatState();

    if (to_state == AUTOMAT_STATE_END)
      endOfWork = true;
  }
}

int main(int argc, char* argv[])
{	
	PrintCurrAutomatState();

	Client client({TOPIC_STATE, TOPIC_GO_TO_STATE}, SetState);

	printf("\nPress Q<Enter> to quit\n\n");
	int ch;
	do
	{
	}
  while (!KbHitQ() || !endOfWork);

 	return 0;
}