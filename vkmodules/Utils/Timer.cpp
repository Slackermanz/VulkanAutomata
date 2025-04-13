#include "Timer.h"
#include "Logger.h"
#include <iostream>
#include <string>

NS_Timer start_timer(NS_Timer t) {
    t.st = std::chrono::high_resolution_clock::now();
    return t;
}

void end_timer(NS_Timer t, std::string msg) {
    t.ft = std::chrono::high_resolution_clock::now();
    std::string ftime = std::to_string(
        std::chrono::duration_cast<std::chrono::nanoseconds>(t.ft-t.st).count()) + " ns, " +
        std::to_string(int(1000000000.0 / std::chrono::duration_cast<std::chrono::nanoseconds>(t.ft-t.st).count())) + " FPS";
    ov(msg, ftime);
}

void framesleep(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void tog(bool *b) {
    *b = (*b) ? false : true;
}