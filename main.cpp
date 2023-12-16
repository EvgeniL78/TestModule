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

void PrintCurrAutomatState(shared_ptr<Automat> a)
{
	cout << a->GetStateMsg() << endl;
}

  /// \brief returns key code if key of keyboard was pressed
  /// \return key code
  int KbHit()
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
    return bytes_waiting;
  }

int main(int argc, char* argv[])
{
	map<int, shared_ptr<Automat>> autoStates = {
	 { AUTOMAT_STATE_BEGIN, make_shared<AutomatStart>() },
	 { AUTOMAT_STATE_END, make_shared<AutomatEnd>() },
	 { AUTOMAT_STATE_2, make_shared<AutomatState2>() },
	 { AUTOMAT_STATE_OTHER, make_shared<AutomatStateOther>() }
	};
	shared_ptr<Automat> currAuto = autoStates[AUTOMAT_STATE_BEGIN];
	
	PrintCurrAutomatState(currAuto);

	Client client({TOPIC_STATE, TOPIC_GO_TO_STATE});

	printf("\nPress Q<Enter> to quit\n\n");
	int ch;
	do
	{
		ch = getchar();
	} while (ch!='Q' && ch != 'q');

 	return 0;
}