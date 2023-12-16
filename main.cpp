#if defined(_WRS_KERNEL)
#include <OsWrapper.h>
#endif
#include <iostream>
#include <memory>

#include "automat.h"

using namespace std;

void PrintCurrAutomatState(const unique_ptr<Automat>& a)
{
	cout << a->GetStateMsg() << endl;
}

int main(int argc, char* argv[])
{
	unique_ptr<AutomatStart> automat_start = make_unique<AutomatStart>();
	if (!automat_start->IsWorking())
		return 1;
	PrintCurrAutomatState(dynamic_cast<Automat>(automat_start));

	printf("\nPress Q<Enter> to quit\n\n");
	int ch;
	do
	{
		ch = getchar();
	} while (ch!='Q' && ch != 'q');

 	return 0;
}