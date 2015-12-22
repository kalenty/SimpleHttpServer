#include "cfg.h"
#include <time.h>
#include <stdio.h>
enum log_type{ ERROR, NORMAL}; 


int logging(char *cause, char *msg, enum log_type type);

int getTime(char *out, int fmt);

int log_init();
