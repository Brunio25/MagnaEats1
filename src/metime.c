#include "../include/metime.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// The clock_gettime() function gets the current time of the clock specified by clock_id, 
// and puts it into the buffer pointed to by tp. The only supported clock ID is CLOCK_REALTIME. 
void markTime(struct timespec *timeS)
{
    if (clock_gettime(CLOCK_REALTIME, timeS) == -1)
    {
        perror("clock error");
        exit(1);
    }
}

