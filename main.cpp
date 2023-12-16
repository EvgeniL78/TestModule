#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"

#if defined(_WRS_KERNEL)
#include <OsWrapper.h>
#endif



int main(int argc, char* argv[])
{

	while (!subscribed && !finished)
	{
		usleep(10000L);
	}

	if (finished)
		return 1;

	printf("\nPress Q<Enter> to quit\n\n");
	int ch;
	do
	{
		ch = getchar();
	} while (ch!='Q' && ch != 'q');

 	return 0;
}