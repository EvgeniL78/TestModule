#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"

#if defined(_WRS_KERNEL)
#include <OsWrapper.h>
#endif

#define TOPIC_STATE      	"State"
#define TOPIC_GO_TO_STATE	"GoToState"
#define TOPIC_TIME_ELAPSED	"TimeElapsed"
#define TOPIC_TIME			"Time"

int main(int argc, char* argv[])
{
	Client client({TOPIC_STATE, TOPIC_GO_TO_STATE});

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