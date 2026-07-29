#include <GLFW/glfw3.h>

#include "MenuSystem.h"
#include "ActionDispatcher.h"
#include "Notifications.h"     // For notification_list
#include "../Utils/Timer.h"   // For tog function
#include "../../lib/imgui.h"  // Fix include path for imgui.h

void imgui_menu(GLFWwindow *w, UI_info *ui, EngineInfo *ei, IMGUI_Config *gc) {
	
	if( ImGui::BeginMainMenuBar() ) {
		if( ImGui::BeginMenu("Application") ) {
			if( ImGui::MenuItem( (ei->show_gui) ? "Hide GUI" : "Show GUI", "ESC" ) ) 	{ do_action(  0, ui, ei, gc ); }
			ImGui::Separator();
			if( ImGui::MenuItem( "Help", "F1", false, false ) ) 						{ }
			ImGui::Separator();
			if( ImGui::MenuItem( "QUIT", "ALT-F4" ) ) 								{ glfwSetWindowShouldClose(w, 1); }
			ImGui::EndMenu(); }
		ImGui::Separator();

		if( ImGui::BeginMenu("Archive") ) {
		//	if( ImGui::MenuItem( "Reload" ) ) 										{ do_action(  3, ui, ei, gc ); }
		//	ImGui::Separator();
			if( ImGui::MenuItem( "Random Pattern", "TAB" ) )						{ do_action( 39, ui, ei, gc ); }
			if( ImGui::MenuItem( "Next", "MB-Forward" ) ) 							{ do_action( 37, ui, ei, gc ); }
			if( ImGui::MenuItem( "Previous", "MB-Back" ) ) 							{ do_action( 38, ui, ei, gc ); }
			ImGui::Separator();
			if( ImGui::MenuItem( "Load Pattern", "..." ) ) 							{ gc->load_pattern 		= true; }
			ImGui::Separator();
			if( ImGui::MenuItem( "Save to Archive", "CTRL-S" ) ) 					{ do_action(  7, ui, ei, gc ); }
			ImGui::EndMenu(); }
		ImGui::Separator();

		if( ImGui::BeginMenu("Playback") ) {
			if( ImGui::MenuItem( (ei->paused) ? "Resume" : "Pause", "SPACE" ) ) 		{ do_action(  1, ui, ei, gc ); }
			if( ImGui::MenuItem( "Step [1]", "S" ) ) 									{ do_action(  2, ui, ei, gc ); }
			ImGui::Separator();
			if( ImGui::MenuItem( "Throttle", "... | T" ) ) 							{ gc->throttle_menu 	= true; }
			ImGui::Separator();
			if( ImGui::MenuItem( "Recording", "... | Num-ENTER" ) ) 				{ gc->recording_config 	= true; }
			ImGui::EndMenu(); }
		ImGui::Separator();

		if( ImGui::BeginMenu("Modify") ) {
			if( ImGui::MenuItem( "Randomize", "R" ) ) 								{ do_action(  8, ui, ei, gc ); }
			ImGui::Separator();
			if( ImGui::MenuItem( "Mutate Target", "V" ) ) 							{ do_action( 10, ui, ei, gc ); }
			if( ImGui::MenuItem( "Update Target", "MB-Mid" ) ) 						{ do_action(  9, ui, ei, gc ); }
			if( ImGui::MenuItem( "Reload Target", "Q" ) ) 							{ do_action( 24, ui, ei, gc ); }
			ImGui::Separator();
			if( ImGui::MenuItem( "Mutation Controls", "..." ) ) 					{ gc->mutate_menu 		= true; }
			ImGui::Separator();
			if( ImGui::MenuItem( "Scale & Zoom", "..." ) ) 							{ gc->scale_zoom_menu 	= true; }
			ImGui::Separator();
			if( ImGui::MenuItem( "Shader Selection", "...", false, false ) ) 		{ gc->load_shader 		= true; }
			ImGui::EndMenu(); }
		ImGui::Separator();

		if( ImGui::BeginMenu("Command") ) {
			if( ImGui::MenuItem( "Clear", "X" ) ) 									{ do_action( 12, ui, ei, gc ); }
			if( ImGui::MenuItem( "Reseed: Symmetric", "Z" ) ) 						{ do_action( 13, ui, ei, gc ); }
			if( ImGui::MenuItem( "Reseed: Random", "L-Shift" ) ) 					{ do_action( 11, ui, ei, gc ); }
			if( ImGui::MenuItem( "Reseed: Blend [12]", "C" ) ) 						{ do_action( 34, ui, ei, gc ); }
			ImGui::EndMenu(); }
		ImGui::Separator();

		if( ImGui::BeginMenu("Mode") ) {
			if( ImGui::MenuItem( "Reset", 		NULL, 	false, 				false 	) )	{  }
			ImGui::Separator();
			if( ImGui::MenuItem( "Planar Map", 	 "1", 	&gc->mode_planar 			) )	{ do_action( 14, ui, ei, gc ); }
			if( ImGui::MenuItem( "Linear Map", 	 "2", 	&gc->mode_linear 			) )	{ do_action( 15, ui, ei, gc ); }
			if( ImGui::MenuItem( "Circular Map", "3", 	&gc->mode_circular		 	) )	{ do_action( 16, ui, ei, gc ); }
			if( !(	gc->mode_planar
				||	gc->mode_linear
				||	gc->mode_circular ) ) 												{ do_action( 28, ui, ei, gc ); }
			ImGui::Separator();
			if( ImGui::MenuItem( "Show Data", 	 "0", 	&gc->mode_showdata, false 	) )	{ do_action( 17, ui, ei, gc ); }
			ImGui::EndMenu(); }
		ImGui::Separator();

		ImGui::EndMainMenuBar(); }

//	TODO This will need to be re-imagined later on.
//	if(gc->load_shader) {
//		if(!ImGui::Begin("Select Shader", &gc->load_shader, 
//			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize )) {
//			ImGui::End(); } 
//		else {
//			if(ImGui::Button( "Select File", 	ImVec2(112.0,0.0) )) { }
//			if(ImGui::Button( "Compile", 		ImVec2(112.0,0.0) )) { }
//			if(ImGui::Button( "Load SPIR-V", 	ImVec2(112.0,0.0) )) { }
//			ImGui::End(); } }

	  /////////////////////////////////////
	 // 	Notifications				//
	/////////////////////////////////////

	if(gc->show_notification) {
		ImGui::SetNextWindowPos(ImVec2(4,28), ImGuiCond_Always);
		float 	color_flash = gc->notification_age * gc->notification_age;
				color_flash = (color_flash >= 0.5f) ? color_flash - 0.5f : 0.0f;
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(color_flash, color_flash * 0.8f, color_flash * 0.6f, gc->notification_age));
		if(!ImGui::Begin( "Notification", &gc->show_notification,
				ImGuiWindowFlags_NoDecoration
			| 	ImGuiWindowFlags_AlwaysAutoResize 
			|	ImGuiWindowFlags_NoSavedSettings
			| 	ImGuiWindowFlags_NoFocusOnAppearing
			| 	ImGuiWindowFlags_NoNav
			|	ImGuiWindowFlags_NoMove )) {
			ImGui::End(); } 
		else {
        	ImGui::Text("%s", notification_list[gc->notification_index] );
			ImGui::End(); }
		ImGui::PopStyleColor(); }

	if(gc->show_notification_float) {
		int txtoffset = strlen(notification_list[gc->notification_index]);
			txtoffset = (txtoffset * 8) + ((txtoffset <= 1) ? 0 :20);
		ImGui::SetNextWindowPos(ImVec2(4 + txtoffset, 28 ), ImGuiCond_Always);
		float 	color_flash = gc->notification_float_age * gc->notification_float_age;
				color_flash = (color_flash >= 0.5f) ? color_flash - 0.5f : 0.0f;
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(color_flash, color_flash * 0.8f, color_flash * 0.6f, gc->notification_float_age));
		if(!ImGui::Begin( "NotificationFloat", &gc->show_notification_float,
				ImGuiWindowFlags_NoDecoration
			| 	ImGuiWindowFlags_AlwaysAutoResize 
			|	ImGuiWindowFlags_NoSavedSettings
			| 	ImGuiWindowFlags_NoFocusOnAppearing
			| 	ImGuiWindowFlags_NoNav
			|	ImGuiWindowFlags_NoMove )) {
			ImGui::End(); } 
		else {
        	ImGui::Text("%f", gc->notification_float_value );
			ImGui::End(); }
		ImGui::PopStyleColor(); }

	  /////////////////////////////////////
	 // 	Menus					//
	/////////////////////////////////////

	if(gc->scale_zoom_menu) {
		if(!ImGui::Begin("Scale & Zoom", &gc->scale_zoom_menu, 
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize )) {
			ImGui::End(); } 
		else {
			ImGui::SetNextItemWidth(256);
            ImGui::SliderFloat("S0", 	&gc->scale_value, 	0.0f, 	256.0f,		"%.4f"	);
		    ImGui::SameLine();
			ImGui::SetNextItemWidth(0);
			ImGui::InputFloat ("Scale", &gc->scale_value, 	0.01f, 	1024.0f, 	""		);
			ImGui::SetNextItemWidth(256);
            ImGui::SliderFloat("S1", 	&gc->scale_value, 	256.0f, 1024.0f,	"%.4f"	);
		    ImGui::SameLine();
        	ImGui::Text( "[Shift+MBR]" );
			ImGui::Separator();
			ImGui::SetNextItemWidth(256);
            ImGui::SliderFloat("Z0", 	&gc->zoom_value, 	0.0f, 	32.0f, 		"%.4f", ImGuiSliderFlags_Logarithmic );
		    ImGui::SameLine();
			ImGui::SetNextItemWidth(0);
			ImGui::InputFloat ("Zoom", 	&gc->zoom_value, 	0.01f, 	32.0f, 		""		);
			if( check_input_update(&gc->scale_value, &gc->scale_last_value) ) 						{ do_action( 22, ui, ei, gc ); }
			if( check_input_update(&gc->zoom_value,  &gc->zoom_last_value ) ) 						{ do_action( 23, ui, ei, gc ); }
			ImGui::End(); } }

	if(gc->throttle_menu) {
		if(!ImGui::Begin("FPS Throttle", &gc->throttle_menu, 
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize )) {
			ImGui::End(); } 
		else {
			ImGui::SetNextItemWidth(96);
			ImGui::InputInt( "ms", 		&gc->throttle_target	);
			if(ImGui::Button( (gc->throttle_enabled) ? "Disable" : "Enable", ImVec2(112.0,0.0) )) { do_action( 18, ui, ei, gc ); }
			ImGui::End(); } }

	if(gc->mutate_menu) {
		if(!ImGui::Begin("Mutate", 		&gc->mutate_menu, 
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize )) {
			ImGui::End(); } 
		else {
//			const char* tpat = (gc->mutate_backstep_idx == -1) ? "Current" : "Modified";
//        	ImGui::Text( "Pattern Targeting: ( %s )", tpat );
			if(ImGui::Button( "Reload Target", ImVec2(112.0,0.0) )) 								{ do_action( 24, ui, ei, gc ); }
		    ImGui::SameLine();
			if(ImGui::Button( "Update Target", ImVec2(112.0,0.0) )) 								{ do_action(  9, ui, ei, gc ); }
			ImGui::SetNextItemWidth(112);
			ImGui::InputInt("Target History", &gc->mutate_backstep_idx);
			ImGui::Separator();
			if(ImGui::Button( "Mutate Target", ImVec2(112.0,0.0) )) 								{ do_action( 10, ui, ei, gc ); }
		    ImGui::SameLine();
			if(ImGui::Button( "Fully Randomize", ImVec2(112.0,0.0) )) 								{ do_action(  8, ui, ei, gc ); }
			ImGui::SetNextItemWidth(112);
			ImGui::InputInt("Bit Flip Chance",  &gc->mutate_flip_str);
//			ImGui::Separator();
//        	ImGui::Text( "Note:\n Import Confirms Scale/Zoom\n Update Discards Scale/Zoom" );
			ImGui::Separator();
			if(ImGui::Button( "Scale & Zoom", ImVec2(112.0,0.0) )) 									{ tog(&gc->scale_zoom_menu); }
		    ImGui::SameLine();
            const char* pmap_names[] = { "Planar Map", "Linear Map", "Circular Map" };
			ImGui::SetNextItemWidth(112);
            ImGui::Combo("", &gc->pmap_index, pmap_names, IM_ARRAYSIZE(pmap_names));
        	ImGui::Text( "Scale: %f", gc->scale_value );
        	ImGui::Text( "Zoom:  %f", gc->zoom_value  );
			if( check_input_update(&gc->mutate_backstep_idx, &gc->mutate_backstep_last_value) ) { do_action( 24, ui, ei, gc ); }
			if( check_input_update(&gc->pmap_index, &gc->pmap_index_last) ) 						{ do_action( 29, ui, ei, gc ); }
			ImGui::End(); } }

	if(gc->recording_config) {
		if(!ImGui::Begin("Recording Controls", &gc->recording_config, 
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize )) {
			ImGui::End(); } 
		else {
			if(ImGui::Button( (ei->export_enabled) 	? "Stop" 	: "Start", ImVec2(96.0,0.0) )) 	{ do_action( 20, ui, ei, gc ); }
		    ImGui::SameLine();
			if(ImGui::Button( (ei->paused) 			? "Resume" 	: "Pause", ImVec2(96.0,0.0) )) 	{ do_action(  1, ui, ei, gc ); }
			ImGui::Separator();
			ImGui::SetNextItemWidth(112);
			ImGui::InputInt( "Frequency",	&ei->export_frequency	);
			ImGui::SetNextItemWidth(112);
			ImGui::InputInt( "Batch",		&ei->export_batch_size	);
		    ImGui::SameLine();
        	ImGui::Text("/ %d", 	 	 	 ei->export_batch_left 	);
			ImGui::Checkbox("Record Interface", 	&gc->record_imgui);
			ImGui::Separator();
			if(ImGui::Button( "Reset", ImVec2(64.0,0.0) )) 								{ do_action( 21, ui, ei, gc ); do_action( 25, ui, ei, gc ); }
		    ImGui::SameLine();
        	ImGui::Text("IMG%d.PAM", 		 ei->imgdat_idx 		);
			if( check_input_update(&ei->export_batch_size, &ei->export_batch_last) ) 			{ do_action( 25, ui, ei, gc ); }
			ImGui::End(); } }

	if(gc->load_pattern) {
		if(!ImGui::Begin("Load Pattern", &gc->load_pattern, 
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize )) {
			ImGui::End(); } 
		else {
			ImGui::SetNextItemWidth(112);
			ImGui::InputInt("", &gc->load_A256_index);
		    ImGui::SameLine();
			if(ImGui::Button( "Load", ImVec2(64.0,0.0) )) 											{ do_action( 40, ui, ei, gc ); }
		    ImGui::SameLine();
			if(ImGui::Button( "Random", ImVec2(64.0,0.0) ))											{ do_action( 39, ui, ei, gc ); }
			ImGui::Checkbox("Instant", 	&gc->load_pattern_check_instant);
		    ImGui::SameLine();
			ImGui::Checkbox("Reseed", 	&gc->load_pattern_check_reseed);
			if(	gc->load_pattern_check_instant
			&&	check_input_update(&gc->load_A256_index, &gc->load_A256_index_last) ) 			{ do_action( 40, ui, ei, gc ); }
			ImGui::End(); } }
}