#include "log.h"

int getTime(char *out, int fmt)
{
	if(out == NULL)
	{
	    return -1;
	}
	time_t t;
	struct tm *tp;
	t = time(NULL);

	tp = localtime(&t);
	if(fmt == 0)
	{
	    sprintf(out, "%2.2d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", tp->tm_year+2000, tp->tm_mon+1, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);
	}
	else if(fmt == 1)
	{
	    sprintf(out, "%2.2d-%2.2d-%2.2d", tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday);
	}
	else if(fmt == 2)
	{
	    sprintf(out, "%2.2d:%2.2d:%2.2d", tp->tm_hour, tp->tm_min, tp->tm_sec);
	}
	return 0;
}

int logging(char *msg, log_type type)
{
    char curTime[100] = {0,};

    if (msg == NULL) return -1
    switch(type)
    {
    case NORMAL:
       fprintf(logfd, "[%s] : %s\n", curTime, msg);
	   break;
    case ERROR:
       fprintf(logfd, "Error: [%s] : %s\n", curTime, msg);
	   break;
    default:
	   break;
    }

}
