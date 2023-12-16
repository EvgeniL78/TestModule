#include "automat.h"

#define TOPIC_STATE      	"State"
#define TOPIC_GO_TO_STATE	"GoToState"
#define TOPIC_TIME_ELAPSED	"TimeElapsed"
#define TOPIC_TIME			"Time"

AutomatStart::AutomatStart()
{
    Client client({TOPIC_STATE, TOPIC_GO_TO_STATE});
}