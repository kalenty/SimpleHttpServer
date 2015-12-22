#include "config.h"
#include <time.h>

enum log_type{ ERROR, NORMAL} log_type;


int logging(char *msg, log_type);

int log_init();