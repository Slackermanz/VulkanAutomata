#include "Logger.h"
#include <iostream>

// Global log level variable - defined here
int loglevel = 2;

void hd(const std::string& id, const std::string& msg) {
    // Header output message
    std::string bar = "";
    for(int i = 0; i < 20; i++) { bar = bar + "____"; }
    if(loglevel >= 0) {
        std::cout << bar << "\n " << id << "\t" << msg << "\n"; }
}

void rv(const std::string& id) {
    // Return void output message
    if(loglevel >= 2) {
        std::cout << "  void: \t" << id << "\n"; }
}

void nt(const std::string& id) {
    // Notification output message
    if(loglevel >= 2) {
        std::cout << id << "\n"; }
}