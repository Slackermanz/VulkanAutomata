#ifndef VKMODULES_TYPES_H
#define VKMODULES_TYPES_H

struct GLFWwindow;

#include <chrono>
#include <string>
#include <cstdint>

// GLFW Input handling
struct GLFW_key {
	GLFWwindow* window;
	int 		key;
	int 		scancode;
	int 		action;
	int 		mods;
};

struct GLFW_mouse {
	GLFWwindow* window;
	double 		xpos;
	double 		ypos;
	int 		button;
	int 		action;
	int 		mods;
	double 		xoffset;
	double 		yoffset;
	double 		scroll_x_accum;
	double 		scroll_y_accum;
};

struct KeyCapture {
	bool has_mouse;
	bool has_keyboard;
};

// Timing
struct NS_Timer {
	std::chrono::_V2::system_clock::time_point st;
	std::chrono::_V2::system_clock::time_point ft;
};

// Uniform buffer data structures
struct UB32_64 { uint32_t u32[64]; };
struct SB_4096 { UB32_64 ub[16]; };

// Simulation info
struct PatternConfigData_408 {
	uint32_t scd_save[48];
	uint32_t ubi_save[4];
	uint32_t ubv_save[48];
	float	 scl_save;
	float	 pzm_save;
};

// Engine state
struct EngineInfo {
	bool 		paused;
	bool 		show_gui;
	bool 		run_headless;
	uint32_t	imgdat_idx;
	int		 	export_frequency;
	int		 	export_batch_size;
	int		 	export_batch_left;
	int		 	export_batch_last;
	bool 		export_enabled;
	uint32_t 	loglevel;
	uint32_t 	tick_loop;
	int 		load_pattern;
	uint32_t 	PCD_count;
};

// UI and interaction
struct UI_info {
	uint32_t 	mx;
	uint32_t 	my;
	uint32_t 	mbl;
	uint32_t 	mbr;
	uint32_t 	cmd;
};

inline uint32_t pack_ui_info(UI_info ui) {
	uint32_t packed_ui32 =
		( (uint32_t)ui.mx	 	  )
	+ 	( (uint32_t)ui.my 	<< 12 )
	+ 	( (uint32_t)ui.mbl 	<< 24 )
	+ 	( (uint32_t)ui.mbr 	<< 25 )
	+ 	( (uint32_t)ui.cmd 	<< 26 );
	return packed_ui32;
}

struct FT_info {
	uint32_t 	frame;
	uint32_t 	seed;
};

inline uint32_t pack_ft_info(FT_info ft) {
	uint32_t packed_ui32 =
		( (uint32_t)ft.frame	   )
	+ 	( (uint32_t)ft.seed	 << 24 );
	return packed_ui32;
}

struct VW_info {
	uint32_t 	pmap;		// Parameter Map Index
	uint32_t 	sdat;		// Show Data Flag
};

inline uint32_t pack_vw_info(VW_info vw) {
	uint32_t packed_ui32 =
		( (uint32_t)vw.pmap	 	  )
	+ 	( (uint32_t)vw.sdat << 2  );
	return packed_ui32;
}

// IMGUI configuration
struct IMGUI_Config {
	bool 		load_shader;
	bool 		load_pattern;
	bool 		load_pattern_confirm;
	bool 		load_pattern_check_instant;
	bool 		load_pattern_check_reseed;
	int			load_pattern_last_value;
	bool		load_pattern_random;
	bool		save_to_archive;
	bool 		load_A256_confirm;
	int 		load_A256_index;
	int 		load_A256_index_last;
	int 		load_A256_count;
	bool		mutate_menu;
	bool		mutate_full_random;
	bool		mutate_set_target;
	bool		mutate_backstep;
	bool		mutate_backstep_retry;
	int			mutate_backstep_idx;
	int			mutate_backstep_last_value;
	bool		mutate_flip;
	int			mutate_flip_str;
	bool		throttle_menu;
	bool		throttle_enabled;
	int			throttle_target;
	bool		mode_planar;
	bool		mode_linear;
	bool		mode_circular;
	bool		mode_showdata;
	int			pmap_index;
	int			pmap_index_last;
	bool		scale_zoom_menu;
	bool		scale_update;
	float		scale_value;
	float		scale_last_value;
	bool		zoom_update;
	float		zoom_value;
	float		zoom_last_value;
	bool		show_notification;
	int			notification_index;
	NS_Timer	notification_timer;
	float		notification_age;
	NS_Timer	notification_float_timer;
	float		notification_float_age;
	bool		show_notification_float;
	float		notification_float_value;
	int 		glfw_mouse_xpos_last;
	bool		glfw_mod_LCTRL;
	bool		glfw_mod_LSHIFT;
	bool		scale_has_panned;
	bool		record_imgui;
	bool 		recording_config;
};

// Sound/frequency data
struct fspec256 {
	float rl[256];
	float im[256];
};

struct fsmag256 { 
    float fsm[256]; 
};

struct WAVS16_1024 {
	int16_t i16[1024*8];
};

#endif // VKMODULES_TYPES_H
