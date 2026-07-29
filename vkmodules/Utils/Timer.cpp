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
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t.ft-t.st).count();
    std::string ftime = (ns > 0)
        ? std::to_string(ns) + " ns, " + std::to_string(int(1000000000.0 / ns)) + " FPS"
        : "0 ns, INF FPS";
    ov(msg, ftime);
}

void framesleep(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void tog(bool *b) {
    *b = (*b) ? false : true;
}