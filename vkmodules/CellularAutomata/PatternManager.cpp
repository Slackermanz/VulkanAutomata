#include "PatternManager.h"
#include <iostream>
#include <cstring>
#include <cmath>
#include <limits>

template<typename T>
static int count_binary_records(const std::string& loadfile, const std::string& label) {
	std::ifstream file(loadfile.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
	if(!file.is_open()) {
		ov("Missing " + label + " file", loadfile);
		return 0;
	}

	std::streampos end_pos = file.tellg();
	if(end_pos == std::streampos(-1) || end_pos <= std::streampos(0)) {
		ov("Empty " + label + " file", loadfile);
		return 0;
	}

	std::streamoff byte_count = static_cast<std::streamoff>(end_pos);
	std::streamoff record_size = static_cast<std::streamoff>(sizeof(T));
	std::streamoff record_count = byte_count / record_size;
	std::streamoff trailing_bytes = byte_count % record_size;

	if(trailing_bytes != 0) {
		ov("Partial " + label + " record bytes ignored", trailing_bytes);
		ov("Partial " + label + " file", loadfile);
	}

	if(record_count <= 0) {
		ov("No complete " + label + " records", loadfile);
		return 0;
	}

	if(record_count > std::numeric_limits<int>::max()) {
		ov("Too many " + label + " records", loadfile);
		return std::numeric_limits<int>::max();
	}

	return static_cast<int>(record_count);
}

static int normalize_record_index(int idx, int count) {
	if(count <= 0) { return 0; }
	int normalized = idx % count;
	return (normalized < 0) ? normalized + count : normalized;
}

template<typename T>
static bool read_binary_record(const std::string& loadfile, int idx, T* record, int* count_out, const std::string& label) {
	if(record == NULL) { return false; }

	*record = T{};
	int count = count_binary_records<T>(loadfile, label);
	if(count_out != NULL) { *count_out = count; }
	if(count <= 0) { return false; }

	int normalized = normalize_record_index(idx, count);
	std::ifstream file(loadfile.c_str(), std::ios::in | std::ios::binary);
	if(!file.is_open()) {
		ov("Failed reopening " + label + " file", loadfile);
		return false;
	}

	std::streamoff offset = static_cast<std::streamoff>(normalized) * static_cast<std::streamoff>(sizeof(T));
	file.seekg(offset, std::ios::beg);
	if(!file.good()) {
		ov("Failed seeking " + label + " record", normalized);
		ov("Failed seeking " + label + " file", loadfile);
		return false;
	}

	file.read(reinterpret_cast<char*>(record), sizeof(T));
	if(file.gcount() != static_cast<std::streamsize>(sizeof(T))) {
		ov("Failed reading complete " + label + " record", normalized);
		ov("Failed reading " + label + " file", loadfile);
		*record = T{};
		return false;
	}

	return true;
}

int get_PCD408_count(std::string loadfile) {
	return count_binary_records<PatternConfigData_408>(loadfile, "PCD408");
}

PatternConfigData_408 get_PCD_408(std::string loadfile, int idx, EngineInfo *ei) {
	PatternConfigData_408 pcd{};
	int count = 0;
	bool loaded = read_binary_record<PatternConfigData_408>(loadfile, idx, &pcd, &count, "PCD408");
	if(ei != NULL) { ei->PCD_count = count; }
	std::cout << "\n\tPCD408 Data Count: " << (loaded ? normalize_record_index(idx, count) : idx) << " / " << count << "\n";
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

bool save_PCD256(std::string savefile, UB32_64 *pcd) {
	if(pcd == NULL) {
		ov("Failed saving PCD256", "NULL pattern");
		return false;
	}

	std::ofstream file(savefile.c_str(), std::ios::out | std::ios::binary | std::ios::app);
	if(!file.is_open()) {
		ov("Failed opening PCD256 save file", savefile);
		return false;
	}

	file.write(reinterpret_cast<const char*>(pcd), sizeof(UB32_64));
	if(!file.good()) {
		ov("Failed writing PCD256 save file", savefile);
		return false;
	}

	std::cout << "\nSAVE:" << show_PCD256(pcd);
	return true;
}

int get_PCD256_count(std::string loadfile) {
	return count_binary_records<UB32_64>(loadfile, "PCD256");
}

UB32_64 load_PCD256(std::string loadfile, int idx) {
	UB32_64 pcd = new_PCD_256();
	int count = 0;
	bool loaded = read_binary_record<UB32_64>(loadfile, idx, &pcd, &count, "PCD256");
	std::cout << "\tLoad PCD256 Data Count: " << (loaded ? normalize_record_index(idx, count) : idx) << " / " << count << "\n";
	if(loaded) { std::cout << "\nLOAD:" << show_PCD256(&pcd); }
	return pcd;
}

WAVS16_1024 load_WAVS16(std::string loadfile, int idx) {
	WAVS16_1024 WAVS16{};
	std::ifstream file(loadfile.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
	if(!file.is_open()) {
		ov("Missing WAVS16 file", loadfile);
		return WAVS16;
	}

	std::streampos end_pos = file.tellg();
	if(end_pos == std::streampos(-1) || end_pos <= std::streampos(0)) {
		ov("Empty WAVS16 file", loadfile);
		return WAVS16;
	}

	std::streamoff byte_count = static_cast<std::streamoff>(end_pos);
	int sample_count = static_cast<int>(byte_count / static_cast<std::streamoff>(sizeof(int16_t)));
	if(sample_count <= 0) {
		ov("No WAVS16 samples", loadfile);
		return WAVS16;
	}

	int normalized = normalize_record_index(idx, sample_count);
	file.seekg(static_cast<std::streamoff>(normalized) * static_cast<std::streamoff>(sizeof(int16_t)), std::ios::beg);
	if(!file.good()) {
		ov("Failed seeking WAVS16 sample", normalized);
		return WAVS16;
	}

	file.read(reinterpret_cast<char*>(&WAVS16), sizeof(WAVS16_1024));
	if(file.gcount() != static_cast<std::streamsize>(sizeof(WAVS16_1024))) {
		ov("Short WAVS16 read bytes", file.gcount());
	}

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
uint32_t u32_flp(uint32_t u32, uint32_t off) {
	return (off < 32u) ? (u32 ^ (1u << off)) : u32;
}

uint32_t u32_set(uint32_t u32, uint32_t off) {
	return (off < 32u) ? (u32 | (1u << off)) : u32;
}

uint32_t u32_clr(uint32_t u32, uint32_t off) {
	return (off < 32u) ? (u32 & ~(1u << off)) : u32;
}

uint32_t mut_rnd() {
	uint32_t out = 0u;
	for(uint32_t bits = 0u; bits < 32u; bits += 15u) {
		out |= (static_cast<uint32_t>(rand()) & 0x7fffu) << bits;
	}
	return out;
}

uint32_t mutation_period_from_strength(int strength) {
	uint32_t s = (strength < 1) ? 1u : static_cast<uint32_t>(strength);
	if(s > UINT32_MAX / 2u) { s = UINT32_MAX / 2u; }
	return (mut_rnd() % (s * 2u)) + (s / 2u) + 1u;
}

uint32_t blk_clr(uint32_t u32) {
	return 0u;
}

uint32_t blk_set(uint32_t u32) {
	return UINT32_MAX;
}

static uint32_t wrd_end(uint32_t off, uint32_t len) {
	if(off >= 32u || len == 0u) { return off; }
	uint32_t max_len = 32u - off;
	return off + ((len < max_len) ? len : max_len);
}

uint32_t wrd_clr(uint32_t u32, uint32_t off, uint32_t len) {
	uint32_t end = wrd_end(off, len);
	for(uint32_t i = off; i < end; i++) { u32 = u32_clr(u32, i); }
	return u32;
}

uint32_t wrd_set(uint32_t u32, uint32_t off, uint32_t len) {
	uint32_t end = wrd_end(off, len);
	for(uint32_t i = off; i < end; i++) { u32 = u32_set(u32, i); }
	return u32;
}

uint32_t wrd_flp(uint32_t u32, uint32_t off, uint32_t len) {
	uint32_t end = wrd_end(off, len);
	for(uint32_t i = off; i < end; i++) { u32 = u32_flp(u32, i); }
	return u32;
}

static bool one_in(uint32_t period) {
	return period > 0u && (mut_rnd() % period) == 0u;
}

static uint32_t doubled_period(uint32_t period) {
	return (period > UINT32_MAX / 2u) ? UINT32_MAX : period * 2u;
}

uint32_t bit_flp(uint32_t u32, uint32_t rnd) { 
	if(rnd == 0u) { return u32; }
	for(uint32_t i = 0u; i < 32u; i++) { if(one_in(rnd)) { u32 = u32_flp(u32, i); } }
	uint32_t range_period = doubled_period(rnd);
	if(one_in(range_period)) { u32 = wrd_set( u32, mut_rnd()%32u, mut_rnd()%8u ); }
	if(one_in(range_period)) { u32 = wrd_clr( u32, mut_rnd()%32u, mut_rnd()%8u ); }
	if(one_in(range_period)) { u32 = wrd_flp( u32, mut_rnd()%32u, mut_rnd()%8u ); }
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