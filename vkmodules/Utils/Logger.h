#ifndef VKMODULES_LOGGER_H
#define VKMODULES_LOGGER_H

#include <string>
#include <vector>
#include <iostream>
#include <vulkan/vulkan.h>

// Global log level variable
extern int loglevel;

// Header output message
void hd(const std::string& id, const std::string& msg);

// Single info output message
template<typename T>
void ov(const std::string& id, T v) {
    int         padlen  =  4;
    int         pads    = 11;
    std::string pad     = " ";
    int         padsize = (pads*padlen - id.size()) - 3;
    for(int i = 0; i < padsize; i++) { pad = pad + "."; }
    if(loglevel >= 1) {
        std::cout << "\tinfo:\t    " << id << pad << " [" << v << "]\n"; }
}

// Multiple info output message
template<typename T>
void iv(const std::string& id, T ov, int idx) {
    int         padlen  = 4;
    int         pads    = 10;
    std::string pad     = " ";
    int         padsize = (pads*padlen - id.size()) - 3;
    for(int i = 0; i < padsize; i++) { pad = pad + "."; }
    if(loglevel >= 1) {
        std::cout << "\tinfo:\t" << idx << "\t" << id << pad << " [" << ov << "]\n"; }
}

// VkResult output message
template<typename T>
void vr(const std::string& id, std::vector<VkResult>* reslist, T v, VkResult res) {
    const size_t max_history = 4096;
    if(reslist->size() >= max_history) {
        reslist->erase(reslist->begin());
    }
    reslist->push_back(res);
    uint32_t     idx         = reslist->size() - 1;
    std::string  idx_string  = std::to_string(idx);
    uint32_t     idx_sz      = idx_string.size();
    std::string res_string   = std::to_string(res);
    if(idx_sz < 4) { for(int i = 0; i < 4-idx_sz; i++) { idx_string = " " + idx_string; } }
    int         padlen  = 4;
    int         pads    = 12;
    std::string pad     = " ";
    int         padsize = (pads*padlen - id.size()) - 3;
    for(int i = 0; i < padsize; i++) { pad = pad + " "; }
    if(loglevel >= 2) {
        std::cout << "  " << idx_string << ":\t" << (res==0?" ":res_string) << " \t" << id << pad << " [" << v << "]\n"; }
}

// Return void output message
void rv(const std::string& id);

// Notification output message
void nt(const std::string& id);

// NullFlags shorthand for Vulkan objects
template<typename T>
void nf(T *Vk_obj) {
    Vk_obj->pNext = NULL;
    Vk_obj->flags = 0;
}

#endif // VKMODULES_LOGGER_H