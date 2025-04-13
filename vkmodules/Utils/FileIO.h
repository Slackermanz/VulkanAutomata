#ifndef VKMODULES_FILEIO_H
#define VKMODULES_FILEIO_H

#include <string>
#include <cstdint>
#include "../Types/Types.h"

// Save image to PAM format
void save_image(void* image_data, std::string fname, uint32_t w, uint32_t h, GLFW_mouse m, bool cursor = false);

// Save sound data to binary files
void save_sound(void* image_data, std::string fname, uint32_t w, uint32_t h, GLFW_mouse m, bool cursor = false);

// Save frequency spectrum data as image
void save_fspec(fsmag256 *fsm, std::string fname, uint32_t w, uint32_t h);

// Initialize a frequency spectrum structure
void new_fspec256(fspec256 *fs);

#endif // VKMODULES_FILEIO_H