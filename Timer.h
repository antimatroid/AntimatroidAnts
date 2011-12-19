#ifndef TIMER_H_
#define TIMER_H_

#include <sys/time.h>

struct Timer
{
    timeval start_time;

    // starts the timer
    void start()
    {
        gettimeofday(&start_time, NULL);
    };

    // returns how long it has been since the timer was last started in microseconds
    double getTime()
    {
        timeval current_time;
        gettimeofday(&current_time, NULL);
        return (current_time.tv_sec - start_time.tv_sec) * 1000.0
            + (current_time.tv_usec - start_time.tv_usec) / 1000.0;
    };
};

#endif //TIMER_H_
