#include "../include/client.h"
#include "../include/driver.h"
#include "../include/main.h"
#include "../include/restaurant.h"
#include "../include/metime.h"
#include "../include/memory.h"

#include <stdio.h>
#include <time.h>

// The clock_gettime() function gets the current time of the clock specified by clock_id, 
// and puts it into the buffer pointed to by tp. The only supported clock ID is CLOCK_REALTIME. 
int markTime(time_t timeS)
{
    if (clock_gettime(CLOCK_REALTIME, &timeS) == -1)
    {
        perror("clock error");
        exit(0);
    }
}

