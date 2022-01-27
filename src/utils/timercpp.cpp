#include <iostream>
#include <thread>
#include <chrono>
#include "timercpp.h"

void Timer::setTimeout(void (*function)(), int delay) {
    this->clear = false;
    std::thread t([=]() {
        if(this->clear) return;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        if(this->clear) return;
        function();
		this->clear = false;
    });
    t.detach();
}

void Timer::setInterval(void (*function)(), int interval) {
    this->clear = false;
    std::thread t([=]() {
        while(true) {
            if(this->clear) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            if(this->clear) return;
            function();
        }
    });
    t.detach();
}

void Timer::stop() {
    this->clear = true;
}

bool Timer::isRunning() {
	return this->clear == false;
}