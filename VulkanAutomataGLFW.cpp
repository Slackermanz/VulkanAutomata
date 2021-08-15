#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "lib/imgui.h"
#include "lib/imgui_impl_vulkan.h"
#include "lib/imgui_impl_glfw.h"
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <cstring>

const 	uint32_t 	VERT_FLS 	=  1;	//	Number of Vertex Shader Files
const 	uint32_t 	FRAG_FLS 	=  1;	//	Number of Fragment Shader Files
const	int 		MAXLOG 		=  2;
		int 		loglevel	=  MAXLOG;
		int 		valid 		=  1;

//	Keyboard input handler
struct GLFW_key {
	GLFWwindow* window;
	int 		key;
	int 		scancode;
	int 		action;
	int 		mods; };
GLFW_key 	glfw_key;
void clear_glfw_key(GLFW_key *e) {
	e->key 		= 0;
	e->scancode = 0;
	e->action 	= 0;
	e->mods 	= 0; }

//	Mouse input handler
struct GLFW_mouse {
	GLFWwindow* window;
	double 		xpos;
	double 		ypos;
	int 		button;
	int 		action;
	int 		mods;
	double 		xoffset;
	double 		yoffset; };
GLFW_mouse 	glfw_mouse;
void clear_glfw_mouse(GLFW_mouse *e) {
	e->mods 	= 0;
	e->xoffset 	= 0;
	e->yoffset 	= 0; }

struct KeyCapture {
	bool has_mouse;
	bool has_keyboard; };
KeyCapture kc;

static VKAPI_ATTR VkBool32 VKAPI_CALL
//	Vulkan validation layer message output
	debugCallback(			VkDebugUtilsMessageSeverityFlagBitsEXT	messageSeverity, 
							VkDebugUtilsMessageTypeFlagsEXT			messageType, 
					const 	VkDebugUtilsMessengerCallbackDataEXT* 	pCallbackData, 
					void* 											pUserData			) {
	std::string bar = "";
	for(int i = 0; i < 20; i++) { bar = bar + "####"; }

	std::string msg 	= pCallbackData->pMessage;
	std::string msg_fmt = "";
	for(int i = 0; i < msg.size()-1; i++) {
		char chr = msg[i];
		char chrhtml = msg[i+1];
		msg_fmt = ( 
			chr == ':' && chrhtml != '/' ?
				msg_fmt+":\n\n" : ( chr == '|' ?
					msg_fmt+"\n" : msg_fmt+chr ) ); }

	msg_fmt = msg_fmt + msg[msg.size()-1];
	if(messageSeverity != 16) {
		loglevel = MAXLOG;
		if(loglevel >= 0) {	std::cout << "\n\n" << bar << "\n " << msg_fmt << "\n" 	<< bar << "\n\n"; }
		std::cout << "  Validation messageSeverity: " << messageSeverity << "\n\n";
		valid = 0; }
	return VK_FALSE; }

// Find a memory in `memoryTypeBitsRequirement` that includes all of `requiredProperties`
int32_t findProperties(
	const 		VkPhysicalDeviceMemoryProperties* 	pMemoryProperties,
				uint32_t 							memoryTypeBitsRequirement,
				VkMemoryPropertyFlags 				requiredProperties 			) {
    const 		uint32_t 	memoryCount = pMemoryProperties->memoryTypeCount;
    for ( uint32_t memoryIndex = 0; memoryIndex < memoryCount; ++memoryIndex ) {
        const 	uint32_t 				memoryTypeBits 			= (1 << memoryIndex);
        const 	bool 					isRequiredMemoryType 	= memoryTypeBitsRequirement & memoryTypeBits;
        const 	VkMemoryPropertyFlags 	properties 				= pMemoryProperties->memoryTypes[memoryIndex].propertyFlags;
        const 	bool 					hasRequiredProperties 	= (properties & requiredProperties) == requiredProperties;
        if (isRequiredMemoryType && hasRequiredProperties) { return static_cast<int32_t>(memoryIndex); } } return -1; }

void hd(const std::string& id, const std::string& msg) {
//	Header output message
	std::string bar = "";
	for(int i = 0; i < 20; i++) { bar = bar + "____"; }
	if(loglevel >= 0) {
		std::cout << bar << "\n " << id << "\t" << msg << "\n"; } }

void ov(const std::string& id, auto v) {
//	Single info output message
	int 		padlen	=  4;
	int 		pads	= 11;
	std::string pad 	= " ";
	int 		padsize = (pads*padlen - id.size()) - 3;
	for(int i = 0; i < padsize; i++) { pad = pad + "."; }
	if(loglevel >= 1) {
		std::cout << "\tinfo:\t    " << id << pad << " [" << v << "]\n"; } }

void iv(const std::string& id, auto ov, int idx) {
//	Multiple info output message
	int 		padlen 	= 4;
	int 		pads 	= 10;
	std::string pad 	= " ";
	int 		padsize = (pads*padlen - id.size()) - 3;
	for(int i = 0; i < padsize; i++) { pad = pad + "."; }
	if(loglevel >= 1) {
		std::cout << "\tinfo:\t" << idx << "\t" << id << pad << " [" << ov << "]\n"; } }

void vr(const std::string& id, std::vector<VkResult>* reslist, auto v, VkResult res) {
//	VkResult output message
	reslist->push_back(res); 	// TODO slow memory leak
	uint32_t 	idx 		= reslist->size() - 1;
	std::string	idx_string 	= std::to_string(idx);
	uint32_t 	idx_sz		= idx_string.size();
	std::string res_string 	= std::to_string(res);
	if(idx_sz < 4) { for(int i = 0; i < 4-idx_sz; i++) { idx_string = " " + idx_string; } }
	int 		padlen	= 4;
	int 		pads	= 12;
	std::string pad 	= " ";
	int 		padsize = (pads*padlen - id.size()) - 3;
	for(int i = 0; i < padsize; i++) { pad = pad + " "; }
	if(loglevel >= 2) {
		std::cout << "  " << idx_string << ":\t" << (res==0?" ":res_string) << " \t" << id << pad << " [" << v << "]\n"; } }

void rv(const std::string& id) {
//	Return void output message
	if(loglevel >= 2) {
		std::cout << "  void: \t" << id	<< "\n"; } }

void nf(auto *Vk_obj) {
//	NullFlags shorthand
	Vk_obj->pNext = NULL;
	Vk_obj->flags = 0; }

struct VK_Obj {
	VkInstance			VKI;
	uint32_t			VKP_i;
	VkPhysicalDevice	VKP;
	uint32_t			VKQ_i;
	VkDevice			VKL;
};

struct VK_Config {
	VkApplicationInfo 		app_info;
	VkInstanceCreateInfo	inst_info;
};

struct VK_Debug {
	VkDebugUtilsMessengerCreateInfoEXT	debug_msg_info;
	VkDebugUtilsMessengerEXT			vk_debug_utils_messenger_ext;
};

struct VK_PhysDev {
	uint32_t 							pd_count;
	VkPhysicalDevice					vk_pdev;
	VkPhysicalDeviceProperties			vk_pdev_props;
	VkPhysicalDeviceFeatures			vk_pdev_feats;
	VkPhysicalDeviceMemoryProperties	vk_pdev_mem_props;
};

struct VK_PDQueues {
	VkDeviceQueueCreateInfo		pdq_info; 
};

struct VK_LogDev {
	VkDeviceCreateInfo			ldev_info;
};

struct VK_Layer_1x2D {
	VkExtent3D				ext3D;
	VkImageCreateInfo		img_info;
	VkImage					vk_image;
	uint32_t				MTB_index;
	VkMemoryRequirements	vk_mem_reqs;
	VkMemoryAllocateInfo	vk_mem_allo_info;
	VkDeviceMemory			vk_dev_mem;
};

struct VK_Layer_2x2D {
	VkExtent3D				ext3D[2];
	VkImageCreateInfo		img_info[2];
	VkImage					vk_image[2];
	uint32_t				MTB_index[2];
	VkMemoryRequirements	vk_mem_reqs[2];
	VkMemoryAllocateInfo	vk_mem_allo_info[2];
	VkDeviceMemory			vk_dev_mem[2];
};

struct VK_Command {
	VkCommandPoolCreateInfo			pool_info;
	VkCommandPool					vk_command_pool;
	VkCommandBufferAllocateInfo		comm_buff_alloc_info;
	VkCommandBuffer					vk_command_buffer;
	VkCommandBufferBeginInfo		comm_buff_begin_info;
};

struct ShaderCodeInfo {
	std::string 		shaderFilename;
	std::vector<char>	shaderData;
	size_t 				shaderBytes;
	bool 				shaderBytesValid; };

ShaderCodeInfo getShaderCodeInfo(const std::string& filename) {
	std::ifstream 		file		(filename, std::ios::ate | std::ios::binary);
	size_t 				fileSize = 	(size_t) file.tellg();
	std::vector<char> 	buffer		(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	ShaderCodeInfo sc_info;
		sc_info.shaderFilename		= filename;
		sc_info.shaderData			= buffer;
		sc_info.shaderBytes			= buffer.size();
		sc_info.shaderBytesValid	= (sc_info.shaderBytes%4==0?1:0);
	return sc_info; }

struct ShaderData {
	ShaderCodeInfo 				SC_info;
	VkShaderModuleCreateInfo	vk_SM_info;
	VkShaderModule				vk_shader_module;
	VkShaderStageFlagBits		stage_bits;
};

struct VK_RPConfig {
	VkRect2D									rect2D;
	VkClearValue								clear_val;
	VkImageSubresourceRange						img_subres_range;
	VkImageSubresourceLayers					img_subres_layer;
	VkImageBlit									img_blit;
	VkBufferImageCopy							buffer_img_cpy;
	VkViewport									vk_viewport;
	VkSamplerCreateInfo							samp_info;
	VkSampler									vk_sampler;
};

struct VK_PipeInfo {
	VkPipelineRasterizationStateCreateInfo		p_rast_info;
	VkPipelineViewportStateCreateInfo			p_vprt_info;
	VkPipelineMultisampleStateCreateInfo		p_msam_info;
	VkPipelineVertexInputStateCreateInfo		p_vtin_info;
	VkPipelineInputAssemblyStateCreateInfo		p_inas_info;
	VkPipelineColorBlendAttachmentState			p_cbat_info;
	VkPipelineColorBlendStateCreateInfo			p_cbst_info;
	VkPipelineShaderStageCreateInfo				p_shad_info[VERT_FLS+FRAG_FLS];
};

struct VK_ImageView {
	VkImageViewCreateInfo	img_view_info;
	VkImageView				vk_image_view;
	VkImageMemoryBarrier	img_mem_barr;
};

struct VK_QueueSync {
	VkQueue				vk_queue;
	VkSubmitInfo		sub_info;
	VkFenceCreateInfo	fence_info;
	VkFence				vk_fence;
};

struct VK_DescSetLayout {
	VkDescriptorSetLayoutBinding		set_bind[2];
	VkDescriptorSetLayoutCreateInfo		set_info;
	VkDescriptorPoolSize				pool_size[2];
	VkDescriptorPoolCreateInfo			pool_info;
	VkDescriptorSetLayout				vk_desc_set_layout;
	VkDescriptorPool					vk_desc_pool;
	VkDescriptorSetAllocateInfo			allo_info;
	VkDescriptorSet						vk_descriptor_set[2];
};

struct VK_RenderPass {
	VkAttachmentDescription		attach_desc;
	VkAttachmentReference		attach_ref;
	VkSubpassDescription		subpass_desc;
	VkRenderPassCreateInfo		rp_info;
	VkRenderPass				vk_render_pass;
};

struct VK_FrameBuff {
	VkFramebufferCreateInfo		fb_info;
	VkFramebuffer				vk_framebuffer;
};

struct VK_Pipe {
	VkPipelineLayoutCreateInfo		layout_info;
	VkPipelineLayout				vk_pipeline_layout;
	VkGraphicsPipelineCreateInfo	gfx_pipe_info;
	VkPipeline						vk_pipeline;
};

struct UniBuf {
	uint32_t v0;  uint32_t v1;  uint32_t v2;  uint32_t v3;	uint32_t v4;  uint32_t v5;  uint32_t v6;  uint32_t v7;
	uint32_t v8;  uint32_t v9;  uint32_t v10; uint32_t v11;	uint32_t v12; uint32_t v13; uint32_t v14; uint32_t v15;
	uint32_t v16; uint32_t v17; uint32_t v18; uint32_t v19;	uint32_t v20; uint32_t v21; uint32_t v22; uint32_t v23;
	uint32_t v24; uint32_t v25; uint32_t v26; uint32_t v27;	uint32_t v28; uint32_t v29; uint32_t v30; uint32_t v31;
	uint32_t v32; uint32_t v33; uint32_t v34; uint32_t v35;	uint32_t v36; uint32_t v37; uint32_t v38; uint32_t v39;
	uint32_t v40; uint32_t v41; uint32_t v42; uint32_t v43;	uint32_t v44; uint32_t v45; uint32_t v46; uint32_t v47;
	uint32_t v48; uint32_t v49; uint32_t v50; uint32_t v51;	uint32_t v52; uint32_t v53; uint32_t v54; uint32_t v55;
	uint32_t v56; uint32_t v57; uint32_t v58; uint32_t v59;	uint32_t v60; uint32_t v61; uint32_t v62; uint32_t v63; };

void save_image(void* image_data, std::string fname, uint32_t w, uint32_t h) {
	fname = "out/" + fname + ".PAM";
	ov("Save Image", fname);
	std::ofstream file(fname.c_str(), std::ios::out | std::ios::binary);
		file 	<<	"P7" 							<< "\n"
			 	<< 	"WIDTH "	<< w 				<< "\n"
			 	<< 	"HEIGHT "	<< h				<< "\n"
			 	<< 	"DEPTH "	<< "4"				<< "\n"
			 	<< 	"MAXVAL "	<< "255"			<< "\n"
			 	<< 	"TUPLTYPE "	<< "RGB_ALPHA"		<< "\n"
			 	<< 	"ENDHDR"	<< "\n";
		file.write( (const char*)image_data, w*h*4 );
	file.close(); }

struct NS_Timer {
	std::chrono::_V2::system_clock::time_point st;
	std::chrono::_V2::system_clock::time_point ft;
};

NS_Timer start_timer(NS_Timer t) {
	t.st = std::chrono::high_resolution_clock::now();
	return t; }

void end_timer(NS_Timer t, std::string msg) {
	t.ft = std::chrono::high_resolution_clock::now();
	std::string ftime 	= std::to_string(
		std::chrono::duration_cast<std::chrono::nanoseconds>(t.ft-t.st).count()) + " ns, " +
		std::to_string( int(1000000000.0 / std::chrono::duration_cast<std::chrono::nanoseconds>(t.ft-t.st).count()) ) + " FPS";
	ov(msg, ftime); }

struct PatternConfigData_408 {
	uint32_t scd_save[48];
	uint32_t ubi_save[4];
	uint32_t ubv_save[48];
	float	 scl_save;
	float	 pzm_save; };

struct EngineInfo {
	uint32_t 	paused;
	uint32_t 	show_gui;
	uint32_t 	run_headless;
	int		 	export_frequency;
	int		 	export_batch_size;
	uint32_t 	export_enabled;
	uint32_t 	verbose_loops;
	uint32_t 	loglevel;
	uint32_t 	tick_loop;
	int 		load_pattern;
	uint32_t 	PCD_count; };

PatternConfigData_408 get_PCD_408(std::string loadfile, int idx, EngineInfo *ei) {
	PatternConfigData_408 pcd;
	std::ifstream fload_pcd(loadfile.c_str(), std::ios::in | std::ios::binary);
		fload_pcd.seekg(0, fload_pcd.end);
		int f_len = fload_pcd.tellg();
		fload_pcd.seekg (( (idx + (f_len / sizeof(pcd))) % (f_len / sizeof(pcd))) * sizeof(pcd));
		fload_pcd.read((char*)&pcd, sizeof(pcd));
		std::cout << "\n\tPCD408 Data Count: " << (f_len / sizeof(pcd)) << "\n"; // Report how many patterns are in data file
		ei->PCD_count = (f_len / sizeof(pcd));
	fload_pcd.close();
	return pcd; }

struct PatternConfigData_256 {
	uint32_t ubi[64]; };

PatternConfigData_256 new_PCD_256() {
	PatternConfigData_256 pcd;
		for(int i = 0; i < 64; i++) { pcd.ubi[i] = 0; }
	return pcd; }

struct UI_info {
	uint32_t 	mx;
	uint32_t 	my;
	uint32_t 	mbl;
	uint32_t 	mbr;
	uint32_t 	cmd; };
uint32_t pack_ui_info(UI_info ui) {
	uint32_t packed_ui32 =
		( (uint32_t)ui.mx	 	  )
	+ 	( (uint32_t)ui.my 	<< 12 )
	+ 	( (uint32_t)ui.mbl 	<< 24 )
	+ 	( (uint32_t)ui.mbr 	<< 25 )
	+ 	( (uint32_t)ui.cmd 	<< 26 );
	return packed_ui32; }

struct IMGUI_Config {
	bool 		load_pattern_input;
	bool 		load_pattern_input_confirm;
	bool 		load_pattern_input_check_instant;
	bool 		load_pattern_input_check_reseed;
	int			load_pattern_input_last_value;
	bool 		recording_config; };

void imgui_menu(GLFWwindow *w, UI_info *ui, EngineInfo *ei, IMGUI_Config *gc) {
	
	if( ImGui::BeginMainMenuBar() ) {

		if( ImGui::BeginMenu("Application") ) {
			if( ImGui::MenuItem( "Hide Menu", "SPACE" ) ) {
				ei->show_gui 	= 0; }
			if( ImGui::MenuItem( "QUIT", "ESC" ) ) {
				glfwSetWindowShouldClose(w, 1);	}
			ImGui::EndMenu(); }

		if( ImGui::BeginMenu("Engine") ) {
			if( ImGui::MenuItem( (ei->paused) ? "Run" : "Pause" ) ) {
				ei->paused 		= (ei->paused) ? 0 : 1;	}
			ImGui::Separator();
			if( ImGui::MenuItem( "Load Pattern" ) ) {
				gc->load_pattern_input = true; }
			if( ImGui::MenuItem( "Recording Controls" ) ) {
				gc->recording_config = true; }
			ImGui::EndMenu(); }

		if( ImGui::BeginMenu("Command") ) {
			if( ImGui::MenuItem( "Reseed", "L-Shift" ) ) {
				ei->tick_loop	= 1;
				ui->cmd 		= 1; }
			if( ImGui::MenuItem( "Clear", "X" ) ) {
				ei->tick_loop	= 1;
				ui->cmd 		= 2; }
			if( ImGui::MenuItem( "Symmetrical Reseed", "Z" ) ) {
				ei->tick_loop	= 1;
				ui->cmd 		= 3; }
			ImGui::EndMenu(); }

		ImGui::EndMainMenuBar(); }

	if(gc->recording_config) {
		if(!ImGui::Begin("Recording Controls", &gc->recording_config, 
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize )) {
			ImGui::End(); } 
		else {
			gc->load_pattern_input_last_value = ei->load_pattern;
			ImGui::SetNextItemWidth(128);
			ImGui::InputInt( "Export Frequency", 	&ei->export_frequency	);
//			ImGui::SetNextItemWidth(128);
//			ImGui::InputInt( "Batch Size",			&ei->export_batch_size	);
			if(ImGui::Button( (ei->export_enabled) ? "Stop" : "Start", ImVec2(64.0,0.0) )) { 
				ei->export_enabled = (ei->export_enabled) ? 0 : 1; }
			ImGui::End(); } }

	if(gc->load_pattern_input) {
		if(!ImGui::Begin("Load Pattern Index", &gc->load_pattern_input, 
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize )) {
			ImGui::End(); } 
		else {
			gc->load_pattern_input_last_value = ei->load_pattern;
			ImGui::SetNextItemWidth(128);
			ImGui::InputInt("", &ei->load_pattern);
		    ImGui::SameLine();
			if(ImGui::Button("Load")) {
				gc->load_pattern_input_confirm = true; }
		    ImGui::SameLine();
			if(ImGui::Button("Random")) {
				ei->load_pattern = (rand()%(ei->PCD_count - 18080)) + 18080;	// Depreicated patterns under 17000
				gc->load_pattern_input_confirm = true; }
			ImGui::Checkbox("Instant Load", &gc->load_pattern_input_check_instant);
		    ImGui::SameLine();
			ImGui::Checkbox("Reseed", &gc->load_pattern_input_check_reseed);
			if(	gc->load_pattern_input_check_instant
			&&	gc->load_pattern_input_last_value != ei->load_pattern ) {
				gc->load_pattern_input_confirm = true; }
			ImGui::End(); } }
}

void glfw_keyboard_event(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if(kc.has_keyboard) {
		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods); }
	else {
		glfw_key.window 	= window;
		glfw_key.key		= key;
		glfw_key.scancode 	= scancode;
		glfw_key.action 	= action;
		glfw_key.mods 		= mods;
/*		loglevel = MAXLOG;
		hd("INPUT:", "KEY");
		ov( "key scancode", scancode);
		ov( "key action", 	action 	);
		ov( "key mods", 	mods 	);
		loglevel = -1;*/ } }

void glfw_mousemove_event(GLFWwindow* window, double xpos, double ypos) {
	glfw_mouse.window 	= window;
	glfw_mouse.xpos		= xpos;
	glfw_mouse.ypos 	= ypos; }

void glfw_mouseclick_event(GLFWwindow* window, int button, int action, int mods) {
	glfw_mouse.window 	= window;
	glfw_mouse.button	= button;
	glfw_mouse.action 	= action;
	glfw_mouse.mods 	= mods;
/*	loglevel = MAXLOG;
	hd("INPUT:", "MOUSE");
	ov( "mouse button", button 	);
	ov( "mouse action", action 	);
	ov( "mouse mods", 	mods 	);
	loglevel = -1;*/ }

void glfw_mousescroll_event(GLFWwindow* window, double xoffset, double yoffset) {
	glfw_mouse.window 	= window;
	glfw_mouse.xoffset	= xoffset;
	glfw_mouse.yoffset 	= yoffset;
/*	loglevel = MAXLOG;
	hd("INPUT:", "MOUSE");
	ov( "mouse xoffset", xoffset );
	ov( "mouse yoffset", yoffset );
	loglevel = -1;*/ }

void update_ub( PatternConfigData_256 *pcd, UniBuf *ub ) {
	ub->v0  = pcd->ubi[ 0]; ub->v1  = pcd->ubi[ 1]; ub->v2  = pcd->ubi[ 2]; ub->v3  = pcd->ubi[ 3];
	ub->v4  = pcd->ubi[ 4]; ub->v5  = pcd->ubi[ 5]; ub->v6  = pcd->ubi[ 6]; ub->v7  = pcd->ubi[ 7];
	ub->v8  = pcd->ubi[ 8]; ub->v9  = pcd->ubi[ 9]; ub->v10 = pcd->ubi[10]; ub->v11 = pcd->ubi[11];
	ub->v12 = pcd->ubi[12]; ub->v13 = pcd->ubi[13]; ub->v14 = pcd->ubi[14]; ub->v15 = pcd->ubi[15];
	ub->v16 = pcd->ubi[16]; ub->v17 = pcd->ubi[17]; ub->v18 = pcd->ubi[18]; ub->v19 = pcd->ubi[19];
	ub->v20 = pcd->ubi[20]; ub->v21 = pcd->ubi[21]; ub->v22 = pcd->ubi[22]; ub->v23 = pcd->ubi[23];
	ub->v24 = pcd->ubi[24]; ub->v25 = pcd->ubi[25]; ub->v26 = pcd->ubi[26]; ub->v27 = pcd->ubi[27];
	ub->v28 = pcd->ubi[28]; ub->v29 = pcd->ubi[29]; ub->v30 = pcd->ubi[30]; ub->v31 = pcd->ubi[31];
	ub->v32 = pcd->ubi[32]; ub->v33 = pcd->ubi[33]; ub->v34 = pcd->ubi[34]; ub->v35 = pcd->ubi[35];
	ub->v36 = pcd->ubi[36]; ub->v37 = pcd->ubi[37]; ub->v38 = pcd->ubi[38]; ub->v39 = pcd->ubi[39];
	ub->v40 = pcd->ubi[40]; ub->v41 = pcd->ubi[41]; ub->v42 = pcd->ubi[42]; ub->v43 = pcd->ubi[43];
	ub->v44 = pcd->ubi[44]; ub->v45 = pcd->ubi[45]; ub->v46 = pcd->ubi[46]; ub->v47 = pcd->ubi[47];
	ub->v48 = pcd->ubi[48]; ub->v49 = pcd->ubi[49]; ub->v50 = pcd->ubi[50]; ub->v51 = pcd->ubi[51];
	ub->v52 = pcd->ubi[52]; ub->v53 = pcd->ubi[53]; ub->v54 = pcd->ubi[54]; ub->v55 = pcd->ubi[55];
	ub->v56 = pcd->ubi[56]; ub->v57 = pcd->ubi[57]; ub->v58 = pcd->ubi[58]; ub->v59 = pcd->ubi[59];
	ub->v60 = pcd->ubi[60]; ub->v61 = pcd->ubi[61]; ub->v62 = pcd->ubi[62]; ub->v63 = pcd->ubi[63]; }

void loadPattern_PCD408_to_256( EngineInfo *ei, PatternConfigData_256 *pcd ) {
	PatternConfigData_408 pcd_load 	= get_PCD_408("res/data/save_global.vkpat", ei->load_pattern, ei);
		for(int i = 0; i < 48; i++) { 
			pcd->ubi[i] =  pcd_load.ubv_save[i]; }
	memcpy(&pcd->ubi[62], &pcd_load.scl_save, sizeof(uint32_t));
	memcpy(&pcd->ubi[61], &pcd_load.pzm_save, sizeof(uint32_t)); }

int main() {

//	Set the random seed
	srand(time(0));

//	Result storage
	std::vector<VkResult> vkres;

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "USER CONFIG");			/**/
	///////////////////////////////////////////////////

//	Some shaders require dimensions to be powers of two (fast wrap-around/torus code)
//		16:9 Resolutions:
//			W:	1920 	1536 	1280	768		512		384		256
//			H:	1080 	864  	720		432		288		216		144
//		2:1 Resolutions:
//			W:	16384 	8192 	4096	2048	1024	512		256
//			H:	8192 	4096  	2048	1024	512		256		128

	const 	uint32_t 	APP_W 	=  1024;	//	Window & Simulation Width
	const 	uint32_t 	APP_H 	=   512;	//	Window & Simulation Height

	EngineInfo ei;
		ei.paused 				= 0;		//	Pause Simulation
		ei.show_gui 			= 1;		//	Render Dear IMGUI
		ei.run_headless 		= 0;		//	Use Headless Mode
		ei.export_frequency 	= 8;		//	Export image file every n frames
		ei.export_batch_size 	= 300;		//	Pause recording after n exported frames
		ei.export_enabled		= 0;
		ei.verbose_loops 		= 16;		//	Use verbose logging for n Main Loop iterations
		ei.load_pattern 		= 18372;	//	Load this index from the pattern archive file
		ei.tick_loop 			= 0;		//	Run the main loop n times, ignoring pause state

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "APPLICATION INIT");		/**/
	///////////////////////////////////////////////////

//	Paths to shader files
	const 	char* 	filepath_vert		[VERT_FLS]
	=	{	"./app/vert_TriQuad.spv" 			};
	const 	char* 	filepath_frag		[FRAG_FLS]
	=	{	"./app/frag_automata0000.spv"		};

//	Config Notification Messages
	ov("Application Width", 	APP_W			);
	ov("Application Height", 	APP_H			);
	ov("ei.load_pattern", 		ei.load_pattern	);
	for(int i = 0; i < VERT_FLS; i++) {	iv("Vertex Shaders", 			filepath_vert[i], 		i ); }
	for(int i = 0; i < FRAG_FLS; i++) {	iv("Fragment Shaders", 			filepath_frag[i], 		i ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "GLFW EXTENSIONS");		/**/
	///////////////////////////////////////////////////

	vr("glfwInit",				&vkres, "GLFW", VkResult( glfwInit() 			) );
	vr("glfwVulkanSupported", 	&vkres, "GLFW", VkResult( glfwVulkanSupported() ) );

	rv("glfwGetRequiredInstanceExtensions");
	uint32_t 		glfw_ext_count 	= UINT32_MAX;
	const char** 	glfw_extentions = glfwGetRequiredInstanceExtensions( &glfw_ext_count );

	for(int i = 0; i < glfw_ext_count; i++) { iv("GLFW Extensions", glfw_extentions[i], i ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "VULKAN EXTENSIONS");		/**/
	///////////////////////////////////////////////////

	uint32_t 	INST_EXS 		= 1 + glfw_ext_count;	//	Number of Vulkan Instance Extensions
	uint32_t 	LAYR_EXS 		= 1;					//	Number of Vulkan Layers
	uint32_t 	LDEV_EXS 		= 1;					//	Number of Vulkan Logical Device Extensions

//	Paths to shader files and extension names
	const 	char* 	instance_extensions	[INST_EXS]
	=	{	"VK_EXT_debug_utils", 
			glfw_extentions[0], 
			glfw_extentions[1]					};	// TODO Number of entries might not be the same for all systems!
	const 	char* 	layer_extensions	[LAYR_EXS]
	=	{	"VK_LAYER_KHRONOS_validation" 		};
	const 	char* 	device_extensions	[LDEV_EXS]
	=	{	VK_KHR_SWAPCHAIN_EXTENSION_NAME		};

//	Extension Notification Messages
	for(int i = 0; i < INST_EXS; i++) {	iv("Instance Extensions", 		instance_extensions[i], i ); }
	for(int i = 0; i < LAYR_EXS; i++) {	iv("Layer Extensions", 			layer_extensions[i], 	i ); }
	for(int i = 0; i < LDEV_EXS; i++) {	iv("Logical Device Extensions", device_extensions[i], 	i ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "VULKAN INIT");			/**/
	///////////////////////////////////////////////////

	vr("init", &vkres, "INIT", VK_ERROR_UNKNOWN);

	VK_Obj vob;

	VK_Config vkcfg;

		vkcfg.app_info.sType						= VK_STRUCTURE_TYPE_APPLICATION_INFO;
		vkcfg.app_info.pNext						= NULL;
		vkcfg.app_info.pApplicationName				= "VulkanAutomataGLFW";
		vkcfg.app_info.applicationVersion			= 0;
		vkcfg.app_info.pEngineName					= NULL;
		vkcfg.app_info.engineVersion				= 0;
		vkcfg.app_info.apiVersion					= VK_API_VERSION_1_2;

		vkcfg.inst_info.sType 						= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	nf(&vkcfg.inst_info);
		vkcfg.inst_info.pApplicationInfo 			= &vkcfg.app_info;
		vkcfg.inst_info.enabledLayerCount 			= LAYR_EXS;
		vkcfg.inst_info.ppEnabledLayerNames 		= layer_extensions;
		vkcfg.inst_info.enabledExtensionCount 		= INST_EXS;
		vkcfg.inst_info.ppEnabledExtensionNames		= instance_extensions;

	vr("vkCreateInstance", &vkres, vob.VKI,
		vkCreateInstance(&vkcfg.inst_info, NULL, &vob.VKI) );

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "DEBUG UTILS");			/**/
	///////////////////////////////////////////////////

	VK_Debug vkdbg;

		vkdbg.debug_msg_info.sType				= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	nf(&vkdbg.debug_msg_info);
		vkdbg.debug_msg_info.messageSeverity	= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
												| VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT 
												| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT 
												| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		vkdbg.debug_msg_info.messageType		= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
												| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		vkdbg.debug_msg_info.pfnUserCallback	= debugCallback;

	rv("vkGetInstanceProcAddr");
	auto PFN_VKCDUM = (PFN_vkCreateDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr( vob.VKI, "vkCreateDebugUtilsMessengerEXT" );

	vr("vkCreateDebugUtilsMessengerEXT", &vkres, vkdbg.vk_debug_utils_messenger_ext,
		PFN_VKCDUM(vob.VKI, &vkdbg.debug_msg_info, NULL, &vkdbg.vk_debug_utils_messenger_ext) );

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "PHYSICAL DEVICE");		/**/
	///////////////////////////////////////////////////

	uint32_t PDev_Count 	= UINT32_MAX;
	uint32_t PDev_Index 	= PDev_Count;

	vr("vkEnumeratePhysicalDevices", &vkres, PDev_Count,
		vkEnumeratePhysicalDevices(vob.VKI, &PDev_Count, NULL) );

	VkPhysicalDevice vk_physical_device[PDev_Count];
	vr("vkEnumeratePhysicalDevices", &vkres, vk_physical_device, 
		vkEnumeratePhysicalDevices(vob.VKI, &PDev_Count, vk_physical_device) );

	VK_PhysDev pdev[PDev_Count];

	for(int i = 0; i < PDev_Count; i++) {
		pdev[i].pd_count 		= PDev_Count;
		pdev[i].vk_pdev 		= vk_physical_device[i];
		rv("vkGetPhysicalDeviceProperties");
			vkGetPhysicalDeviceProperties(pdev[i].vk_pdev, &pdev[i].vk_pdev_props);

		iv("Physical Devices", 					pdev[i].vk_pdev, 												i );
		iv("apiVersion", 						pdev[i].vk_pdev_props.apiVersion, 								i );
		iv("driverVersion", 					pdev[i].vk_pdev_props.driverVersion, 							i );
		iv("deviceType", 						pdev[i].vk_pdev_props.deviceType, 								i );
		iv("deviceName", 						pdev[i].vk_pdev_props.deviceName, 								i );
		iv("maxImageDimension2D", 				pdev[i].vk_pdev_props.limits.maxImageDimension2D, 				i );
		iv("maxPushConstantsSize", 				pdev[i].vk_pdev_props.limits.maxPushConstantsSize, 				i );
		iv("maxBoundDescriptorSets", 			pdev[i].vk_pdev_props.limits.maxBoundDescriptorSets, 			i );
		iv("maxFragmentInputComponents", 		pdev[i].vk_pdev_props.limits.maxFragmentInputComponents, 		i );
		iv("maxFragmentOutputAttachments", 		pdev[i].vk_pdev_props.limits.maxFragmentOutputAttachments, 		i );
		iv("maxComputeSharedMemorySize", 		pdev[i].vk_pdev_props.limits.maxComputeSharedMemorySize, 		i );
		iv("maxComputeWorkGroupInvocations", 	pdev[i].vk_pdev_props.limits.maxComputeWorkGroupInvocations, 	i );
		iv("maxViewports", 						pdev[i].vk_pdev_props.limits.maxViewports, 						i );
		iv("minMemoryMapAlignment", 			pdev[i].vk_pdev_props.limits.minMemoryMapAlignment, 			i );
		iv("minUniformBufferOffsetAlignment", 	pdev[i].vk_pdev_props.limits.minUniformBufferOffsetAlignment, 	i );
		iv("minTexelOffset", 					pdev[i].vk_pdev_props.limits. minTexelOffset, 					i );
		iv("maxTexelOffset", 					pdev[i].vk_pdev_props.limits.maxTexelOffset, 					i );
		iv("minTexelGatherOffset", 				pdev[i].vk_pdev_props.limits. minTexelGatherOffset, 			i );
		iv("maxTexelGatherOffset", 				pdev[i].vk_pdev_props.limits.maxTexelGatherOffset, 				i );
		iv("maxFramebufferWidth", 				pdev[i].vk_pdev_props.limits.maxFramebufferWidth, 				i );
		iv("maxFramebufferHeight", 				pdev[i].vk_pdev_props.limits.maxFramebufferHeight, 				i );
		iv("maxFramebufferLayers", 				pdev[i].vk_pdev_props.limits.maxFramebufferLayers, 				i );
		iv("maxColorAttachments", 				pdev[i].vk_pdev_props.limits.maxColorAttachments, 				i );

		rv("vkGetPhysicalDeviceFeatures");
			vkGetPhysicalDeviceFeatures(pdev[i].vk_pdev, &pdev[i].vk_pdev_feats); }

//	Find a valid physical device (GPU) to use
	uint32_t PDev_TypeList[5] = { 2, 1, 3, 4, 0 };
	for(int j = 0; j < 5; j++) {
		if(PDev_Index == UINT32_MAX) {
			for(int i = 0; i < PDev_Count; i++) {
				if(	PDev_Index 							== UINT32_MAX
				&&	pdev[i].vk_pdev_props.deviceType 	== PDev_TypeList[j] ) {
					PDev_Index = i;
					ov("PDev_Index", i);
					ov("PDev_Index deviceType", PDev_TypeList[j]); } } }
		if(PDev_Index == UINT32_MAX) { ov("No devices found of type", PDev_TypeList[j]); } }
	if(PDev_Index == UINT32_MAX) { valid = 0; }

	vob.VKP_i	= PDev_Index;
	vob.VKP 	= pdev[vob.VKP_i].vk_pdev;

//	Physical device memory information
	rv("vkGetPhysicalDeviceMemoryProperties");
		vkGetPhysicalDeviceMemoryProperties(vob.VKP, &pdev[vob.VKP_i].vk_pdev_mem_props);

	ov("memoryTypeCount", 	pdev[vob.VKP_i].vk_pdev_mem_props.memoryTypeCount						);
	for(int i = 0; i < 		pdev[vob.VKP_i].vk_pdev_mem_props.memoryTypeCount; 					i++	) {
		iv("propertyFlags", pdev[vob.VKP_i].vk_pdev_mem_props.memoryTypes[i].propertyFlags,		i	); 
		iv("heapIndex", 	pdev[vob.VKP_i].vk_pdev_mem_props.memoryTypes[i].heapIndex, 		i	); }

	ov("memoryHeapCount", 	pdev[vob.VKP_i].vk_pdev_mem_props.memoryHeapCount						);
	for(int i = 0; i < 		pdev[vob.VKP_i].vk_pdev_mem_props.memoryHeapCount; 					i++	) {
		iv("size", 			pdev[vob.VKP_i].vk_pdev_mem_props.memoryHeaps[i].size,				i	); 
		iv("flags", 		pdev[vob.VKP_i].vk_pdev_mem_props.memoryHeaps[i].flags,				i	); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "QUEUES");					/**/
	///////////////////////////////////////////////////

	uint32_t PDev_QFP_Count = UINT32_MAX;
	uint32_t PDev_QFP_Index = PDev_QFP_Count;

	VK_PDQueues pdq;

//	List the available device queues
	rv("vkGetPhysicalDeviceQueueFamilyProperties");
		vkGetPhysicalDeviceQueueFamilyProperties(vob.VKP, &PDev_QFP_Count, NULL);
	ov("PDev Queue Family Props", PDev_QFP_Count);

	VkQueueFamilyProperties vk_qf_props[PDev_QFP_Count];
	rv("vkGetPhysicalDeviceQueueFamilyProperties");
		vkGetPhysicalDeviceQueueFamilyProperties(vob.VKP, &PDev_QFP_Count, vk_qf_props);

	for(int i = 0; i < PDev_QFP_Count; i++) {
		iv("queueFlags", vk_qf_props[i].queueFlags, i );
		iv("queueCount", vk_qf_props[i].queueCount, i ); }

//	Select the graphics queue
	for(int i = 0; i < PDev_QFP_Count; i++) {
		if(	PDev_QFP_Index == UINT32_MAX
		&&	vk_qf_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT ) { 
			PDev_QFP_Index = i;
		 	ov("PDev_QFP_Index", i);
		 	ov("PDev_QFP_Index queueFlags", vk_qf_props[PDev_QFP_Index].queueFlags);
		 	ov("PDev_QFP_Index queueCount", vk_qf_props[PDev_QFP_Index].queueCount); } }

	vob.VKQ_i = PDev_QFP_Index;

	float GFXQ_Priorities[vk_qf_props[vob.VKQ_i].queueCount];
	for(int i = 0; i < vk_qf_props[vob.VKQ_i].queueCount; i++) { GFXQ_Priorities[i] = 0.0f; }

		pdq.pdq_info.sType				= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	nf(&pdq.pdq_info);
		pdq.pdq_info.queueFamilyIndex	= vob.VKQ_i;
		pdq.pdq_info.queueCount			= vk_qf_props[vob.VKQ_i].queueCount;
		pdq.pdq_info.pQueuePriorities	= GFXQ_Priorities;

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "GLFW VULKAN SURFACE");	/**/
	///////////////////////////////////////////////////

	VkSurfaceKHR 				glfw_surface;
	VkSurfaceCapabilitiesKHR 	vk_surface_capabilities;
	GLFWwindow* 				glfw_W;

	if(!ei.run_headless) {
		glfwWindowHint(GLFW_CLIENT_API,	GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, 	GL_FALSE);

		ov("glfwCreateWindow", glfw_W);
		glfw_W = glfwCreateWindow(APP_W, APP_H, vkcfg.app_info.pApplicationName, NULL, NULL);

		vr("glfwGetPhysicalDevicePresentationSupport", &vkres, "GLFW", VkResult(
			glfwGetPhysicalDevicePresentationSupport(vob.VKI, vob.VKP, vob.VKQ_i) ) );

		vr("glfwCreateWindowSurface", &vkres, "GLFW", VkResult(
			glfwCreateWindowSurface(vob.VKI, glfw_W, NULL, &glfw_surface) ) );

		vr("vkGetPhysicalDeviceSurfaceCapabilitiesKHR", &vkres, "ARRAY",
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vob.VKP, glfw_surface, &vk_surface_capabilities) );
			ov("minImageCount", 			vk_surface_capabilities.minImageCount			);
			ov("maxImageCount", 			vk_surface_capabilities.maxImageCount			);
			ov("currentExtent.width", 		vk_surface_capabilities.currentExtent.width		);
			ov("currentExtent.height", 		vk_surface_capabilities.currentExtent.height	);
			ov("maxImageArrayLayers", 		vk_surface_capabilities.maxImageArrayLayers		);
			ov("supportedCompositeAlpha", 	vk_surface_capabilities.supportedCompositeAlpha	);
			ov("supportedUsageFlags", 		vk_surface_capabilities.supportedUsageFlags		);

		uint32_t vkpd_surface_format_count;
		vr("vkGetPhysicalDeviceSurfaceFormatsKHR", &vkres, vkpd_surface_format_count,
			vkGetPhysicalDeviceSurfaceFormatsKHR(vob.VKP, glfw_surface, &vkpd_surface_format_count, NULL) );

		VkSurfaceFormatKHR vk_surface_format[vkpd_surface_format_count];
		vr("vkGetPhysicalDeviceSurfaceFormatsKHR", &vkres, "ARRAY",
			vkGetPhysicalDeviceSurfaceFormatsKHR(vob.VKP, glfw_surface, &vkpd_surface_format_count, vk_surface_format) );
		for(int i = 0; i < vkpd_surface_format_count; i++) {
			iv("vk_surface_format.format", 		vk_surface_format[i].format, 		i );
			iv("vk_surface_format.colorSpace", 	vk_surface_format[i].colorSpace, 	i ); }

	//	Is Presentation Supported by this queue index?
		VkBool32 surface_supported;
		vr("vkGetPhysicalDeviceSurfaceSupportKHR", &vkres, pdq.pdq_info.queueFamilyIndex,
			vkGetPhysicalDeviceSurfaceSupportKHR(vob.VKP, pdq.pdq_info.queueFamilyIndex, glfw_surface, &surface_supported) );
			ov("Surface Supported", ((surface_supported==VK_TRUE)?"TRUE":"FALSE")); } // TODO ? Is this even checking?
	else { rv("Headless Mode Enabled!"); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "LOGICAL DEVICE");			/**/
	///////////////////////////////////////////////////

	VK_LogDev ldev;

		ldev.ldev_info.sType 						= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	nf(&ldev.ldev_info);
		ldev.ldev_info.queueCreateInfoCount 		= 1;
		ldev.ldev_info.pQueueCreateInfos 			= &pdq.pdq_info;
		ldev.ldev_info.enabledLayerCount 			= 0;
		ldev.ldev_info.ppEnabledLayerNames 			= NULL;
		ldev.ldev_info.enabledExtensionCount 		= LDEV_EXS;
		ldev.ldev_info.ppEnabledExtensionNames 		= device_extensions;
		ldev.ldev_info.pEnabledFeatures 			= &pdev[vob.VKP_i].vk_pdev_feats;

	vr("vkCreateDevice", &vkres, vob.VKL,
		vkCreateDevice(vob.VKP, &ldev.ldev_info, NULL, &vob.VKL) );
	ov("VkDevice", vob.VKL);

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "SWAPCHAIN");				/**/
	///////////////////////////////////////////////////

	VkSwapchainCreateInfoKHR vk_swapchhain_info;
		vk_swapchhain_info.sType	= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	nf(&vk_swapchhain_info);
		vk_swapchhain_info.surface					= glfw_surface;
		vk_swapchhain_info.minImageCount			= vk_surface_capabilities.minImageCount;
		vk_swapchhain_info.imageFormat				= VK_FORMAT_B8G8R8A8_UNORM;
		vk_swapchhain_info.imageColorSpace			= VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		vk_swapchhain_info.imageExtent				= vk_surface_capabilities.currentExtent;
		vk_swapchhain_info.imageArrayLayers			= 1;
		vk_swapchhain_info.imageUsage				= vk_surface_capabilities.supportedUsageFlags;
		vk_swapchhain_info.imageSharingMode			= VK_SHARING_MODE_EXCLUSIVE;
		vk_swapchhain_info.queueFamilyIndexCount	= 1;
		vk_swapchhain_info.pQueueFamilyIndices		= &pdq.pdq_info.queueFamilyIndex;
		vk_swapchhain_info.preTransform				= vk_surface_capabilities.currentTransform;
		vk_swapchhain_info.compositeAlpha			= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		vk_swapchhain_info.presentMode				= VK_PRESENT_MODE_IMMEDIATE_KHR;
		vk_swapchhain_info.clipped					= VK_FALSE;
		vk_swapchhain_info.oldSwapchain				= VK_NULL_HANDLE;

	VkSwapchainKHR vk_swapchain;
	uint32_t swap_image_count = 0;
	VkImage vk_image_swapimgs[swap_image_count];

	if(!ei.run_headless) {
	vr("vkCreateSwapchainKHR", &vkres, vk_swapchain,
		vkCreateSwapchainKHR(vob.VKL, &vk_swapchhain_info, NULL, &vk_swapchain) );

	vr("vkGetSwapchainImagesKHR", &vkres, swap_image_count,
		vkGetSwapchainImagesKHR(vob.VKL, vk_swapchain, &swap_image_count, NULL) );

	vr("vkGetSwapchainImagesKHR", &vkres, "ARRAY",
		vkGetSwapchainImagesKHR(vob.VKL, vk_swapchain, &swap_image_count, vk_image_swapimgs) );

	for(int i = 0; i < swap_image_count; i++) {
		iv("Swapchain Image", vk_image_swapimgs[i], i); } }
	else { rv("Headless Mode Enabled!"); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "WORK LAYER IMAGES");		/**/
	///////////////////////////////////////////////////

	VK_Layer_2x2D work;

	for(int i = 0; i < 2; i++) {
		work.ext3D[i].width 	= APP_W;
		work.ext3D[i].height 	= APP_H;
		work.ext3D[i].depth 	= 1;

		work.img_info[i].sType 					= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	nf(&work.img_info[i]);
		work.img_info[i].imageType 				= VK_IMAGE_TYPE_2D;
		work.img_info[i].format 				= VK_FORMAT_R16G16B16A16_UNORM;
		work.img_info[i].extent 				= work.ext3D[i];
		work.img_info[i].mipLevels 				= 1;
		work.img_info[i].arrayLayers 			= 1;
		work.img_info[i].samples 				= VK_SAMPLE_COUNT_1_BIT;
		work.img_info[i].tiling 				= VK_IMAGE_TILING_OPTIMAL;
		work.img_info[i].usage 					= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
												| VK_IMAGE_USAGE_SAMPLED_BIT
												| VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		work.img_info[i].sharingMode 			= VK_SHARING_MODE_EXCLUSIVE;
		work.img_info[i].queueFamilyIndexCount 	= 0;
		work.img_info[i].pQueueFamilyIndices 	= NULL;
		work.img_info[i].initialLayout 			= VK_IMAGE_LAYOUT_UNDEFINED;

		vr("vkCreateImage", &vkres, work.vk_image[i],
			vkCreateImage(vob.VKL, &work.img_info[i], NULL, &work.vk_image[i]) );

		rv("vkGetImageMemoryRequirements");
			vkGetImageMemoryRequirements(vob.VKL, work.vk_image[i], &work.vk_mem_reqs[i]);

		iv("vk_mem_reqs size", 				work.vk_mem_reqs[i].size, 			i);
		iv("vk_mem_reqs alignment", 		work.vk_mem_reqs[i].alignment,		i);
		iv("vk_mem_reqs memoryTypeBits", 	work.vk_mem_reqs[i].memoryTypeBits, i);

		work.MTB_index[i] = findProperties(
			&pdev[vob.VKP_i].vk_pdev_mem_props,
			work.vk_mem_reqs[i].memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

		iv("memoryTypeIndex", work.MTB_index[i], i);

		work.vk_mem_allo_info[i].sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		work.vk_mem_allo_info[i].pNext				= NULL;
		work.vk_mem_allo_info[i].allocationSize		= work.vk_mem_reqs[i].size;
		work.vk_mem_allo_info[i].memoryTypeIndex	= work.MTB_index[i];

		vr("vkAllocateMemory", &vkres, work.vk_dev_mem[i],
			vkAllocateMemory(vob.VKL, &work.vk_mem_allo_info[i], NULL, &work.vk_dev_mem[i]) );

		vr("vkBindImageMemory", &vkres, work.vk_image[i],
			vkBindImageMemory(vob.VKL,  work.vk_image[i], work.vk_dev_mem[i], 0) ); }
	
	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "BLIT EXPORT IMAGE");		/**/
	///////////////////////////////////////////////////

	VK_Layer_1x2D blit;

		blit.ext3D.width 	= APP_W;
		blit.ext3D.height 	= APP_H;
		blit.ext3D.depth 	= 1;

		blit.img_info.sType 				= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	nf(&blit.img_info);
		blit.img_info.imageType 			= VK_IMAGE_TYPE_2D;
		blit.img_info.format 				= VK_FORMAT_R8G8B8A8_UNORM;
		blit.img_info.extent 				= blit.ext3D;
		blit.img_info.mipLevels 			= 1;
		blit.img_info.arrayLayers 			= 1;
		blit.img_info.samples 				= VK_SAMPLE_COUNT_1_BIT;
		blit.img_info.tiling 				= VK_IMAGE_TILING_OPTIMAL;
		blit.img_info.usage 				= VK_IMAGE_USAGE_TRANSFER_SRC_BIT
											| VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		blit.img_info.sharingMode 			= VK_SHARING_MODE_EXCLUSIVE;
		blit.img_info.queueFamilyIndexCount = 0;
		blit.img_info.pQueueFamilyIndices 	= NULL;
		blit.img_info.initialLayout 		= VK_IMAGE_LAYOUT_UNDEFINED;

		vr("vkCreateImage", &vkres, blit.vk_image,
			vkCreateImage(vob.VKL, &blit.img_info, NULL, &blit.vk_image) );

		rv("vkGetImageMemoryRequirements");
			vkGetImageMemoryRequirements(vob.VKL, blit.vk_image, &blit.vk_mem_reqs);

		ov("vk_mem_reqs size", 				blit.vk_mem_reqs.size);
		ov("vk_mem_reqs alignment", 		blit.vk_mem_reqs.alignment);
		ov("vk_mem_reqs memoryTypeBits", 	blit.vk_mem_reqs.memoryTypeBits);

		blit.MTB_index = findProperties(
			&pdev[vob.VKP_i].vk_pdev_mem_props,
			blit.vk_mem_reqs.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

		ov("memoryTypeIndex", blit.MTB_index);

		blit.vk_mem_allo_info.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		blit.vk_mem_allo_info.pNext				= NULL;
		blit.vk_mem_allo_info.allocationSize	= blit.vk_mem_reqs.size;
		blit.vk_mem_allo_info.memoryTypeIndex	= blit.MTB_index;

		vr("vkAllocateMemory", &vkres, blit.vk_dev_mem,
			vkAllocateMemory(vob.VKL, &blit.vk_mem_allo_info, NULL, &blit.vk_dev_mem) );

		vr("vkBindImageMemory", &vkres, blit.vk_image,
			vkBindImageMemory(vob.VKL,  blit.vk_image, blit.vk_dev_mem, 0) );

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "SHADER DATA");			/**/
	///////////////////////////////////////////////////

	ShaderData shade_data[VERT_FLS+FRAG_FLS];

		for(int i = 0; i < VERT_FLS; i++) {
			rv("getShaderCodeInfo");
			shade_data[i].SC_info 		= getShaderCodeInfo(filepath_vert[i]);
			shade_data[i].stage_bits 	= VK_SHADER_STAGE_VERTEX_BIT; }

		for(int i = VERT_FLS; i < VERT_FLS+FRAG_FLS; i++) {
			rv("getShaderCodeInfo");
			shade_data[i].SC_info 		= getShaderCodeInfo(filepath_frag[i-VERT_FLS]);
			shade_data[i].stage_bits 	= VK_SHADER_STAGE_FRAGMENT_BIT; }

		for(int i = 0; i < VERT_FLS+FRAG_FLS; i++) {
			shade_data[i].vk_SM_info.sType		= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		nf(&shade_data[i].vk_SM_info);
			shade_data[i].vk_SM_info.codeSize	= shade_data[i].SC_info.shaderBytes;
			shade_data[i].vk_SM_info.pCode		= reinterpret_cast<const uint32_t*>(shade_data[i].SC_info.shaderData.data());
			iv("Vert shaderFilename",		shade_data[i].SC_info.shaderFilename, 						i);
			iv("Vert shaderBytes", 			shade_data[i].SC_info.shaderBytes, 							i);
			iv("Vert shaderBytesValid", ( 	shade_data[i].SC_info.shaderBytesValid?"TRUE":"FALSE" ), 	i); }

		for(int i = 0; i < VERT_FLS+FRAG_FLS; i++) {
			vr("vkCreateShaderModule", &vkres, shade_data[i].vk_shader_module,
				vkCreateShaderModule(vob.VKL, &shade_data[i].vk_SM_info, NULL, &shade_data[i].vk_shader_module) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RENDERPASS INFO");		/**/
	///////////////////////////////////////////////////

	VK_RPConfig rpass_info;

		rpass_info.rect2D.offset.x		= 0;
		rpass_info.rect2D.offset.y		= 0;
		rpass_info.rect2D.extent.width	= APP_W;
		rpass_info.rect2D.extent.height	= APP_H;

		rpass_info.clear_val.color = { 1.0f, 0.0f, 0.0f, 1.0f };

		rpass_info.img_subres_range.aspectMask 			= VK_IMAGE_ASPECT_COLOR_BIT;
		rpass_info.img_subres_range.baseMipLevel 		= 0;
		rpass_info.img_subres_range.levelCount 			= 1;
		rpass_info.img_subres_range.baseArrayLayer 		= 0;
		rpass_info.img_subres_range.layerCount 			= 1;

		rpass_info.img_subres_layer.aspectMask 			= VK_IMAGE_ASPECT_COLOR_BIT;
		rpass_info.img_subres_layer.mipLevel 			= 0;
		rpass_info.img_subres_layer.baseArrayLayer 		= 0;
		rpass_info.img_subres_layer.layerCount 			= 1;

		rpass_info.img_blit.srcSubresource 				= rpass_info.img_subres_layer;
		rpass_info.img_blit.srcOffsets[0].x 			= 0;
		rpass_info.img_blit.srcOffsets[0].y 			= 0;
		rpass_info.img_blit.srcOffsets[0].z 			= 0;
		rpass_info.img_blit.srcOffsets[1].x 			= APP_W;
		rpass_info.img_blit.srcOffsets[1].y 			= APP_H;
		rpass_info.img_blit.srcOffsets[1].z 			= 1;
		rpass_info.img_blit.dstSubresource 				= rpass_info.img_subres_layer;
		rpass_info.img_blit.dstOffsets[0].x 			= 0;
		rpass_info.img_blit.dstOffsets[0].y 			= 0;
		rpass_info.img_blit.dstOffsets[0].z 			= 0;
		rpass_info.img_blit.dstOffsets[1].x 			= APP_W;
		rpass_info.img_blit.dstOffsets[1].y 			= APP_H;
		rpass_info.img_blit.dstOffsets[1].z 			= 1;

		rpass_info.buffer_img_cpy.bufferOffset 			= 0;
		rpass_info.buffer_img_cpy.bufferRowLength 		= APP_W;
		rpass_info.buffer_img_cpy.bufferImageHeight 	= APP_H;
		rpass_info.buffer_img_cpy.imageSubresource 		= rpass_info.img_subres_layer;
		rpass_info.buffer_img_cpy.imageOffset.x 		= 0;
		rpass_info.buffer_img_cpy.imageOffset.y 		= 0;
		rpass_info.buffer_img_cpy.imageOffset.z 		= 0;
		rpass_info.buffer_img_cpy.imageExtent.width 	= APP_W;
		rpass_info.buffer_img_cpy.imageExtent.height 	= APP_H;
		rpass_info.buffer_img_cpy.imageExtent.depth		= 1;

		rpass_info.vk_viewport.x						= 0;
		rpass_info.vk_viewport.y						= 0;
		rpass_info.vk_viewport.width					= APP_W;
		rpass_info.vk_viewport.height					= APP_H;
		rpass_info.vk_viewport.minDepth					= 0.0f;
		rpass_info.vk_viewport.maxDepth					= 1.0f;

		rpass_info.samp_info.sType 						= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	nf(&rpass_info.samp_info);
		rpass_info.samp_info.magFilter					= VK_FILTER_NEAREST;
		rpass_info.samp_info.minFilter					= VK_FILTER_NEAREST;
		rpass_info.samp_info.mipmapMode					= VK_SAMPLER_MIPMAP_MODE_NEAREST;
		rpass_info.samp_info.addressModeU				= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		rpass_info.samp_info.addressModeV				= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		rpass_info.samp_info.addressModeW				= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		rpass_info.samp_info.mipLodBias					= 1.0f;
		rpass_info.samp_info.anisotropyEnable			= VK_FALSE;
		rpass_info.samp_info.maxAnisotropy				= 1.0f;
		rpass_info.samp_info.compareEnable				= VK_FALSE;
		rpass_info.samp_info.compareOp					= VK_COMPARE_OP_NEVER;
		rpass_info.samp_info.minLod						= 1.0f;
		rpass_info.samp_info.maxLod						= 1.0f;
		rpass_info.samp_info.borderColor				= VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		rpass_info.samp_info.unnormalizedCoordinates	= VK_FALSE;

	vr("vkCreateSampler", &vkres, rpass_info.vk_sampler,
		vkCreateSampler(vob.VKL, &rpass_info.samp_info, NULL, &rpass_info.vk_sampler) );

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "PIPELINE INFO"); 			/**/
	///////////////////////////////////////////////////

	VK_PipeInfo pipe_info;

		pipe_info.p_rast_info.sType						= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	nf(&pipe_info.p_rast_info);
		pipe_info.p_rast_info.depthClampEnable			= VK_FALSE;
		pipe_info.p_rast_info.rasterizerDiscardEnable	= VK_FALSE;
		pipe_info.p_rast_info.polygonMode				= VK_POLYGON_MODE_FILL;
		pipe_info.p_rast_info.cullMode					= VK_CULL_MODE_NONE;
		pipe_info.p_rast_info.frontFace					= VK_FRONT_FACE_CLOCKWISE;
		pipe_info.p_rast_info.depthBiasEnable			= VK_FALSE;
		pipe_info.p_rast_info.depthBiasConstantFactor	= 0.0f;
		pipe_info.p_rast_info.depthBiasClamp			= 0.0f;
		pipe_info.p_rast_info.depthBiasSlopeFactor		= 0.0f;
		pipe_info.p_rast_info.lineWidth					= 1.0f;

		pipe_info.p_vprt_info.sType 					= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	nf(&pipe_info.p_vprt_info);
		pipe_info.p_vprt_info.viewportCount				= 1;
		pipe_info.p_vprt_info.pViewports				= &rpass_info.vk_viewport;
		pipe_info.p_vprt_info.scissorCount				= 1;
		pipe_info.p_vprt_info.pScissors					= &rpass_info.rect2D;

		pipe_info.p_msam_info.sType						= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	nf(&pipe_info.p_msam_info);
		pipe_info.p_msam_info.rasterizationSamples		= VK_SAMPLE_COUNT_1_BIT;
		pipe_info.p_msam_info.sampleShadingEnable		= VK_FALSE;
		pipe_info.p_msam_info.minSampleShading			= 0.0f;
		pipe_info.p_msam_info.pSampleMask				= NULL;
		pipe_info.p_msam_info.alphaToCoverageEnable		= VK_FALSE;
		pipe_info.p_msam_info.alphaToOneEnable			= VK_FALSE;

		pipe_info.p_vtin_info.sType						= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	nf(&pipe_info.p_vtin_info);
		pipe_info.p_vtin_info.vertexBindingDescriptionCount		= 0;
		pipe_info.p_vtin_info.pVertexBindingDescriptions		= NULL;
		pipe_info.p_vtin_info.vertexAttributeDescriptionCount	= 0;
		pipe_info.p_vtin_info.pVertexAttributeDescriptions		= NULL;

		pipe_info.p_inas_info.sType						= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	nf(&pipe_info.p_inas_info);
		pipe_info.p_inas_info.topology					= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipe_info.p_inas_info.primitiveRestartEnable	= VK_FALSE;

		pipe_info.p_cbat_info.blendEnable				= VK_FALSE;
		pipe_info.p_cbat_info.srcColorBlendFactor		= VK_BLEND_FACTOR_SRC_ALPHA;
		pipe_info.p_cbat_info.dstColorBlendFactor		= VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		pipe_info.p_cbat_info.colorBlendOp				= VK_BLEND_OP_ADD;
		pipe_info.p_cbat_info.srcAlphaBlendFactor		= VK_BLEND_FACTOR_ONE;
		pipe_info.p_cbat_info.dstAlphaBlendFactor		= VK_BLEND_FACTOR_ZERO;
		pipe_info.p_cbat_info.alphaBlendOp				= VK_BLEND_OP_ADD;
		pipe_info.p_cbat_info.colorWriteMask			= 15;

		pipe_info.p_cbst_info.sType						= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	nf(&pipe_info.p_cbst_info);
		pipe_info.p_cbst_info.logicOpEnable				= VK_FALSE;
		pipe_info.p_cbst_info.logicOp					= VK_LOGIC_OP_NO_OP;
		pipe_info.p_cbst_info.attachmentCount			= 1;
		pipe_info.p_cbst_info.pAttachments				= &pipe_info.p_cbat_info;
		pipe_info.p_cbst_info.blendConstants[0]			= 1.0f;
		pipe_info.p_cbst_info.blendConstants[1]			= 1.0f;
		pipe_info.p_cbst_info.blendConstants[2]			= 1.0f;
		pipe_info.p_cbst_info.blendConstants[3]			= 1.0f;

		for(int i = 0; i < VERT_FLS+FRAG_FLS; i++) {
			pipe_info.p_shad_info[i].sType					= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		nf(&pipe_info.p_shad_info[i]);
			pipe_info.p_shad_info[i].stage					= shade_data[i].stage_bits;
			pipe_info.p_shad_info[i].module					= shade_data[i].vk_shader_module;
			pipe_info.p_shad_info[i].pName					= "main";
			pipe_info.p_shad_info[i].pSpecializationInfo	= NULL; }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "COMMAND BUFFERS");		/**/
	///////////////////////////////////////////////////

	// Are all of these used? TODO

	VK_Command combuf_imgui_loop[swap_image_count];
	for(int i = 0; i < swap_image_count; i++) {
		combuf_imgui_loop[i].pool_info.sType							= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		combuf_imgui_loop[i].pool_info.pNext							= NULL;
		combuf_imgui_loop[i].pool_info.flags							= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		combuf_imgui_loop[i].pool_info.queueFamilyIndex					= vob.VKQ_i;

		vr("vkCreateCommandPool", &vkres, combuf_imgui_loop[i].vk_command_pool,
			vkCreateCommandPool(vob.VKL, &combuf_imgui_loop[i].pool_info, NULL, &combuf_imgui_loop[i].vk_command_pool) );

		combuf_imgui_loop[i].comm_buff_alloc_info.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		combuf_imgui_loop[i].comm_buff_alloc_info.pNext					= NULL;
		combuf_imgui_loop[i].comm_buff_alloc_info.commandPool			= combuf_imgui_loop[i].vk_command_pool;
		combuf_imgui_loop[i].comm_buff_alloc_info.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		combuf_imgui_loop[i].comm_buff_alloc_info.commandBufferCount	= 1;

		vr("vkAllocateCommandBuffers", &vkres, combuf_imgui_loop[i].vk_command_buffer,
			vkAllocateCommandBuffers(vob.VKL, &combuf_imgui_loop[i].comm_buff_alloc_info, &combuf_imgui_loop[i].vk_command_buffer) );

		combuf_imgui_loop[i].comm_buff_begin_info.sType 				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	nf(&combuf_imgui_loop[i].comm_buff_begin_info);
		combuf_imgui_loop[i].comm_buff_begin_info.pInheritanceInfo		= NULL; }



	VK_Command combuf_pres_init[swap_image_count];
	for(int i = 0; i < swap_image_count; i++) {
		combuf_pres_init[i].pool_info.sType							= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		combuf_pres_init[i].pool_info.pNext							= NULL;
		combuf_pres_init[i].pool_info.flags							= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		combuf_pres_init[i].pool_info.queueFamilyIndex				= vob.VKQ_i;

		vr("vkCreateCommandPool", &vkres, combuf_pres_init[i].vk_command_pool,
			vkCreateCommandPool(vob.VKL, &combuf_pres_init[i].pool_info, NULL, &combuf_pres_init[i].vk_command_pool) );

		combuf_pres_init[i].comm_buff_alloc_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		combuf_pres_init[i].comm_buff_alloc_info.pNext				= NULL;
		combuf_pres_init[i].comm_buff_alloc_info.commandPool		= combuf_pres_init[i].vk_command_pool;
		combuf_pres_init[i].comm_buff_alloc_info.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		combuf_pres_init[i].comm_buff_alloc_info.commandBufferCount	= 1;

		vr("vkAllocateCommandBuffers", &vkres, combuf_pres_init[i].vk_command_buffer,
			vkAllocateCommandBuffers(vob.VKL, &combuf_pres_init[i].comm_buff_alloc_info, &combuf_pres_init[i].vk_command_buffer) );

		combuf_pres_init[i].comm_buff_begin_info.sType 				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	nf(&combuf_pres_init[i].comm_buff_begin_info);
		combuf_pres_init[i].comm_buff_begin_info.pInheritanceInfo	= NULL; }

	VK_Command combuf_pres_loop[swap_image_count*2];
	for(int i = 0; i < swap_image_count*2; i++) {
		combuf_pres_loop[i].pool_info.sType							= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		combuf_pres_loop[i].pool_info.pNext							= NULL;
		combuf_pres_loop[i].pool_info.flags							= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		combuf_pres_loop[i].pool_info.queueFamilyIndex				= vob.VKQ_i;

		vr("vkCreateCommandPool", &vkres, combuf_pres_loop[i].vk_command_pool,
			vkCreateCommandPool(vob.VKL, &combuf_pres_loop[i].pool_info, NULL, &combuf_pres_loop[i].vk_command_pool) );

		combuf_pres_loop[i].comm_buff_alloc_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		combuf_pres_loop[i].comm_buff_alloc_info.pNext				= NULL;
		combuf_pres_loop[i].comm_buff_alloc_info.commandPool		= combuf_pres_loop[i].vk_command_pool;
		combuf_pres_loop[i].comm_buff_alloc_info.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		combuf_pres_loop[i].comm_buff_alloc_info.commandBufferCount	= 1;

		vr("vkAllocateCommandBuffers", &vkres, combuf_pres_loop[i].vk_command_buffer,
			vkAllocateCommandBuffers(vob.VKL, &combuf_pres_loop[i].comm_buff_alloc_info, &combuf_pres_loop[i].vk_command_buffer) );

		combuf_pres_loop[i].comm_buff_begin_info.sType 				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	nf(&combuf_pres_loop[i].comm_buff_begin_info);
		combuf_pres_loop[i].comm_buff_begin_info.pInheritanceInfo	= NULL; }

	VK_Command combuf_work_init[2];
	for(int i = 0; i < 2; i++) {
		combuf_work_init[i].pool_info.sType							= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		combuf_work_init[i].pool_info.pNext							= NULL;
		combuf_work_init[i].pool_info.flags							= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		combuf_work_init[i].pool_info.queueFamilyIndex				= vob.VKQ_i;

		vr("vkCreateCommandPool", &vkres, combuf_work_init[i].vk_command_pool,
			vkCreateCommandPool(vob.VKL, &combuf_work_init[i].pool_info, NULL, &combuf_work_init[i].vk_command_pool) );

		combuf_work_init[i].comm_buff_alloc_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		combuf_work_init[i].comm_buff_alloc_info.pNext				= NULL;
		combuf_work_init[i].comm_buff_alloc_info.commandPool		= combuf_work_init[i].vk_command_pool;
		combuf_work_init[i].comm_buff_alloc_info.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		combuf_work_init[i].comm_buff_alloc_info.commandBufferCount	= 1;

		vr("vkAllocateCommandBuffers", &vkres, combuf_work_init[i].vk_command_buffer,
			vkAllocateCommandBuffers(vob.VKL, &combuf_work_init[i].comm_buff_alloc_info, &combuf_work_init[i].vk_command_buffer) );

		combuf_work_init[i].comm_buff_begin_info.sType 				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	nf(&combuf_work_init[i].comm_buff_begin_info);
		combuf_work_init[i].comm_buff_begin_info.pInheritanceInfo	= NULL; }

	VK_Command combuf_work_loop[2];
	for(int i = 0; i < 2; i++) {
		combuf_work_loop[i].pool_info.sType							= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		combuf_work_loop[i].pool_info.pNext							= NULL;
		combuf_work_loop[i].pool_info.flags							= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		combuf_work_loop[i].pool_info.queueFamilyIndex				= vob.VKQ_i;

		vr("vkCreateCommandPool", &vkres, combuf_work_loop[i].vk_command_pool,
			vkCreateCommandPool(vob.VKL, &combuf_work_loop[i].pool_info, NULL, &combuf_work_loop[i].vk_command_pool) );

		combuf_work_loop[i].comm_buff_alloc_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		combuf_work_loop[i].comm_buff_alloc_info.pNext				= NULL;
		combuf_work_loop[i].comm_buff_alloc_info.commandPool		= combuf_work_loop[i].vk_command_pool;
		combuf_work_loop[i].comm_buff_alloc_info.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		combuf_work_loop[i].comm_buff_alloc_info.commandBufferCount	= 1;

		vr("vkAllocateCommandBuffers", &vkres, combuf_work_loop[i].vk_command_buffer,
			vkAllocateCommandBuffers(vob.VKL, &combuf_work_loop[i].comm_buff_alloc_info, &combuf_work_loop[i].vk_command_buffer) );

		combuf_work_loop[i].comm_buff_begin_info.sType 				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	nf(&combuf_work_loop[i].comm_buff_begin_info);
		combuf_work_loop[i].comm_buff_begin_info.pInheritanceInfo	= NULL; }

	VK_Command combuf_work_imagedata_init;
		combuf_work_imagedata_init.pool_info.sType							= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		combuf_work_imagedata_init.pool_info.pNext							= NULL;
		combuf_work_imagedata_init.pool_info.flags							= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		combuf_work_imagedata_init.pool_info.queueFamilyIndex				= vob.VKQ_i;
		vr("vkCreateCommandPool", &vkres, combuf_work_imagedata_init.vk_command_pool,
			vkCreateCommandPool(vob.VKL, &combuf_work_imagedata_init.pool_info, NULL, &combuf_work_imagedata_init.vk_command_pool) );
		combuf_work_imagedata_init.comm_buff_alloc_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		combuf_work_imagedata_init.comm_buff_alloc_info.pNext				= NULL;
		combuf_work_imagedata_init.comm_buff_alloc_info.commandPool			= combuf_work_imagedata_init.vk_command_pool;
		combuf_work_imagedata_init.comm_buff_alloc_info.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		combuf_work_imagedata_init.comm_buff_alloc_info.commandBufferCount	= 1;
		vr("vkAllocateCommandBuffers", &vkres, combuf_work_imagedata_init.vk_command_buffer,
			vkAllocateCommandBuffers(vob.VKL, &combuf_work_imagedata_init.comm_buff_alloc_info, &combuf_work_imagedata_init.vk_command_buffer) );
		combuf_work_imagedata_init.comm_buff_begin_info.sType 				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	nf(&combuf_work_imagedata_init.comm_buff_begin_info);
		combuf_work_imagedata_init.comm_buff_begin_info.pInheritanceInfo	= NULL;

	VK_Command combuf_work_imagedata[2];
	for(int i = 0; i < 2; i++) {
		combuf_work_imagedata[i].pool_info.sType							= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		combuf_work_imagedata[i].pool_info.pNext							= NULL;
		combuf_work_imagedata[i].pool_info.flags							= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		combuf_work_imagedata[i].pool_info.queueFamilyIndex				= vob.VKQ_i;

		vr("vkCreateCommandPool", &vkres, combuf_work_imagedata[i].vk_command_pool,
			vkCreateCommandPool(vob.VKL, &combuf_work_imagedata[i].pool_info, NULL, &combuf_work_imagedata[i].vk_command_pool) );

		combuf_work_imagedata[i].comm_buff_alloc_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		combuf_work_imagedata[i].comm_buff_alloc_info.pNext				= NULL;
		combuf_work_imagedata[i].comm_buff_alloc_info.commandPool		= combuf_work_imagedata[i].vk_command_pool;
		combuf_work_imagedata[i].comm_buff_alloc_info.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		combuf_work_imagedata[i].comm_buff_alloc_info.commandBufferCount	= 1;

		vr("vkAllocateCommandBuffers", &vkres, combuf_work_imagedata[i].vk_command_buffer,
			vkAllocateCommandBuffers(vob.VKL, &combuf_work_imagedata[i].comm_buff_alloc_info, &combuf_work_imagedata[i].vk_command_buffer) );

		combuf_work_imagedata[i].comm_buff_begin_info.sType 				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	nf(&combuf_work_imagedata[i].comm_buff_begin_info);
		combuf_work_imagedata[i].comm_buff_begin_info.pInheritanceInfo	= NULL; }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "QUEUE SYNC");				/**/
	///////////////////////////////////////////////////

	VK_QueueSync qsync;

	rv("vkGetDeviceQueue");
		vkGetDeviceQueue(vob.VKL, vob.VKQ_i, 0, &qsync.vk_queue);

		qsync.sub_info.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		qsync.sub_info.pNext					= NULL;
		qsync.sub_info.waitSemaphoreCount		= 0;
		qsync.sub_info.pWaitSemaphores			= NULL;
		qsync.sub_info.pWaitDstStageMask		= NULL;
		qsync.sub_info.commandBufferCount		= 1;
		qsync.sub_info.pCommandBuffers			= NULL;
		qsync.sub_info.signalSemaphoreCount		= 0;
		qsync.sub_info.pSignalSemaphores		= NULL;

		qsync.fence_info.sType					= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	nf(&qsync.fence_info);

	vr("vkCreateFence", &vkres, qsync.vk_fence,
		vkCreateFence(vob.VKL, &qsync.fence_info, NULL, &qsync.vk_fence) );

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "SWAPCHAIN SYNC");			/**/
	///////////////////////////////////////////////////

	VkSemaphoreCreateInfo vk_semaphore_info;
		vk_semaphore_info.sType 	= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	nf(&vk_semaphore_info);

	VkSemaphore vk_semaphore_swapchain;

	vr("vkCreateSemaphore", &vkres, vk_semaphore_swapchain,
		vkCreateSemaphore(vob.VKL, &vk_semaphore_info, NULL, &vk_semaphore_swapchain) );

	uint32_t swap_image_index = 0;

	VK_QueueSync swpsync;

	rv("vkGetDeviceQueue");
		vkGetDeviceQueue(vob.VKL, vob.VKQ_i, 0, &swpsync.vk_queue);

	VkPipelineStageFlags vk_pipeline_stage_flags_swpsync;
		vk_pipeline_stage_flags_swpsync = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

//	Is this section correct or needed at all? TODO
		swpsync.sub_info.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		swpsync.sub_info.pNext					= NULL;
		swpsync.sub_info.waitSemaphoreCount		= 1;
		swpsync.sub_info.pWaitSemaphores		= &vk_semaphore_swapchain;
		swpsync.sub_info.pWaitDstStageMask		= &vk_pipeline_stage_flags_swpsync;
		swpsync.sub_info.commandBufferCount		= 1;
		swpsync.sub_info.pCommandBuffers		= &combuf_pres_loop[0].vk_command_buffer; // TODO i? 0, 1, 2 ? 
		swpsync.sub_info.signalSemaphoreCount	= 1;
		swpsync.sub_info.pSignalSemaphores		= &vk_semaphore_swapchain;

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "WORK IMAGE VIEWS");		/**/
	///////////////////////////////////////////////////

	VK_ImageView work_init[2];

	for(int i = 0; i < 2; i++) {
		work_init[i].img_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	nf(&work_init[i].img_view_info);
		work_init[i].img_view_info.image 				= work.vk_image[i];
		work_init[i].img_view_info.viewType 			= VK_IMAGE_VIEW_TYPE_2D;
		work_init[i].img_view_info.format 				= work.img_info[i].format;
		work_init[i].img_view_info.components.r			= VK_COMPONENT_SWIZZLE_IDENTITY;
		work_init[i].img_view_info.components.g			= VK_COMPONENT_SWIZZLE_IDENTITY;
		work_init[i].img_view_info.components.b			= VK_COMPONENT_SWIZZLE_IDENTITY;
		work_init[i].img_view_info.components.a			= VK_COMPONENT_SWIZZLE_IDENTITY;
		work_init[i].img_view_info.subresourceRange 	= rpass_info.img_subres_range;

	vr("vkCreateImageView", &vkres, work_init[i].vk_image_view,
		vkCreateImageView(vob.VKL, &work_init[i].img_view_info, NULL, &work_init[i].vk_image_view) ); }

	for(int i = 0; i < 2; i++) {
		work_init[i].img_mem_barr.sType 				= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		work_init[i].img_mem_barr.pNext 				= NULL;
		work_init[i].img_mem_barr.srcAccessMask 		= 0;
		work_init[i].img_mem_barr.dstAccessMask 		= 0;
		work_init[i].img_mem_barr.oldLayout 			= VK_IMAGE_LAYOUT_UNDEFINED;
		work_init[i].img_mem_barr.newLayout 			= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		work_init[i].img_mem_barr.srcQueueFamilyIndex 	= vob.VKQ_i;
		work_init[i].img_mem_barr.dstQueueFamilyIndex 	= vob.VKQ_i;
		work_init[i].img_mem_barr.image 				= work.vk_image[i];
		work_init[i].img_mem_barr.subresourceRange 		= rpass_info.img_subres_range; }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RECORD WORK_INIT");		/**/
	///////////////////////////////////////////////////

	for(int i = 0; i < 2; i++) {
		vr("vkBeginCommandBuffer", &vkres, i,
			vkBeginCommandBuffer(combuf_work_init[i].vk_command_buffer, &combuf_work_init[i].comm_buff_begin_info) );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_work_init[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &work_init[i].img_mem_barr );

		vr("vkEndCommandBuffer", &vkres, i,
			vkEndCommandBuffer(combuf_work_init[i].vk_command_buffer) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "SUBMIT WORK_INIT");		/**/
	///////////////////////////////////////////////////

	for(int i = 0; i < 2; i++) {
		if(valid) {
			rv("vkcombuf_work_init");
				qsync.sub_info.pCommandBuffers = &combuf_work_init[i].vk_command_buffer;
			vr("vkQueueSubmit", &vkres, i,
				vkQueueSubmit(qsync.vk_queue, 1, &qsync.sub_info, VK_NULL_HANDLE) ); } }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "WORK DESCRIPTOR SETS");	/**/
	///////////////////////////////////////////////////

	VK_DescSetLayout dsl_work;

		dsl_work.set_bind[0].binding				= 1;
		dsl_work.set_bind[0].descriptorType			= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		dsl_work.set_bind[0].descriptorCount		= 1;
		dsl_work.set_bind[0].stageFlags				= VK_SHADER_STAGE_FRAGMENT_BIT;
		dsl_work.set_bind[0].pImmutableSamplers		= NULL;
		dsl_work.set_bind[1].binding				= 0;
		dsl_work.set_bind[1].descriptorType			= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		dsl_work.set_bind[1].descriptorCount		= 1;
		dsl_work.set_bind[1].stageFlags				= VK_SHADER_STAGE_FRAGMENT_BIT;
		dsl_work.set_bind[1].pImmutableSamplers		= NULL;

		dsl_work.set_info.sType 					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	nf(&dsl_work.set_info);
		dsl_work.set_info.bindingCount				= 2;
		dsl_work.set_info.pBindings					= dsl_work.set_bind;

		dsl_work.pool_size[0].type 					= dsl_work.set_bind[0].descriptorType;
		dsl_work.pool_size[0].descriptorCount 		= 2;
		dsl_work.pool_size[1].type 					= dsl_work.set_bind[1].descriptorType;
		dsl_work.pool_size[1].descriptorCount 		= 2;

		dsl_work.pool_info.sType 					= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	nf(&dsl_work.pool_info);
		dsl_work.pool_info.maxSets 					= dsl_work.pool_size[0].descriptorCount
													+ dsl_work.pool_size[1].descriptorCount;
		dsl_work.pool_info.poolSizeCount 			= 2;
		dsl_work.pool_info.pPoolSizes 				= dsl_work.pool_size;

	ov("pool_info.maxSets", dsl_work.pool_info.maxSets);

	vr("vkCreateDescriptorSetLayout", &vkres, dsl_work.vk_desc_set_layout,
		vkCreateDescriptorSetLayout(vob.VKL, &dsl_work.set_info, NULL, &dsl_work.vk_desc_set_layout) );

	vr("vkCreateDescriptorPool", &vkres, dsl_work.vk_desc_pool,
		vkCreateDescriptorPool(vob.VKL, &dsl_work.pool_info, NULL, &dsl_work.vk_desc_pool) );

		dsl_work.allo_info.sType 					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		dsl_work.allo_info.pNext 					= NULL;
		dsl_work.allo_info.descriptorPool 			= dsl_work.vk_desc_pool;
		dsl_work.allo_info.descriptorSetCount 		= 1;
		dsl_work.allo_info.pSetLayouts 				= &dsl_work.vk_desc_set_layout;

	for(int i = 0; i < 2; i++) {
		vr("vkAllocateDescriptorSets", &vkres, dsl_work.vk_descriptor_set[i],
			vkAllocateDescriptorSets(vob.VKL, &dsl_work.allo_info, &dsl_work.vk_descriptor_set[i]) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "WORK SAMPLER");			/**/
	///////////////////////////////////////////////////

	VkDescriptorImageInfo vk_desc_img_info_work_sampler[2];
		vk_desc_img_info_work_sampler[0].sampler		= rpass_info.vk_sampler;
		vk_desc_img_info_work_sampler[0].imageView		= work_init[1].vk_image_view;
		vk_desc_img_info_work_sampler[0].imageLayout	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vk_desc_img_info_work_sampler[1].sampler		= rpass_info.vk_sampler;
		vk_desc_img_info_work_sampler[1].imageView		= work_init[0].vk_image_view;
		vk_desc_img_info_work_sampler[1].imageLayout	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet vk_write_descriptor_set_work_sampler[2];
	for(int i = 0; i < 2; i++) {
		vk_write_descriptor_set_work_sampler[i].sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vk_write_descriptor_set_work_sampler[i].pNext				= NULL;
		vk_write_descriptor_set_work_sampler[i].dstSet				= dsl_work.vk_descriptor_set[i];
		vk_write_descriptor_set_work_sampler[i].dstBinding			= 1;
		vk_write_descriptor_set_work_sampler[i].dstArrayElement		= 0;
		vk_write_descriptor_set_work_sampler[i].descriptorCount		= 1;
		vk_write_descriptor_set_work_sampler[i].descriptorType		= dsl_work.set_bind[0].descriptorType;
		vk_write_descriptor_set_work_sampler[i].pImageInfo			= &vk_desc_img_info_work_sampler[i];
		vk_write_descriptor_set_work_sampler[i].pBufferInfo			= NULL;
		vk_write_descriptor_set_work_sampler[i].pTexelBufferView	= NULL; }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "WORK UNIFORM BUFFER");	/**/
	///////////////////////////////////////////////////

	VkDeviceSize vkdevsize_work;
		vkdevsize_work = sizeof(UniBuf);
	ov("UniBuf size", vkdevsize_work);
	VkBufferCreateInfo vkbuff_info_work;
		vkbuff_info_work.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	nf(&vkbuff_info_work);
		vkbuff_info_work.size 						= vkdevsize_work;
		vkbuff_info_work.usage 						= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		vkbuff_info_work.sharingMode 				= VK_SHARING_MODE_EXCLUSIVE;
		vkbuff_info_work.queueFamilyIndexCount 		= 1;
		vkbuff_info_work.pQueueFamilyIndices 		= &vob.VKQ_i;
	VkBuffer vkbuff_work;
	vr("vkCreateBuffer", &vkres, vkbuff_work,
		vkCreateBuffer(vob.VKL, &vkbuff_info_work, NULL, &vkbuff_work) );
	VkDescriptorBufferInfo vkDescBuff_info_work;
		vkDescBuff_info_work.buffer 		= vkbuff_work;
		vkDescBuff_info_work.offset 		= 0;
		vkDescBuff_info_work.range 			= VK_WHOLE_SIZE;
	VkWriteDescriptorSet vkwritedescset_ub_work[2];
	for(int i = 0; i < 2; i++) {
		vkwritedescset_ub_work[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vkwritedescset_ub_work[i].pNext 				= NULL;
		vkwritedescset_ub_work[i].dstSet 				= dsl_work.vk_descriptor_set[i];
		vkwritedescset_ub_work[i].dstBinding 			= 0;
		vkwritedescset_ub_work[i].dstArrayElement 		= 0;
		vkwritedescset_ub_work[i].descriptorCount 		= 1;
		vkwritedescset_ub_work[i].descriptorType 		= dsl_work.set_bind[1].descriptorType;
		vkwritedescset_ub_work[i].pImageInfo 			= NULL;
		vkwritedescset_ub_work[i].pBufferInfo 			= &vkDescBuff_info_work;
		vkwritedescset_ub_work[i].pTexelBufferView 		= NULL; }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "WORK DESCRIPTOR MEMORY");	/**/
	///////////////////////////////////////////////////

	VkMemoryRequirements vkmemreqs_ub_work;
	rv("vkGetBufferMemoryRequirements");
		vkGetBufferMemoryRequirements(vob.VKL, vkbuff_work, &vkmemreqs_ub_work);
	ov("memreq size", 			vkmemreqs_ub_work.size);
	ov("memreq alignment", 		vkmemreqs_ub_work.alignment);
	ov("memreq memoryTypeBits", vkmemreqs_ub_work.memoryTypeBits);
	int mem_index_ub_work = UINT32_MAX;
		mem_index_ub_work = findProperties(
			&pdev[vob.VKP_i].vk_pdev_mem_props,
			vkmemreqs_ub_work.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );
	ov("memoryTypeIndex", mem_index_ub_work);
	VkMemoryAllocateInfo vkmemallo_info_work;
		vkmemallo_info_work.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		vkmemallo_info_work.pNext			= NULL;
		vkmemallo_info_work.allocationSize	= vkmemreqs_ub_work.size;
		vkmemallo_info_work.memoryTypeIndex	= mem_index_ub_work;
	VkDeviceMemory vkdevmem_ub_work;
	vr("vkAllocateMemory", &vkres, vkdevmem_ub_work,
		vkAllocateMemory(vob.VKL, &vkmemallo_info_work, NULL, &vkdevmem_ub_work) );
//	Assign device (GPU) memory to hold the Uniform Buffer
	vr("vkBindBufferMemory", &vkres, vkbuff_work,
		vkBindBufferMemory(vob.VKL, vkbuff_work, vkdevmem_ub_work, 0) );
//	Update the Sampler and Uniform Buffer Descriptors
	for(int i = 0; i < 2; i++) {
		rv("vkUpdateDescriptorSets");
			vkUpdateDescriptorSets(vob.VKL, 1, &vk_write_descriptor_set_work_sampler[i], 0, NULL);
		rv("vkUpdateDescriptorSets");
			vkUpdateDescriptorSets(vob.VKL, 1, &vkwritedescset_ub_work[i], 0, NULL); }

//	Map the memory location on the GPU for memcpy() to submit the Uniform Buffer
	void *pvoid_memmap_work;
	vr("vkMapMemory", &vkres, pvoid_memmap_work,
		vkMapMemory(vob.VKL, vkdevmem_ub_work, vkDescBuff_info_work.offset, vkDescBuff_info_work.range, 0, &pvoid_memmap_work) );

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "WORK RENDER PASS");		/**/
	///////////////////////////////////////////////////

	VK_RenderPass rp_work;

		rp_work.attach_desc.flags 						= 0;
		rp_work.attach_desc.format 						= work.img_info[0].format;
		rp_work.attach_desc.samples 					= VK_SAMPLE_COUNT_1_BIT;
		rp_work.attach_desc.loadOp 						= VK_ATTACHMENT_LOAD_OP_LOAD;
		rp_work.attach_desc.storeOp 					= VK_ATTACHMENT_STORE_OP_STORE;
		rp_work.attach_desc.stencilLoadOp 				= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rp_work.attach_desc.stencilStoreOp 				= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		rp_work.attach_desc.initialLayout 				= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		rp_work.attach_desc.finalLayout 				= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		rp_work.attach_ref.attachment 					= 0;
		rp_work.attach_ref.layout 						= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		rp_work.subpass_desc.flags 						= 0;
		rp_work.subpass_desc.pipelineBindPoint 			= VK_PIPELINE_BIND_POINT_GRAPHICS;
		rp_work.subpass_desc.inputAttachmentCount 		= 0;
		rp_work.subpass_desc.pInputAttachments 			= NULL;
		rp_work.subpass_desc.colorAttachmentCount 		= 1;
		rp_work.subpass_desc.pColorAttachments 			= &rp_work.attach_ref;
		rp_work.subpass_desc.pResolveAttachments 		= NULL;
		rp_work.subpass_desc.pDepthStencilAttachment 	= NULL;
		rp_work.subpass_desc.preserveAttachmentCount 	= 0;
		rp_work.subpass_desc.pPreserveAttachments 		= NULL;

		rp_work.rp_info.sType 							= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&rp_work.rp_info);
		rp_work.rp_info.attachmentCount 				= 1;
		rp_work.rp_info.pAttachments 					= &rp_work.attach_desc;
		rp_work.rp_info.subpassCount 					= 1;
		rp_work.rp_info.pSubpasses 						= &rp_work.subpass_desc;
		rp_work.rp_info.dependencyCount 				= 0;
		rp_work.rp_info.pDependencies 					= NULL;

	vr("vkCreateRenderPass", &vkres, rp_work.vk_render_pass,
		vkCreateRenderPass(vob.VKL, &rp_work.rp_info, NULL, &rp_work.vk_render_pass) );

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "WORK FRAMEBUFFER");		/**/
	///////////////////////////////////////////////////

	VK_FrameBuff fb_work[2];

		for(int i = 0; i < 2; i++) {
			fb_work[i].fb_info.sType 			= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		nf(&fb_work[i].fb_info);
			fb_work[i].fb_info.renderPass 		= rp_work.vk_render_pass;
			fb_work[i].fb_info.attachmentCount 	= 1;
			fb_work[i].fb_info.pAttachments 	= &work_init[i].vk_image_view;
			fb_work[i].fb_info.width 			= APP_W;
			fb_work[i].fb_info.height 			= APP_H;
			fb_work[i].fb_info.layers 			= 1;

		vr("vkCreateFramebuffer", &vkres, fb_work[i].vk_framebuffer,
			vkCreateFramebuffer(vob.VKL, &fb_work[i].fb_info, NULL, &fb_work[i].vk_framebuffer) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "WORK PIPELINE");			/**/
	///////////////////////////////////////////////////

	VK_Pipe	pipe_work;

		pipe_work.layout_info.sType 					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	nf(&pipe_work.layout_info);
		pipe_work.layout_info.setLayoutCount 			= 1;
		pipe_work.layout_info.pSetLayouts 				= &dsl_work.vk_desc_set_layout;
		pipe_work.layout_info.pushConstantRangeCount 	= 0;
		pipe_work.layout_info.pPushConstantRanges 		= NULL;

	vr("vkCreatePipelineLayout", &vkres, pipe_work.vk_pipeline_layout,
		vkCreatePipelineLayout(vob.VKL, &pipe_work.layout_info, NULL, &pipe_work.vk_pipeline_layout) );

		pipe_work.gfx_pipe_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	nf(&pipe_work.gfx_pipe_info);
		pipe_work.gfx_pipe_info.stageCount 				= 2;
		pipe_work.gfx_pipe_info.pStages 				= pipe_info.p_shad_info;
		pipe_work.gfx_pipe_info.pVertexInputState 		= &pipe_info.p_vtin_info;
		pipe_work.gfx_pipe_info.pInputAssemblyState 	= &pipe_info.p_inas_info;
		pipe_work.gfx_pipe_info.pTessellationState 		= NULL;
		pipe_work.gfx_pipe_info.pViewportState 			= &pipe_info.p_vprt_info;
		pipe_work.gfx_pipe_info.pRasterizationState 	= &pipe_info.p_rast_info;
		pipe_work.gfx_pipe_info.pMultisampleState 		= &pipe_info.p_msam_info;
		pipe_work.gfx_pipe_info.pDepthStencilState 		= NULL;
		pipe_work.gfx_pipe_info.pColorBlendState 		= &pipe_info.p_cbst_info;
		pipe_work.gfx_pipe_info.pDynamicState 			= NULL;
		pipe_work.gfx_pipe_info.layout 					= pipe_work.vk_pipeline_layout;
		pipe_work.gfx_pipe_info.renderPass 				= rp_work.vk_render_pass;
		pipe_work.gfx_pipe_info.subpass 				= 0;
		pipe_work.gfx_pipe_info.basePipelineHandle 		= VK_NULL_HANDLE;
		pipe_work.gfx_pipe_info.basePipelineIndex 		= -1;

	vr("vkCreateGraphicsPipelines", &vkres, pipe_work.vk_pipeline,
		vkCreateGraphicsPipelines(vob.VKL, VK_NULL_HANDLE, 1, &pipe_work.gfx_pipe_info, NULL, &pipe_work.vk_pipeline) );

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RECORD WORK LOOP");		/**/
	///////////////////////////////////////////////////

	VkRenderPassBeginInfo vkrpbegininfo_work[2];
	for(int i = 0; i < 2; i++) {
		vkrpbegininfo_work[i].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegininfo_work[i].pNext 				= NULL;
		vkrpbegininfo_work[i].renderPass 			= rp_work.vk_render_pass;
		vkrpbegininfo_work[i].framebuffer 			= fb_work[i].vk_framebuffer;
		vkrpbegininfo_work[i].renderArea 			= rpass_info.rect2D;
		vkrpbegininfo_work[i].clearValueCount 		= 1;
		vkrpbegininfo_work[i].pClearValues 			= &rpass_info.clear_val; }

	for(int i = 0; i < 2; i++) {
		vr("vkBeginCommandBuffer", &vkres, i,
			vkBeginCommandBuffer(combuf_work_loop[i].vk_command_buffer, &combuf_work_loop[i].comm_buff_begin_info) );

			rv("vkCmdBeginRenderPass");
				vkCmdBeginRenderPass (
					combuf_work_loop[i].vk_command_buffer, &vkrpbegininfo_work[i], VK_SUBPASS_CONTENTS_INLINE );

				rv("vkCmdBindPipeline");
					vkCmdBindPipeline (
						combuf_work_loop[i].vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_work.vk_pipeline );

				rv("vkCmdBindDescriptorSets");
					vkCmdBindDescriptorSets (
						combuf_work_loop[i].vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_work.vk_pipeline_layout,
						0, 1, &dsl_work.vk_descriptor_set[i], 0, NULL );

				rv("vkCmdDraw");
					vkCmdDraw (
						combuf_work_loop[i].vk_command_buffer, 3, 1, 0, 0 );

			rv("vkCmdEndRenderPass");
				vkCmdEndRenderPass(combuf_work_loop[i].vk_command_buffer);

		vr("vkEndCommandBuffer", &vkres, i,
			vkEndCommandBuffer(combuf_work_loop[i].vk_command_buffer) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "IMGUI RENDER PASS");		/**/
	///////////////////////////////////////////////////

	VK_RenderPass rp_imgui;

		rp_imgui.attach_desc.flags 						= 0;
		rp_imgui.attach_desc.format 					= VK_FORMAT_B8G8R8A8_UNORM;
		rp_imgui.attach_desc.samples 					= VK_SAMPLE_COUNT_1_BIT;
		rp_imgui.attach_desc.loadOp 					= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rp_imgui.attach_desc.storeOp 					= VK_ATTACHMENT_STORE_OP_STORE;
		rp_imgui.attach_desc.stencilLoadOp 				= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rp_imgui.attach_desc.stencilStoreOp 			= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		rp_imgui.attach_desc.initialLayout 				= VK_IMAGE_LAYOUT_UNDEFINED;
		rp_imgui.attach_desc.finalLayout 				= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		rp_imgui.attach_ref.attachment 					= 0;
		rp_imgui.attach_ref.layout 						= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		rp_imgui.subpass_desc.flags 					= 0;
		rp_imgui.subpass_desc.pipelineBindPoint 		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		rp_imgui.subpass_desc.inputAttachmentCount 		= 0;
		rp_imgui.subpass_desc.pInputAttachments 		= NULL;
		rp_imgui.subpass_desc.colorAttachmentCount 		= 1;
		rp_imgui.subpass_desc.pColorAttachments 		= &rp_imgui.attach_ref;
		rp_imgui.subpass_desc.pResolveAttachments 		= NULL;
		rp_imgui.subpass_desc.pDepthStencilAttachment 	= NULL;
		rp_imgui.subpass_desc.preserveAttachmentCount 	= 0;
		rp_imgui.subpass_desc.pPreserveAttachments 		= NULL;

		rp_imgui.rp_info.sType 							= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&rp_imgui.rp_info);
		rp_imgui.rp_info.attachmentCount 				= 1;
		rp_imgui.rp_info.pAttachments 					= &rp_imgui.attach_desc;
		rp_imgui.rp_info.subpassCount 					= 1;
		rp_imgui.rp_info.pSubpasses 					= &rp_imgui.subpass_desc;
		rp_imgui.rp_info.dependencyCount 				= 0;
		rp_imgui.rp_info.pDependencies 					= NULL;

	vr("vkCreateRenderPass", &vkres, rp_imgui.vk_render_pass,
		vkCreateRenderPass(vob.VKL, &rp_imgui.rp_info, NULL, &rp_imgui.vk_render_pass) );

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "IMGUI FRAMEBUFFER");		/**/
	///////////////////////////////////////////////////

	VK_ImageView vk_imgview_imgui[swap_image_count];

	for(int i = 0; i < swap_image_count; i++) {
		vk_imgview_imgui[i].img_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	nf(&vk_imgview_imgui[i].img_view_info);
		vk_imgview_imgui[i].img_view_info.image 				= vk_image_swapimgs[i];
		vk_imgview_imgui[i].img_view_info.viewType 				= VK_IMAGE_VIEW_TYPE_2D;
		vk_imgview_imgui[i].img_view_info.format 				= VK_FORMAT_B8G8R8A8_UNORM;
		vk_imgview_imgui[i].img_view_info.components.r			= VK_COMPONENT_SWIZZLE_IDENTITY;
		vk_imgview_imgui[i].img_view_info.components.g			= VK_COMPONENT_SWIZZLE_IDENTITY;
		vk_imgview_imgui[i].img_view_info.components.b			= VK_COMPONENT_SWIZZLE_IDENTITY;
		vk_imgview_imgui[i].img_view_info.components.a			= VK_COMPONENT_SWIZZLE_IDENTITY;
		vk_imgview_imgui[i].img_view_info.subresourceRange 		= rpass_info.img_subres_range;

	vr("vkCreateImageView", &vkres, vk_imgview_imgui[i].vk_image_view,
		vkCreateImageView(vob.VKL, &vk_imgview_imgui[i].img_view_info, NULL, &vk_imgview_imgui[i].vk_image_view) ); }

	VK_FrameBuff fb_imgui[swap_image_count];

		for(int i = 0; i < swap_image_count; i++) {
			fb_imgui[i].fb_info.sType 				= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		nf(&fb_imgui[i].fb_info);
			fb_imgui[i].fb_info.renderPass 			= rp_imgui.vk_render_pass;
			fb_imgui[i].fb_info.attachmentCount 	= 1;
			fb_imgui[i].fb_info.pAttachments 		= &vk_imgview_imgui[i].vk_image_view;
			fb_imgui[i].fb_info.width 				= APP_W;
			fb_imgui[i].fb_info.height 				= APP_H;
			fb_imgui[i].fb_info.layers 				= 1;

		vr("vkCreateFramebuffer", &vkres, fb_imgui[i].vk_framebuffer,
			vkCreateFramebuffer(vob.VKL, &fb_imgui[i].fb_info, NULL, &fb_imgui[i].vk_framebuffer) ); }

	VkRenderPassBeginInfo vkrpbegininfo_imgui[swap_image_count];
	for(int i = 0; i < swap_image_count; i++) {
		vkrpbegininfo_imgui[i].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegininfo_imgui[i].pNext 				= NULL;
		vkrpbegininfo_imgui[i].renderPass 			= rp_imgui.vk_render_pass;
		vkrpbegininfo_imgui[i].framebuffer 			= fb_imgui[i].vk_framebuffer;
		vkrpbegininfo_imgui[i].renderArea 			= rpass_info.rect2D;
		vkrpbegininfo_imgui[i].clearValueCount 		= 1;
		vkrpbegininfo_imgui[i].pClearValues 		= &rpass_info.clear_val; }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "WORK IMAGEDATA BUFFER");	/**/
	///////////////////////////////////////////////////

	VkBufferCreateInfo vkbuff_info_work_imagedata;
		vkbuff_info_work_imagedata.sType 					= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	nf(&vkbuff_info_work_imagedata);
		vkbuff_info_work_imagedata.size 					= work.vk_mem_reqs[0].size;
		vkbuff_info_work_imagedata.usage 					= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		vkbuff_info_work_imagedata.sharingMode 				= VK_SHARING_MODE_EXCLUSIVE;
		vkbuff_info_work_imagedata.queueFamilyIndexCount 	= 1;
		vkbuff_info_work_imagedata.pQueueFamilyIndices 		= &vob.VKQ_i;

	VkBuffer vk_buffer_work_imagedata;
	vr("vkCreateBuffer", &vkres, vk_buffer_work_imagedata,
		vkCreateBuffer(vob.VKL, &vkbuff_info_work_imagedata, NULL, &vk_buffer_work_imagedata) );

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "WORK IMAGEDATA MEMORY");	/**/
	///////////////////////////////////////////////////

	VkMemoryRequirements vk_mem_reqs_imagedata_work;
	rv("vkGetBufferMemoryRequirements");
		vkGetBufferMemoryRequirements(vob.VKL, vk_buffer_work_imagedata, &vk_mem_reqs_imagedata_work);
		ov("memreq size", 			vk_mem_reqs_imagedata_work.size);
		ov("memreq alignment", 		vk_mem_reqs_imagedata_work.alignment);
		ov("memreq memoryTypeBits", vk_mem_reqs_imagedata_work.memoryTypeBits);

	int mem_index_imagedata_work = UINT32_MAX;
		mem_index_imagedata_work = findProperties(
			&pdev[vob.VKP_i].vk_pdev_mem_props,
			vk_mem_reqs_imagedata_work.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT );
	ov("memoryTypeIndex", mem_index_imagedata_work);

	VkMemoryAllocateInfo vkmemallo_info_imagedata_work;
		vkmemallo_info_imagedata_work.sType 			= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		vkmemallo_info_imagedata_work.pNext				= NULL;
		vkmemallo_info_imagedata_work.allocationSize	= vk_mem_reqs_imagedata_work.size;
		vkmemallo_info_imagedata_work.memoryTypeIndex	= mem_index_imagedata_work;

	VkDeviceMemory vkdevmem_imagedata_work;
	vr("vkAllocateMemory", &vkres, vkdevmem_imagedata_work,
		vkAllocateMemory(vob.VKL, &vkmemallo_info_imagedata_work, NULL, &vkdevmem_imagedata_work) );

//	Assign device (GPU) memory to hold the Image Buffer
	vr("vkBindBufferMemory", &vkres, vk_buffer_work_imagedata,
		vkBindBufferMemory(vob.VKL, vk_buffer_work_imagedata, vkdevmem_imagedata_work, 0) );

//	Map the memory location on the GPU to export image data
	void* pvoid_imagedata_work;
	vr("vkMapMemory", &vkres, pvoid_imagedata_work,
		vkMapMemory(vob.VKL, vkdevmem_imagedata_work, 0, VK_WHOLE_SIZE, 0, &pvoid_imagedata_work) );

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "WORK IMAGEDATA BARRIERS");/**/
	///////////////////////////////////////////////////

	// Are all of these used? TODO

	VkImageMemoryBarrier vk_IMB_blit_imagedata_UND_to_TDO;
		vk_IMB_blit_imagedata_UND_to_TDO.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_blit_imagedata_UND_to_TDO.pNext 					= NULL;
		vk_IMB_blit_imagedata_UND_to_TDO.srcAccessMask 			= 0;
		vk_IMB_blit_imagedata_UND_to_TDO.dstAccessMask 			= 0;
		vk_IMB_blit_imagedata_UND_to_TDO.oldLayout 				= VK_IMAGE_LAYOUT_UNDEFINED;
		vk_IMB_blit_imagedata_UND_to_TDO.newLayout 				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		vk_IMB_blit_imagedata_UND_to_TDO.srcQueueFamilyIndex 	= vob.VKQ_i;
		vk_IMB_blit_imagedata_UND_to_TDO.dstQueueFamilyIndex 	= vob.VKQ_i;
		vk_IMB_blit_imagedata_UND_to_TDO.image 					= blit.vk_image;
		vk_IMB_blit_imagedata_UND_to_TDO.subresourceRange 		= rpass_info.img_subres_range;

	VkImageMemoryBarrier vk_IMB_pres_CAO_to_PRS[swap_image_count];
	for(int i = 0; i < swap_image_count; i++) {
		vk_IMB_pres_CAO_to_PRS[i].sType 					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_pres_CAO_to_PRS[i].pNext 					= NULL;
		vk_IMB_pres_CAO_to_PRS[i].srcAccessMask 			= 0;
		vk_IMB_pres_CAO_to_PRS[i].dstAccessMask 			= 0;
		vk_IMB_pres_CAO_to_PRS[i].oldLayout 				= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		vk_IMB_pres_CAO_to_PRS[i].newLayout 				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		vk_IMB_pres_CAO_to_PRS[i].srcQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_pres_CAO_to_PRS[i].dstQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_pres_CAO_to_PRS[i].image 					= vk_image_swapimgs[i];
		vk_IMB_pres_CAO_to_PRS[i].subresourceRange 			= rpass_info.img_subres_range; }

	VkImageMemoryBarrier vk_IMB_work_imagedata_SRO_to_TSO[2];
	for(int i = 0; i < 2; i++) {
		vk_IMB_work_imagedata_SRO_to_TSO[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_work_imagedata_SRO_to_TSO[i].pNext 					= NULL;
		vk_IMB_work_imagedata_SRO_to_TSO[i].srcAccessMask 			= 0;
		vk_IMB_work_imagedata_SRO_to_TSO[i].dstAccessMask 			= 0;
		vk_IMB_work_imagedata_SRO_to_TSO[i].oldLayout 				= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vk_IMB_work_imagedata_SRO_to_TSO[i].newLayout 				= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vk_IMB_work_imagedata_SRO_to_TSO[i].srcQueueFamilyIndex 	= vob.VKQ_i;
		vk_IMB_work_imagedata_SRO_to_TSO[i].dstQueueFamilyIndex 	= vob.VKQ_i;
		vk_IMB_work_imagedata_SRO_to_TSO[i].image 					= work.vk_image[i];
		vk_IMB_work_imagedata_SRO_to_TSO[i].subresourceRange 		= rpass_info.img_subres_range; }

	VkImageMemoryBarrier vk_IMB_work_imagedata_TSO_to_SRO[2];
	for(int i = 0; i < 2; i++) {
		vk_IMB_work_imagedata_TSO_to_SRO[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_work_imagedata_TSO_to_SRO[i].pNext 					= NULL;
		vk_IMB_work_imagedata_TSO_to_SRO[i].srcAccessMask 			= 0;
		vk_IMB_work_imagedata_TSO_to_SRO[i].dstAccessMask 			= 0;
		vk_IMB_work_imagedata_TSO_to_SRO[i].oldLayout 				= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vk_IMB_work_imagedata_TSO_to_SRO[i].newLayout 				= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vk_IMB_work_imagedata_TSO_to_SRO[i].srcQueueFamilyIndex 	= vob.VKQ_i;
		vk_IMB_work_imagedata_TSO_to_SRO[i].dstQueueFamilyIndex 	= vob.VKQ_i;
		vk_IMB_work_imagedata_TSO_to_SRO[i].image 					= work.vk_image[i];
		vk_IMB_work_imagedata_TSO_to_SRO[i].subresourceRange 		= rpass_info.img_subres_range; }

	VkImageMemoryBarrier vk_IMB_blit_imagedata_TDO_to_TSO;
		vk_IMB_blit_imagedata_TDO_to_TSO.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_blit_imagedata_TDO_to_TSO.pNext 					= NULL;
		vk_IMB_blit_imagedata_TDO_to_TSO.srcAccessMask 			= 0;
		vk_IMB_blit_imagedata_TDO_to_TSO.dstAccessMask 			= 0;
		vk_IMB_blit_imagedata_TDO_to_TSO.oldLayout 				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		vk_IMB_blit_imagedata_TDO_to_TSO.newLayout 				= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vk_IMB_blit_imagedata_TDO_to_TSO.srcQueueFamilyIndex 	= vob.VKQ_i;
		vk_IMB_blit_imagedata_TDO_to_TSO.dstQueueFamilyIndex 	= vob.VKQ_i;
		vk_IMB_blit_imagedata_TDO_to_TSO.image 					= blit.vk_image;
		vk_IMB_blit_imagedata_TDO_to_TSO.subresourceRange 		= rpass_info.img_subres_range;

	VkImageMemoryBarrier vk_IMB_blit_imagedata_TSO_to_TDO;
		vk_IMB_blit_imagedata_TSO_to_TDO.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_blit_imagedata_TSO_to_TDO.pNext 					= NULL;
		vk_IMB_blit_imagedata_TSO_to_TDO.srcAccessMask 			= 0;
		vk_IMB_blit_imagedata_TSO_to_TDO.dstAccessMask 			= 0;
		vk_IMB_blit_imagedata_TSO_to_TDO.oldLayout 				= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vk_IMB_blit_imagedata_TSO_to_TDO.newLayout 				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		vk_IMB_blit_imagedata_TSO_to_TDO.srcQueueFamilyIndex 	= vob.VKQ_i;
		vk_IMB_blit_imagedata_TSO_to_TDO.dstQueueFamilyIndex 	= vob.VKQ_i;
		vk_IMB_blit_imagedata_TSO_to_TDO.image 					= blit.vk_image;
		vk_IMB_blit_imagedata_TSO_to_TDO.subresourceRange 		= rpass_info.img_subres_range;

	VkImageMemoryBarrier vk_IMB_pres_UND_to_PRS[swap_image_count];
	for(int i = 0; i < swap_image_count; i++) {
		vk_IMB_pres_UND_to_PRS[i].sType 					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_pres_UND_to_PRS[i].pNext 					= NULL;
		vk_IMB_pres_UND_to_PRS[i].srcAccessMask 			= 0;
		vk_IMB_pres_UND_to_PRS[i].dstAccessMask 			= 0;
		vk_IMB_pres_UND_to_PRS[i].oldLayout 				= VK_IMAGE_LAYOUT_UNDEFINED;
		vk_IMB_pres_UND_to_PRS[i].newLayout 				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		vk_IMB_pres_UND_to_PRS[i].srcQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_pres_UND_to_PRS[i].dstQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_pres_UND_to_PRS[i].image 					= vk_image_swapimgs[i];
		vk_IMB_pres_UND_to_PRS[i].subresourceRange 		= rpass_info.img_subres_range; }

	VkImageMemoryBarrier vk_IMB_pres_PRS_to_TDO[swap_image_count];
	for(int i = 0; i < swap_image_count; i++) {
		vk_IMB_pres_PRS_to_TDO[i].sType 					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_pres_PRS_to_TDO[i].pNext 					= NULL;
		vk_IMB_pres_PRS_to_TDO[i].srcAccessMask 			= 0;
		vk_IMB_pres_PRS_to_TDO[i].dstAccessMask 			= 0;
		vk_IMB_pres_PRS_to_TDO[i].oldLayout 				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		vk_IMB_pres_PRS_to_TDO[i].newLayout 				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		vk_IMB_pres_PRS_to_TDO[i].srcQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_pres_PRS_to_TDO[i].dstQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_pres_PRS_to_TDO[i].image 					= vk_image_swapimgs[i];
		vk_IMB_pres_PRS_to_TDO[i].subresourceRange 		= rpass_info.img_subres_range; }

	VkImageMemoryBarrier vk_IMB_pres_TDO_to_PRS[swap_image_count];
	for(int i = 0; i < swap_image_count; i++) {
		vk_IMB_pres_TDO_to_PRS[i].sType 					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_pres_TDO_to_PRS[i].pNext 					= NULL;
		vk_IMB_pres_TDO_to_PRS[i].srcAccessMask 			= 0;
		vk_IMB_pres_TDO_to_PRS[i].dstAccessMask 			= 0;
		vk_IMB_pres_TDO_to_PRS[i].oldLayout 				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		vk_IMB_pres_TDO_to_PRS[i].newLayout 				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		vk_IMB_pres_TDO_to_PRS[i].srcQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_pres_TDO_to_PRS[i].dstQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_pres_TDO_to_PRS[i].image 					= vk_image_swapimgs[i];
		vk_IMB_pres_TDO_to_PRS[i].subresourceRange 		= rpass_info.img_subres_range; }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RECORD IMAGEDATA INIT");	/**/
	///////////////////////////////////////////////////

	for(int i = 0; i < 1; i++) {
		vr("vkBeginCommandBuffer", &vkres, i,
			vkBeginCommandBuffer(combuf_work_imagedata_init.vk_command_buffer, &combuf_work_imagedata_init.comm_buff_begin_info) );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_work_imagedata_init.vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_blit_imagedata_UND_to_TDO );

		vr("vkEndCommandBuffer", &vkres, i,
			vkEndCommandBuffer(combuf_work_imagedata_init.vk_command_buffer) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "SUBMIT IMAGEDATA INIT");	/**/
	///////////////////////////////////////////////////

	for(int i = 0; i < 1; i++) {
		if(valid) {
			rv("vkcombuf_work_init");
				qsync.sub_info.pCommandBuffers = &combuf_work_imagedata_init.vk_command_buffer;
			vr("vkQueueSubmit", &vkres, i,
				vkQueueSubmit(qsync.vk_queue, 1, &qsync.sub_info, VK_NULL_HANDLE) ); } }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RECORD IMAGEDATA EXPORT");/**/
	///////////////////////////////////////////////////

	for(int i = 0; i < 2; i++) {
		vr("vkBeginCommandBuffer", &vkres, i,
			vkBeginCommandBuffer(combuf_work_imagedata[i].vk_command_buffer, &combuf_work_imagedata[i].comm_buff_begin_info) );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_work_imagedata[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_work_imagedata_SRO_to_TSO[i] );

			rv("vkCmdBlitImage");
				vkCmdBlitImage (
					combuf_work_imagedata[i].vk_command_buffer, 
					work.vk_image[i], 			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					blit.vk_image, 				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1, &rpass_info.img_blit, 	VK_FILTER_NEAREST );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_work_imagedata[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_work_imagedata_TSO_to_SRO[i] );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_work_imagedata[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_blit_imagedata_TDO_to_TSO );

			rv("vkCmdCopyImageToBuffer");
				vkCmdCopyImageToBuffer (
					combuf_work_imagedata[i].vk_command_buffer, blit.vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					vk_buffer_work_imagedata, 1, &rpass_info.buffer_img_cpy );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_work_imagedata[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_blit_imagedata_TSO_to_TDO );

		vr("vkEndCommandBuffer", &vkres, i,
			vkEndCommandBuffer(combuf_work_imagedata[i].vk_command_buffer) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RECORD PRES INIT");		/**/
	///////////////////////////////////////////////////

	for(int i = 0; i < swap_image_count; i++) {
		vr("vkBeginCommandBuffer", &vkres, i,
			vkBeginCommandBuffer(combuf_pres_init[i].vk_command_buffer, &combuf_pres_init[i].comm_buff_begin_info) );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_pres_init[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_pres_UND_to_PRS[i] );

		vr("vkEndCommandBuffer", &vkres, i,
			vkEndCommandBuffer(combuf_pres_init[i].vk_command_buffer) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "SUBMIT PRES INIT");		/**/
	///////////////////////////////////////////////////

	for(int i = 0; i < swap_image_count; i++) {
		if(valid) {
			rv("vkcombuf_pres_init");
				qsync.sub_info.pCommandBuffers = &combuf_pres_init[i].vk_command_buffer;
			vr("vkQueueSubmit", &vkres, i,
				vkQueueSubmit(qsync.vk_queue, 1, &qsync.sub_info, VK_NULL_HANDLE) ); } }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RECORD PRES LOOP");		/**/
	///////////////////////////////////////////////////

	// Split function into two, so that it can sync with the two "work" frames
	for(int i = 0; i < swap_image_count*2; i++) {
		vr("vkBeginCommandBuffer", &vkres, i,
			vkBeginCommandBuffer(combuf_pres_loop[i].vk_command_buffer, &combuf_pres_loop[i].comm_buff_begin_info) );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_pres_loop[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_pres_PRS_to_TDO[i%swap_image_count] );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_pres_loop[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_work_imagedata_SRO_to_TSO[i/swap_image_count] );

			rv("vkCmdBlitImage");
				vkCmdBlitImage (
					combuf_pres_loop[i].vk_command_buffer, 
					work.vk_image[i/swap_image_count], 			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					vk_image_swapimgs[i%swap_image_count], 		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1, &rpass_info.img_blit, 	VK_FILTER_NEAREST );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_pres_loop[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_work_imagedata_TSO_to_SRO[i/swap_image_count] );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_pres_loop[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_pres_TDO_to_PRS[i%swap_image_count] );

		vr("vkEndCommandBuffer", &vkres, i,
			vkEndCommandBuffer(combuf_pres_loop[i].vk_command_buffer) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "SWAPCHAIN PRESENT INFO");	/**/
	///////////////////////////////////////////////////

	VkPresentInfoKHR vk_present_info;
		vk_present_info.sType 					= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		vk_present_info.pNext 					= NULL;
		vk_present_info.waitSemaphoreCount 		= 1;
		vk_present_info.pWaitSemaphores 		= &vk_semaphore_swapchain;
		vk_present_info.swapchainCount 			= 1;
		vk_present_info.pSwapchains 			= &vk_swapchain;
		vk_present_info.pImageIndices 			= &swap_image_index;
		vk_present_info.pResults 				= NULL;

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "DEAR IMGUI");				/**/
	///////////////////////////////////////////////////

	if(!ei.run_headless) {
		ov( "IMGUI Version", IMGUI_CHECKVERSION() );

		rv("ImGui::CreateContext");
			ImGui::CreateContext();

		rv("ImGui::StyleColorsDark");
			ImGui::StyleColorsDark();

		rv("ImGui::GetIO");
			ImGuiIO &io = ImGui::GetIO();

		rv("ImGui_ImplGlfw_InitForVulkan");
			ImGui_ImplGlfw_InitForVulkan(glfw_W, true);

		VkDescriptorPoolSize vk_descriptor_pool_size_imgui[] = {
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 					1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 	1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 			1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 			1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 		1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 		1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 			1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 			1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 	1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 	1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 			1000 } };

		VkDescriptorPoolCreateInfo vk_descriptor_pool_info_imgui;
			vk_descriptor_pool_info_imgui.sType 			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		nf(&vk_descriptor_pool_info_imgui);
			vk_descriptor_pool_info_imgui.flags 			= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			vk_descriptor_pool_info_imgui.maxSets 			= 1000;
			vk_descriptor_pool_info_imgui.poolSizeCount 	= std::size(vk_descriptor_pool_size_imgui);
			vk_descriptor_pool_info_imgui.pPoolSizes 		= vk_descriptor_pool_size_imgui;

		VkDescriptorPool vk_descriptor_pool_imgui;
		vr("vkCreateDescriptorPool", &vkres, vk_descriptor_pool_imgui,
			vkCreateDescriptorPool(vob.VKL, &vk_descriptor_pool_info_imgui, NULL, &vk_descriptor_pool_imgui) );

		ImGui_ImplVulkan_InitInfo imgui_vkinit;
			imgui_vkinit.Instance 			= vob.VKI;
			imgui_vkinit.PhysicalDevice 	= vob.VKP;
			imgui_vkinit.Device 			= vob.VKL;
			imgui_vkinit.QueueFamily 		= vob.VKQ_i;
			imgui_vkinit.Queue 				= qsync.vk_queue;
			imgui_vkinit.PipelineCache 		= VK_NULL_HANDLE;
			imgui_vkinit.DescriptorPool 	= vk_descriptor_pool_imgui;
			imgui_vkinit.Subpass 			= 0;
			imgui_vkinit.MinImageCount 		= vk_surface_capabilities.minImageCount;
			imgui_vkinit.ImageCount 		= imgui_vkinit.MinImageCount;
			imgui_vkinit.MSAASamples 		= VK_SAMPLE_COUNT_1_BIT;
			imgui_vkinit.Allocator 			= NULL;
			imgui_vkinit.CheckVkResultFn 	= NULL;

		rv("ImGui_ImplVulkan_Init");
			ImGui_ImplVulkan_Init(&imgui_vkinit, rp_imgui.vk_render_pass);

		VK_Command combuf_imgui;
			combuf_imgui.pool_info.sType							= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			combuf_imgui.pool_info.pNext							= NULL;
			combuf_imgui.pool_info.flags							= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			combuf_imgui.pool_info.queueFamilyIndex					= vob.VKQ_i;

			vr("vkCreateCommandPool", &vkres, combuf_imgui.vk_command_pool,
				vkCreateCommandPool(vob.VKL, &combuf_imgui.pool_info, NULL, &combuf_imgui.vk_command_pool) );

			combuf_imgui.comm_buff_alloc_info.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			combuf_imgui.comm_buff_alloc_info.pNext					= NULL;
			combuf_imgui.comm_buff_alloc_info.commandPool			= combuf_imgui.vk_command_pool;
			combuf_imgui.comm_buff_alloc_info.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			combuf_imgui.comm_buff_alloc_info.commandBufferCount	= 1;

			vr("vkAllocateCommandBuffers", &vkres, combuf_imgui.vk_command_buffer,
				vkAllocateCommandBuffers(vob.VKL, &combuf_imgui.comm_buff_alloc_info, &combuf_imgui.vk_command_buffer) );

			combuf_imgui.comm_buff_begin_info.sType 				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		nf(&combuf_imgui.comm_buff_begin_info);
			combuf_imgui.comm_buff_begin_info.pInheritanceInfo		= NULL;

		vr("vkBeginCommandBuffer", &vkres, combuf_imgui.vk_command_buffer,
			vkBeginCommandBuffer(combuf_imgui.vk_command_buffer, &combuf_imgui.comm_buff_begin_info) );

			rv("ImGui_ImplVulkan_CreateFontsTexture");
				ImGui_ImplVulkan_CreateFontsTexture(combuf_imgui.vk_command_buffer);

		vr("vkEndCommandBuffer", &vkres, combuf_imgui.vk_command_buffer,
			vkEndCommandBuffer(combuf_imgui.vk_command_buffer) );

		if(valid) {
			rv("combuf_imgui");
				qsync.sub_info.pCommandBuffers = &combuf_imgui.vk_command_buffer;
			vr("vkQueueSubmit", &vkres, 0,
				vkQueueSubmit(qsync.vk_queue, 1, &qsync.sub_info, VK_NULL_HANDLE) ); } }
	else { rv("Headless Mode Enabled!"); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "MAIN LOOP INIT");			/**/
	///////////////////////////////////////////////////

	uint32_t 	frame_index 	= 0;	// Loop Frame Index
	uint32_t 	work_index 		= 0;	// Work Image Generation Frame Index
	uint32_t	imgdat_idx		= 0;	// Export Image Data Index
	uint32_t	verbose_loops 	= 120;	// How many loops to output full diagnostics

//	FPS tracking init
	NS_Timer 	ftime;
	NS_Timer 	optime;
	NS_Timer 	prstime;
	NS_Timer 	uitime;
	NS_Timer 	guitime;
	NS_Timer 	cmdtime;
	int  		current_sec		= time(0);
	int  		fps_freq 		= 4;
	int  		fps_report 		= time(0) - fps_freq;

	PatternConfigData_256 pcd 	= new_PCD_256();

	loadPattern_PCD408_to_256( &ei, &pcd );

//	Uniform Buffer Object ( 64 * 32 bits maximum )
	UniBuf ub;

	update_ub( &pcd, &ub );

	UI_info ui;
		ui.mx 	= 0;
		ui.my 	= 0;
		ui.mbl 	= 0;
		ui.mbr 	= 0;
		ui.cmd 	= 0;

	IMGUI_Config gc;
		gc.load_pattern_input_check_instant = true;
		gc.load_pattern_input_check_reseed 	= true;
		

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "MAIN LOOP");				/**/
	///////////////////////////////////////////////////

//	Main Loop code
	do {

	//	Record loop start time
    	ftime = start_timer(ftime);

	//	Reset shader commands
		if(!ei.paused) { ui.cmd = 0; }
		clear_glfw_key(&glfw_key);
		clear_glfw_mouse(&glfw_mouse);

	//	Timing (seconds) setup
		current_sec = time(0);
		if( current_sec - fps_report >= fps_freq + 1) { fps_report = current_sec; }

	//	'Render'
		if(valid) {

		//	Lower the workload / FPS if paused
			if(ei.paused) { nanosleep((const struct timespec[]){{0, 16666666}}, NULL); }

			if(!ei.run_headless) {
				//	Poll for GLFW window events
					glfwPollEvents();

				//	Does IMGUI want to capture input?
					ImGuiIO& io = ImGui::GetIO();

					kc.has_keyboard = io.WantCaptureKeyboard;
					kc.has_mouse 	= io.WantCaptureMouse;

					glfwSetKeyCallback( glfw_W, glfw_keyboard_event	);

					if( !kc.has_mouse ) {
						glfwSetCursorPosCallback 	( glfw_W, glfw_mousemove_event 	 );
						glfwSetMouseButtonCallback	( glfw_W, glfw_mouseclick_event	 );
						glfwSetScrollCallback		( glfw_W, glfw_mousescroll_event );
						ui.mx = glfw_mouse.xpos;
						ui.my = glfw_mouse.ypos;
						if(glfw_mouse.button == 0) { ui.mbl = glfw_mouse.action; }
						if(glfw_mouse.button == 1) { ui.mbr = glfw_mouse.action; } }
					else { ui.mbl = 0; ui.mbr = 0; }

					if( !kc.has_keyboard ) {
					//	Release 	L-Shift: 	Reseed
						if( glfw_key.scancode ==  50 && glfw_key.action == 0 ) { ui.cmd = 1; ei.tick_loop = 1;			}
					//	Press 		X: 			Clear
						if( glfw_key.scancode ==  53 && glfw_key.action == 1 ) { ui.cmd = 2; ei.tick_loop = 1;			}
					//	Press 		Z: 			SymSeed
						if( glfw_key.scancode ==  52 && glfw_key.action >= 1 ) { ui.cmd = 3; ei.tick_loop = 1;			}
					//	Press 		SPACE: 		Toggle IMGUI
						if( glfw_key.scancode ==  65 && glfw_key.action == 1 ) { ei.show_gui = (ei.show_gui) ? 0 : 1; 	}
					//	Press 		ESC: 		Quit
						if( glfw_key.scancode ==   9 && glfw_key.action == 1 ) { glfwSetWindowShouldClose(glfw_W, 1); 	} }

				if(ei.show_gui) {
					guitime = start_timer(guitime);
						ImGui_ImplVulkan_NewFrame();
						ImGui_ImplGlfw_NewFrame();
						ImGui::NewFrame();
						imgui_menu( glfw_W, &ui, &ei, &gc );
//						ImGui::ShowDemoWindow();
						ImGui::Render();
					end_timer(guitime, "IMGUI Build Time");

					if(gc.load_pattern_input_confirm) {
						gc.load_pattern_input_confirm = false;
						loadPattern_PCD408_to_256( &ei, &pcd );
						update_ub( &pcd, &ub );
						if(gc.load_pattern_input_check_reseed) { ui.cmd = 1; ei.tick_loop = 1; } }

					cmdtime = start_timer(cmdtime);
						for(int i = 0; i < swap_image_count; i++) {
							vr("vkBeginCommandBuffer", &vkres, i,
								vkBeginCommandBuffer(combuf_imgui_loop[i].vk_command_buffer, &combuf_imgui_loop[i].comm_buff_begin_info) );

								rv("vkCmdBeginRenderPass");
									vkCmdBeginRenderPass (
										combuf_imgui_loop[i].vk_command_buffer, &vkrpbegininfo_imgui[i%swap_image_count], VK_SUBPASS_CONTENTS_INLINE );

									ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), combuf_imgui_loop[i].vk_command_buffer);

								rv("vkCmdEndRenderPass");
									vkCmdEndRenderPass(combuf_imgui_loop[i].vk_command_buffer);

								rv("vkCmdPipelineBarrier");
									vkCmdPipelineBarrier (
										combuf_imgui_loop[i].vk_command_buffer,
										VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
										0, NULL, 0, NULL,
										1, &vk_IMB_pres_CAO_to_PRS[i%swap_image_count] );

							vr("vkEndCommandBuffer", &vkres, i,
								vkEndCommandBuffer(combuf_imgui_loop[i].vk_command_buffer) ); }
					end_timer(cmdtime, "Command Buffer Build Time"); } }

		//	Update Uniform Buffer values
			ub.v63 = frame_index;
			ub.v60 = pack_ui_info(ui);

		//	Send UB values to GPU
			rv("memcpy");
				memcpy(pvoid_memmap_work, &ub, sizeof(ub));

    		optime = start_timer(optime);

			if(!ei.paused || ei.tick_loop) {
				if(valid) {
				//	Submit 'work' commands to the GPU graphics queue
					rv("vkcombuf_work");
						qsync.sub_info.pCommandBuffers = &combuf_work_loop[(frame_index+1)%2].vk_command_buffer;
					VkFence f = (ei.run_headless) ? qsync.vk_fence : VK_NULL_HANDLE;
					vr("vkQueueSubmit", &vkres, qsync.sub_info.pCommandBuffers,
						vkQueueSubmit(qsync.vk_queue, 1, &qsync.sub_info, f) ); } }

			if(!ei.run_headless) {
				if(valid) {
				//	Acquire a VkImage from the swapchain's pool
					vr("vkAcquireNextImageKHR", &vkres, swap_image_index,
						vkAcquireNextImageKHR(vob.VKL, vk_swapchain, UINT64_MAX, vk_semaphore_swapchain, VK_NULL_HANDLE, &swap_image_index) );
					ov("swap_image_index", swap_image_index); }

				if(valid) {
				//	Copy the "Work" Image to the "Swap" Image
					rv("vkcombuf_pres");
						swpsync.sub_info.pCommandBuffers = &combuf_pres_loop[swap_image_index+((frame_index%2)*swap_image_count)].vk_command_buffer;
					VkFence f = (ei.show_gui) ? VK_NULL_HANDLE : qsync.vk_fence;
					vr("vkQueueSubmit", &vkres, swpsync.sub_info.pCommandBuffers,
						vkQueueSubmit(swpsync.vk_queue, 1, &swpsync.sub_info, f) ); }

				if(valid && ei.show_gui) {
				//	Add IMGUI interface
					rv("vkcombuf_IMGUI");
						swpsync.sub_info.pCommandBuffers = &combuf_imgui_loop[swap_image_index].vk_command_buffer;
					VkFence f = qsync.vk_fence;
					vr("vkQueueSubmit", &vkres, swpsync.sub_info.pCommandBuffers,
						vkQueueSubmit(swpsync.vk_queue, 1, &swpsync.sub_info, f) ); } }

			if(valid) {
			//	Wait for the queued commands to finish execution
				do {
					vr("vkWaitForFences <100ms>", &vkres, qsync.vk_fence,
						vkWaitForFences(vob.VKL, 1, &qsync.vk_fence, VK_TRUE, 100000000) );
				} while (vkres[vkres.size()-1] == VK_TIMEOUT); }

    		end_timer(optime, "Work Queue Time");

			if(valid) {
			//	Reset the fence for reuse in the next iteration
				vr("vkResetFences", &vkres, qsync.vk_fence,
					vkResetFences(vob.VKL, 1, &qsync.vk_fence) ); }

			if(valid && !ei.run_headless) {
				vr("vkQueuePresentKHR", &vkres, swap_image_index,
					vkQueuePresentKHR(swpsync.vk_queue, &vk_present_info) ); }

			if(!ei.paused || ei.tick_loop) { frame_index++; }

			if(valid && ei.export_enabled && ei.export_frequency > 0 && frame_index % ei.export_frequency == 0 && frame_index > 0) {
			//	Submit 'imagedata' commands to the GPU graphics queue
				rv("combuf_work_imagedata");
					qsync.sub_info.pCommandBuffers = &combuf_work_imagedata[frame_index%2].vk_command_buffer;
				vr("vkQueueSubmit", &vkres, qsync.sub_info.pCommandBuffers,
					vkQueueSubmit(qsync.vk_queue, 1, &qsync.sub_info, VK_NULL_HANDLE) );

					optime = start_timer(optime);
					if(frame_index >= verbose_loops) { loglevel = MAXLOG; }
					save_image(pvoid_imagedata_work, "IMG"+std::to_string(imgdat_idx), APP_W, APP_H );
					if(frame_index >= verbose_loops) { loglevel = -1; }
					end_timer(optime, "Save ImageData");
					imgdat_idx++; }

		if(frame_index == verbose_loops) { loglevel = -1; } }

	//	End of loop
		if(fps_report == current_sec) {
			fps_report--;
			if(frame_index >= verbose_loops) { loglevel = MAXLOG; }
			end_timer(ftime, "Full Loop Time");
			if(frame_index >= verbose_loops) { loglevel = -1; } }

		if(frame_index < verbose_loops) { end_timer(ftime, "Full Loop Time"); }

		hd("STAGE:", "LOOP");

		if(frame_index == verbose_loops) { loglevel = -1; }
		if(ei.tick_loop > 0) { ei.tick_loop--; }

	} while ( valid && ((!ei.run_headless && !glfwWindowShouldClose(glfw_W)) || ei.run_headless) );

	loglevel = MAXLOG;

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "EXIT APPLICATION");		/**/
	///////////////////////////////////////////////////

	if(!valid) 	 							{ hd("STAGE:", "ABORTED"); }

	if(!ei.run_headless) {
		if(glfwWindowShouldClose(glfw_W)) 	{ hd("STAGE:",  "CLOSED"); }
		rv("glfwDestroyWindow");
			glfwDestroyWindow(glfw_W); }
	else { rv("Headless Mode Enabled!"); }

	rv("return");
	return 0;
}
