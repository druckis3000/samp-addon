#ifndef TIMERCPP_H
#define TIMERCPP_H

class Timer {
    bool clear = false;

    public:
        void setTimeout(void (*function)(), int delay);
        void setInterval(void (*function)(), int interval);
        void stop();
		bool isRunning();

};

#endif