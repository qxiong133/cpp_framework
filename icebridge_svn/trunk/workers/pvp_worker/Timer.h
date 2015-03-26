#ifndef TIMER_H
#define TIMER_H

#include <stdint.h> // on other compilers use the C99 official header

namespace utils {

    /**
     * This class is for timing purpose as a replacement for SDL_TIMER
     */
    class Timer {
    public:
        /**
         * Constructor.
         */
        Timer(unsigned int ms, bool createActive = true);

        /**
         * Returns the number of elapsed ticks since last call.
         */
        int poll();

        /**
         * Sleeps till the next tick occurs.
         */
        void sleep();

        /**
         * Activates the timer.
         */
        void start();

        /**
         * Deactivates the timer.
         */
        void stop();

        /**
         * Changes the interval between two pulses.
         */
        void changeInterval(unsigned int newinterval);

    private:
        /**
         * Calls gettimeofday() and converts it into milliseconds.
         */
        uint64_t getTimeInMillisec();

        /**
         * Interval between two pulses.
         */
        unsigned int interval;

        /**
         * The time the last pulse occured.
         */
        uint64_t lastpulse;

        /**
         * Activity status of the timer.
         */
        bool active;
    };

}

#endif

