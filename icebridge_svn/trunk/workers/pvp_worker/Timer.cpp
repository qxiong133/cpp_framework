#include "Timer.h"
#include <time.h>
#include <sys/time.h>


namespace utils {

    Timer::Timer(unsigned int ms, bool createActive) {
        active = createActive;
        interval = ms;
        lastpulse = getTimeInMillisec();
    }

    void Timer::sleep() {
        if (!active) return;
        uint64_t now = getTimeInMillisec();
        if (now - lastpulse >= interval) {
            return;
        }
        struct timespec req;
        req.tv_sec = 0;
        req.tv_nsec = (interval - (now - lastpulse)) * (1000 * 1000);
        nanosleep(&req, 0);
    }

    int Timer::poll() {
        int elapsed = 0;
        if (active) {
            uint64_t now = getTimeInMillisec();
            if (now > lastpulse) {
                elapsed = (now - lastpulse) / interval;
                lastpulse += interval * elapsed;
            } else {
                // Time has made a jump to the past. This should be a rare
                // occurence, so just reset lastpulse to prevent problems.
                lastpulse = now;
            }
        };
        return elapsed;
    }

    void Timer::start() {
        active = true;
        lastpulse = getTimeInMillisec();
    }

    void Timer::stop() {
        active = false;
    }

    void Timer::changeInterval(unsigned int newinterval) {
        interval = newinterval;
    }

    uint64_t Timer::getTimeInMillisec() {
        uint64_t timeInMillisec;
        timeval time;

        gettimeofday(&time, 0);
        timeInMillisec = (uint64_t) time.tv_sec * 1000 + time.tv_usec / 1000;
        return timeInMillisec;
    }

} // ::utils
