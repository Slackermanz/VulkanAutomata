#ifndef VKMODULES_PATTERN_MANAGER_H
#define VKMODULES_PATTERN_MANAGER_H

#include <string>
#include <cstdint>
#include <fstream>
#include "../Types/Types.h"
#include "../Utils/Logger.h"
#include "../Utils/FileIO.h" // Added for new_fspec256

// Load a PatternConfigData_408 from a file
PatternConfigData_408 get_PCD_408(std::string loadfile, int idx, EngineInfo *ei);

// Create a new empty UB32_64 pattern
UB32_64 new_PCD_256();

// Convert pattern to string representation for display
std::string show_PCD256(UB32_64 *pcd);

// Save a UB32_64 pattern to a file
bool save_PCD256(std::string savefile, UB32_64 *pcd);

// Get the number of legacy PCD408 patterns in a file
int get_PCD408_count(std::string loadfile);

// Get the number of PCD256 patterns in a file
int get_PCD256_count(std::string loadfile);

// Load a UB32_64 pattern from a file
UB32_64 load_PCD256(std::string loadfile, int idx);

// Load a WAVS16_1024 pattern from a file
WAVS16_1024 load_WAVS16(std::string loadfile, int idx);

// Load a PatternConfigData_408 into a UB32_64
void loadPattern_PCD408_to_256(EngineInfo *ei, UB32_64 *pcd);

// Update uniform buffer with pattern data
void update_ub(UB32_64 *pcd, UB32_64 *ub);

// Create a new fsmag256 structure with zeroed values
fsmag256 new_fsmag256();

// Functions for bit manipulation in patterns
uint32_t u32_flp(uint32_t u32, uint32_t off);
uint32_t u32_set(uint32_t u32, uint32_t off);
uint32_t u32_clr(uint32_t u32, uint32_t off);
uint32_t mut_rnd();
uint32_t mutation_period_from_strength(int strength);
uint32_t blk_clr(uint32_t u32);
uint32_t blk_set(uint32_t u32);
uint32_t wrd_clr(uint32_t u32, uint32_t off, uint32_t len);
uint32_t wrd_set(uint32_t u32, uint32_t off, uint32_t len);
uint32_t wrd_flp(uint32_t u32, uint32_t off, uint32_t len);
uint32_t bit_flp(uint32_t u32, uint32_t rnd);

// Discrete Fourier Transform functions
void dft1d(int idx, int smp, fspec256* fs, fsmag256* fsm);

#endif // VKMODULES_PATTERN_MANAGER_H