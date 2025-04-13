#ifndef VKMODULES_TIMER_H
#define VKMODULES_TIMER_H

#include <string>
#include <chrono>
#include <thread>
#include "../Types/Types.h"

// Start a timer
NS_Timer start_timer(NS_Timer t);

// End a timer and output the elapsed time
void end_timer(NS_Timer t, std::string msg);

// Sleep for specified milliseconds
void framesleep(int ms);

// Toggle a boolean value
void tog(bool *b);

#endif // VKMODULES_TIMER_H