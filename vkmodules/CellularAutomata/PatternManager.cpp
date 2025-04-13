#include "PatternManager.h"
#include <iostream>
#include <cstring>
#include <cmath>

PatternConfigData_408 get_PCD_408(std::string loadfile, int idx, EngineInfo *ei) {
	PatternConfigData_408 pcd;
	std::ifstream fload_pcd(loadfile.c_str(), std::ios::in | std::ios::binary);
		fload_pcd.seekg(0, fload_pcd.end);
		int f_len = fload_pcd.tellg();
		fload_pcd.seekg (( (idx + (f_len / sizeof(pcd))) % (f_len / sizeof(pcd))) * sizeof(pcd));
		fload_pcd.read((char*)&pcd, sizeof(pcd));
		ei->PCD_count = (f_len / sizeof(pcd));
	fload_pcd.close();
	std::cout << "\n\tPCD408 Data Count: " << idx << " / " << (f_len / sizeof(pcd)) << "\n"; // Report how many patterns are in data file
	return pcd;
}

UB32_64 new_PCD_256() {
	UB32_64 pcd;
		for(int i = 0; i < 64; i++) { pcd.u32[i] = 0; }
	return pcd;
}

std::string show_PCD256(UB32_64 *pcd) {
	std::string out_string = "\n  UB32_64:\n";
	for(int i = 0; i < 16; i++) {
		out_string = out_string + "    " + std::to_string(pcd->u32[(i*4)+0]);
		out_string = out_string + ", " 	 + std::to_string(pcd->u32[(i*4)+1]);
		out_string = out_string + ", " 	 + std::to_string(pcd->u32[(i*4)+2]);
		out_string = out_string + ", " 	 + std::to_string(pcd->u32[(i*4)+3]) + ", \n"; }
	return out_string + "\n";
}

void save_PCD256(std::string savefile, UB32_64 *pcd) {
	std::cout << "\nSAVE:" << show_PCD256(pcd);
	FILE* f = fopen(savefile.c_str(), "a");
	fwrite(pcd, sizeof(UB32_64), 1, f);
	fclose(f);
}

int get_PCD256_count(std::string loadfile) {
	int count = 0;
	std::ifstream fload_pcd256(loadfile.c_str(), std::ios::in | std::ios::binary);
		fload_pcd256.seekg(0, fload_pcd256.end);
		count = int(fload_pcd256.tellg() / sizeof(UB32_64));
	fload_pcd256.close();
	return count;
}

UB32_64 load_PCD256(std::string loadfile, int idx) {
	UB32_64 pcd = new_PCD_256();
	std::ifstream fload_pcd256(loadfile.c_str(), std::ios::in | std::ios::binary);
		fload_pcd256.seekg(0, fload_pcd256.end);
		int f_len = fload_pcd256.tellg();
		fload_pcd256.seekg (( (idx + (f_len / sizeof(pcd))) % (f_len / sizeof(pcd))) * sizeof(pcd));
		fload_pcd256.read((char*)&pcd, sizeof(pcd));
		std::cout << "\tLoad PCD256 Data Count: " << idx << " / " << (f_len / sizeof(pcd)) << "\n"; // Report how many patterns are in data file
	fload_pcd256.close();
	std::cout << "\nLOAD:" << show_PCD256(&pcd);
	return pcd;
}

WAVS16_1024 load_WAVS16(std::string loadfile, int idx) {
	WAVS16_1024 WAVS16;
	std::ifstream fload_WAVS16(loadfile.c_str(), std::ios::in | std::ios::binary);
		fload_WAVS16.seekg(0, fload_WAVS16.end);
		int f_len = fload_WAVS16.tellg();
		fload_WAVS16.seekg (( (idx + (f_len / sizeof(int16_t))) % (f_len / sizeof(int16_t))) * sizeof(int16_t));
		fload_WAVS16.read((char*)&WAVS16, sizeof(WAVS16_1024));
	fload_WAVS16.close();
	return WAVS16;
}

void loadPattern_PCD408_to_256(EngineInfo *ei, UB32_64 *pcd) {
	PatternConfigData_408 pcd_load 	= get_PCD_408("res/data/save_global.vkpat", ei->load_pattern, ei);
		for(int i = 0; i < 48; i++) { 
			pcd->u32[i] =  pcd_load.ubv_save[i]; }
	memcpy(&pcd->u32[62], &pcd_load.scl_save, sizeof(uint32_t));
	memcpy(&pcd->u32[61], &pcd_load.pzm_save, sizeof(uint32_t));
}

void update_ub(UB32_64 *pcd, UB32_64 *ub) {
	for(int i = 0; i < 64; i++) { ub->u32[i] =  pcd->u32[i]; }
}

// Bit manipulation functions
uint32_t u32_flp(uint32_t u32, uint32_t off) { return u32 ^ (1 << off); }
uint32_t u32_set(uint32_t u32, uint32_t off) { return u32 | (1 << off); }
uint32_t u32_clr(uint32_t u32, uint32_t off) { return u32 & (1 << off); }

uint32_t mut_rnd() 	{ return rand()%UINT32_MAX; }

uint32_t blk_clr(uint32_t u32) {
	for(int i = 0; i < 32; i++) { u32 = u32_clr(u32, i); }
	return u32;
}

uint32_t blk_set(uint32_t u32) {
	for(int i = 0; i < 32; i++) { u32 = u32_set(u32, i); }
	return u32;
}

uint32_t wrd_clr(uint32_t u32, uint32_t off, uint32_t len) {
	len = len - (((off+len)/32) * ((off+len)%32));
	for(int i = off; i < off+len; i++) { u32 = u32_set(u32, i); }
	return u32;
}

uint32_t wrd_set(uint32_t u32, uint32_t off, uint32_t len) {
	len = len - (((off+len)/32) * ((off+len)%32));
	for(int i = off; i < off+len; i++) { u32 = u32_set(u32, i); }
	return u32;
}

uint32_t wrd_flp(uint32_t u32, uint32_t off, uint32_t len) {
	len = len - (((off+len)/32) * ((off+len)%32));
	for(int i = off; i < off+len; i++) { u32 = u32_flp(u32, i); }
	return u32;
}

uint32_t bit_flp(uint32_t u32, uint32_t rnd) { 
	for(int i = 0; i < 32; i++) { if(rand()%rnd == 0) { u32 = u32_flp(u32, i); } }
	if(rand()%(rnd*2) == 0) { u32 = wrd_set( u32, rand()%32, rand()%8 ); }
	if(rand()%(rnd*2) == 0) { u32 = wrd_clr( u32, rand()%32, rand()%8 ); }
	if(rand()%(rnd*2) == 0) { u32 = wrd_flp( u32, rand()%32, rand()%8 ); }
	return u32;
}

// Create a new fsmag256 structure with zeroed values
fsmag256 new_fsmag256() {
	fsmag256 fsm;
	for(int i = 0; i < 256; i++) { fsm.fsm[i] = 0.0f; }
	return fsm;
}

// Discrete Fourier Transform functions
void dft1d(int idx, int smp, fspec256* fs, fsmag256* fsm) {
	new_fspec256(fs);
	WAVS16_1024 d1024 = load_WAVS16("input.wav", idx+150);
	for(int s = 0; s < smp; s++) {
		int16_t d = d1024.i16[s*8];
		for(int f = 0; f < (smp/2); f++) {
			double trigpi = (2.0 * M_PI * double(f) * double(s)) / double(smp);
			fs->rl[f/((smp/2)/256)] +=  float( (double(d)*cos(trigpi) + double(d)*sin(trigpi)) * (1.0f/smp) );
			fs->im[f/((smp/2)/256)] +=  float( (double(d)*cos(trigpi) - double(d)*sin(trigpi)) * (1.0f/smp) ); } }

	for(int i = 0; i < 256; i++) { fsm->fsm[i] = (fsm->fsm[i] * 0.65) + sqrt(fs->rl[i]*fs->rl[i] + fs->im[i]*fs->im[i]) * 0.35; }
}