#define VK_USE_PLATFORM_XLIB_KHR
#define VK_API_VERSION_1_2 VK_MAKE_VERSION(1, 2, 0)
#include <cstring>
#include <ctime>
#include <chrono>
#include <cmath>
#include <time.h>
#include <fstream>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <X11/Xlib.h>
#include <vulkan/vulkan.h>

bool 	valid 		=	1;	//	Break on error
bool 	output 		=	1;	//	?
int 	loglevel 	=	1;	//	?

//	Messaging systems designed for terminal width of 96 characters
void vr(const std::string& id, std::vector<VkResult>* reslist, VkResult res) {
//	VkResult output message
	reslist->push_back(res);
	uint32_t 	idx 		= reslist->size() - 1;
	std::string	idx_string 	= std::to_string(idx);
	uint32_t 	idx_sz		= idx_string.size();
	std::string res_string 	= std::to_string(res);
	if(idx_sz < 4) { for(int i = 0; i < 4-idx_sz; i++) { idx_string = " " + idx_string; } }
	if(output && loglevel <= 0) {
		std::cout << "  " << idx_string << ":\t" << (res==0?" ":res_string) << " \t" << id << "\n"; } }

void va(const std::string& id, std::vector<VkResult>* reslist, auto v, VkResult res) {
//	VkResult output message
	reslist->push_back(res);
	uint32_t 	idx 		= reslist->size() - 1;
	std::string	idx_string 	= std::to_string(idx);
	uint32_t 	idx_sz		= idx_string.size();
	std::string res_string 	= std::to_string(res);
	if(idx_sz < 4) { for(int i = 0; i < 4-idx_sz; i++) { idx_string = " " + idx_string; } }
	int 		padlen	= 4;
	int 		pads	= 11;
	std::string pad 	= " ";
	int 		padsize = (pads*padlen - id.size()) - 3;
	for(int i = 0; i < padsize; i++) { pad = pad + " "; }
	if(output && loglevel <= 0) {
		std::cout << "  " << idx_string << ":\t" << (res==0?" ":res_string) << " \t" << id << pad << " [" << v << "]\n"; } }

void ov(const std::string& id, auto v) {
//	Single info output message
	int 		padlen	= 4;
	int 		pads	= 10;
	std::string pad 	= " ";
	int 		padsize = (pads*padlen - id.size()) - 3;
	for(int i = 0; i < padsize; i++) { pad = pad + "."; }
	if(output && loglevel <= 1) {
		std::cout << "\tinfo:\t    " << id << pad << " [" << v << "]\n"; } }

void iv(const std::string& id, auto ov, int idx) {
//	Multiple info output message
	int 		padlen 	= 4;
	int 		pads 	= 9;
	std::string pad 	= " ";
	int 		padsize = (pads*padlen - id.size()) - 3;
	for(int i = 0; i < padsize; i++) { pad = pad + "."; }
	if(output && loglevel <= 0) {
		std::cout << "\tinfo:\t    " << idx << "\t" << id << pad << " [" << ov << "]\n"; } }

void rv(const std::string& id) {
//	Return void output message
	if(output && loglevel <= 0) {
		std::cout << "  void: \t" << id	<< "\n"; } }

void hd(const std::string& id, const std::string& msg) {
//	Header output message
	std::string bar = "";
	for(int i = 0; i < 20; i++) { bar = bar + "____"; }
	if(output && loglevel <= 0) {
		std::cout << bar << "\n " << id << "\t" << msg << "\n"; } }

void nf(auto *Vk_obj) {
//	NullFlags shorthand
	Vk_obj->pNext = NULL;
	Vk_obj->flags = 0; }

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
	if(output) {
		std::cout << "\n\n" << bar << "\n " << msg_fmt << "\n" 	<< bar << "\n\n"; }
	valid = 0;
	return VK_FALSE; }

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
	ShaderCodeInfo rfsc_info[1];
		rfsc_info[0].shaderFilename		= filename;
		rfsc_info[0].shaderData			= buffer;
		rfsc_info[0].shaderBytes		= buffer.size();
		rfsc_info[0].shaderBytesValid	= (rfsc_info[0].shaderBytes%4==0?1:0);
	return rfsc_info[0]; }

struct UniBuf {
	uint32_t wsize;
	uint32_t frame;
	uint32_t minfo; };

struct WSize {
	uint32_t 	app_w;
	uint32_t 	app_h;
	uint32_t 	divs;
	uint32_t 	unused; };
uint32_t wsize_pack(WSize ws) {
	uint32_t packed_ui32 	= ( (uint32_t)ws.app_w)
							+ ( (uint32_t)ws.app_h 	<< 12 )
							+ ( (uint32_t)ws.divs 	<< 24 )
							+ ( (uint32_t)ws.unused << 28 );
	return packed_ui32; }

struct MInfo {
	uint32_t 	mouse_x;
	uint32_t 	mouse_y;
	uint32_t 	mouse_c;
	uint32_t 	unused; };
uint32_t minfo_pack(MInfo mi) {
	uint32_t packed_ui32 	= ( (uint32_t)mi.mouse_c)
							+ ( (uint32_t)mi.mouse_x << 4  )
							+ ( (uint32_t)mi.mouse_y << 16 )
							+ ( (uint32_t)mi.unused  << 28 );
	return packed_ui32; }

int main(void) {

//	Set rand() seed
	srand(time(0));

//	Timestamp for file handling
	std::string timestamp = std::to_string(time(0));

	if(loglevel != 0) { loglevel = loglevel * -1; }

//	Make local backup: Fragment Shader (automata)
	std::string fbk_auto 	= "res/frag/frag_automata0000.frag";
	std::string cp_auto		= "cp '" + fbk_auto + "' 'fbk/auto_" + timestamp +".frag'";
	system(cp_auto.c_str());

//	Make local backup: Render Engine
	std::string fbk_engi 	= "VulkanAutomata.cpp";
	std::string cp_engi		= "cp '" + fbk_engi + "' 'vbk/engi_" + timestamp +".cpp.bk'";
	system(cp_engi.c_str());

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "APPLICATION CONFIG");		/**/
	///////////////////////////////////////////////////

	const uint32_t 	APP_W 			= 512;		//	1920 1536 1280	768	512	384	256
	const uint32_t 	APP_H 			= 288;		//	1080 864  720	432	288	216	144
	const long 		FPS 			= 300;		//	2+
	const int 		TEST_CYCLES 	= 0;		//	0+

	uint32_t 		PD_IDX 			= UINT32_MAX;	//	Physical Device Index
	uint32_t 		GQF_IDX 		= UINT32_MAX;	//	Graphics Queue Family Index
	uint32_t		SURF_FMT 		= UINT32_MAX;	//	Surface Format

	const long 		NS_DELAY 		= 1000000000 / FPS;								//	Nanosecond Delay
	const float 	TRIQUAD_SCALE 	= 1.0;											//	Vertex Shader Triangle Scale
	const float 	VP_SCALE 		= TRIQUAD_SCALE + (1.0-TRIQUAD_SCALE) * 0.5;	//	Vertex Shader Viewport Scale

	const uint32_t 	VERT_FLS 		= 1;	//	Number of Vertex Shader Files
	const uint32_t 	FRAG_FLS 		= 1;	//	Number of Fragment Shader Files
	const uint32_t 	INST_EXS 		= 3;	//	Number of Vulkan Instance Extensions
	const uint32_t 	LDEV_EXS 		= 1;	//	Number of Vulkan Logical Device Extensions
	const uint32_t 	VLID_LRS 		= 1;	//	Number of Vulkan Validation Layers

//	Paths to shader files and extension names
	const char* 	filepath_vert		[VERT_FLS] =
		{	"./app/vert_TriQuad.spv" 					};
	const char* 	filepath_frag		[FRAG_FLS] =
		{	"./app/frag_automata0000.spv"				};
	const char* 	instance_extensions	[INST_EXS] =
		{	"VK_KHR_surface", 
			"VK_KHR_xlib_surface", 
			"VK_EXT_debug_utils"						};
	const char* 	validation_layers	[VLID_LRS] =
		{	"VK_LAYER_KHRONOS_validation" 				};
	const char* 	device_extensions	[LDEV_EXS] =
		{	"VK_KHR_swapchain"							};

//	Config Notification Messages
	ov("Window Width", 			APP_W		);
	ov("Window Height", 		APP_H		);
	ov("Render Cycles", 		TEST_CYCLES	);
	ov("FPS Target", 			FPS			);
	ov("Vertex Shaders", 		VERT_FLS	);
	ov("Fragment Shaders", 		FRAG_FLS	);

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "VULKAN INIT");			/**/
	///////////////////////////////////////////////////

//	VkResult storage
	std::vector<VkResult> vkres;
	vr("init", &vkres, VK_ERROR_UNKNOWN);

	VkApplicationInfo vka_info[1];
		vka_info[0].sType	= VK_STRUCTURE_TYPE_APPLICATION_INFO;
		vka_info[0].pNext	= NULL;
		vka_info[0].pApplicationName	= "VulkanAutomata";
		vka_info[0].applicationVersion	= 0;
		vka_info[0].pEngineName			= NULL;
		vka_info[0].engineVersion		= 0;
		vka_info[0].apiVersion			= VK_API_VERSION_1_2;

	VkInstanceCreateInfo vki_info[1];
		vki_info[0].sType 	= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	nf(&vki_info[0]);
		vki_info[0].pApplicationInfo 			= &vka_info[0];
		vki_info[0].enabledLayerCount 			= VLID_LRS;
		vki_info[0].ppEnabledLayerNames 		= validation_layers;
		vki_info[0].enabledExtensionCount 		= INST_EXS;
		vki_info[0].ppEnabledExtensionNames		= instance_extensions;

	VkInstance vki[1];
	vr("vkCreateInstance", &vkres, 
		vkCreateInstance(&vki_info[0], NULL, &vki[0]) );

	VkDebugUtilsMessengerCreateInfoEXT vkdum_info[1];
		vkdum_info[0].sType	= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	nf(&vkdum_info[0]);
		vkdum_info[0].messageSeverity	= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
										| VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT 
										| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT 
										| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		vkdum_info[0].messageType		= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
										| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		vkdum_info[0].pfnUserCallback	= debugCallback;

	VkDebugUtilsMessengerEXT vkdum[1];
	rv("vkGetInstanceProcAddr");
	auto vkgipa_vkcdum = (PFN_vkCreateDebugUtilsMessengerEXT) 
		vkGetInstanceProcAddr( vki[0], "vkCreateDebugUtilsMessengerEXT" );
	vr("vkCreateDebugUtilsMessengerEXT", &vkres, 
		vkgipa_vkcdum(vki[0], &vkdum_info[0], NULL, &vkdum[0]) );

	uint32_t pd_cnt = UINT32_MAX;
	vr("vkEnumeratePhysicalDevices", &vkres, 
		vkEnumeratePhysicalDevices(vki[0], &pd_cnt, NULL) );
		ov("PhysicalDevices", pd_cnt);
	VkPhysicalDevice vkpd[pd_cnt];
	vr("vkEnumeratePhysicalDevices", &vkres, 
		vkEnumeratePhysicalDevices(vki[0], &pd_cnt, vkpd) );

	VkPhysicalDeviceProperties vkpd_props[pd_cnt];
	for(int i = 0; i < pd_cnt; i++) {
		rv("vkGetPhysicalDeviceProperties");
			vkGetPhysicalDeviceProperties(vkpd[i], &vkpd_props[i]);
		if(	PD_IDX == UINT32_MAX 
		&&	vkpd_props[i].deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ) {
			PD_IDX = i; } }

	uint32_t pdqfp_cnt = UINT32_MAX;
	rv("vkGetPhysicalDeviceQueueFamilyProperties");
		vkGetPhysicalDeviceQueueFamilyProperties(vkpd[PD_IDX], &pdqfp_cnt, NULL);
		ov("Queue Families", pdqfp_cnt);
	VkQueueFamilyProperties vkqfamprops[pdqfp_cnt];
	rv("vkGetPhysicalDeviceQueueFamilyProperties");
		vkGetPhysicalDeviceQueueFamilyProperties(vkpd[PD_IDX], &pdqfp_cnt, vkqfamprops);
	
	for(int i = 0; i < pdqfp_cnt; i++) {
		if(	GQF_IDX == UINT32_MAX 
		&&	vkqfamprops[i].queueFlags & VK_QUEUE_GRAPHICS_BIT ) {
			GQF_IDX = i; } }

	uint32_t gfxq_cnt = vkqfamprops[GQF_IDX].queueCount;
	float queue_priorities[gfxq_cnt];
	for(int i = 0; i < gfxq_cnt; i++) { queue_priorities[i] = 0.0f; }

	iv("Queues", 		gfxq_cnt, GQF_IDX							);
	iv("Queue Flags", 	vkqfamprops[GQF_IDX].queueFlags, GQF_IDX	);

	VkDeviceQueueCreateInfo vkdq_info[1];
		vkdq_info[GQF_IDX].sType	= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	nf(&vkdq_info[GQF_IDX]);
		vkdq_info[GQF_IDX].queueFamilyIndex		= GQF_IDX;
		vkdq_info[GQF_IDX].queueCount			= gfxq_cnt;
		vkdq_info[GQF_IDX].pQueuePriorities		= queue_priorities; 

	VkDeviceCreateInfo vkld_info[1];
		vkld_info[0].sType 	= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	nf(&vkld_info[0]);
		vkld_info[0].queueCreateInfoCount 		= 1;
		vkld_info[0].pQueueCreateInfos 			= vkdq_info;
		vkld_info[0].enabledLayerCount 			= 0;
		vkld_info[0].ppEnabledLayerNames 		= NULL;
		vkld_info[0].enabledExtensionCount 		= 1;
		vkld_info[0].ppEnabledExtensionNames 	= device_extensions;
		vkld_info[0].pEnabledFeatures 			= NULL;

	VkDevice vkld[1];
	vr("vkCreateDevice", &vkres, 
		vkCreateDevice(vkpd[PD_IDX], &vkld_info[0], NULL, &vkld[0]) );
		ov("VkDevice vkld[0]", vkld[0]);

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "DISPLAY");				/**/
	///////////////////////////////////////////////////

	rv("XOpenDisplay");
	Display *d = 
		XOpenDisplay(NULL);
		ov("DisplayString", DisplayString(d));

	XSetWindowAttributes xswa;
	xswa.event_mask = ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask;

	Window rw = XDefaultRootWindow(d);
	rv("XCreateWindow");
	Window w = 
		XCreateWindow ( d, rw, 0, 0, APP_W, APP_H, 0, 
						CopyFromParent, CopyFromParent, CopyFromParent, 0, &xswa );
		ov("Window", w);

	XStoreName( d, w, vka_info[0].pApplicationName );

	rv("XMapWindow");
		XMapWindow(d, w);

	rv("XFlush");
		XFlush(d);

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "SURFACE");				/**/
	///////////////////////////////////////////////////

	VkXlibSurfaceCreateInfoKHR vkxls_info[1];
		vkxls_info[0].sType 	= VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	nf(&vkxls_info[0]);
		vkxls_info[0].dpy 		= d;
		vkxls_info[0].window	= w;

	VkSurfaceKHR vksurf[1];
	vr("vkCreateXlibSurfaceKHR", &vkres, 
		vkCreateXlibSurfaceKHR(vki[0], &vkxls_info[0], NULL, &vksurf[0]) );

	VkSurfaceCapabilitiesKHR vksurf_ables[1];
	vr("vkGetPhysicalDeviceSurfaceCapabilitiesKHR", &vkres, 
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkpd[PD_IDX], vksurf[0], &vksurf_ables[0]) );
		ov("minImageCount", 		vksurf_ables[0].minImageCount			);
		ov("maxImageCount", 		vksurf_ables[0].maxImageCount			);
		ov("currentExtent.width", 	vksurf_ables[0].currentExtent.width		);
		ov("currentExtent.height", 	vksurf_ables[0].currentExtent.height	);

	uint32_t pd_surf_fmt_cnt = UINT32_MAX;
	vr("vkGetPhysicalDeviceSurfaceFormatsKHR", &vkres, 
		vkGetPhysicalDeviceSurfaceFormatsKHR(vkpd[PD_IDX], vksurf[0], 
			&pd_surf_fmt_cnt, NULL) );
		ov("PhysicalDeviceSurfaceFormats", pd_surf_fmt_cnt);
	VkSurfaceFormatKHR vksurf_fmt[pd_surf_fmt_cnt];
	vr("vkGetPhysicalDeviceSurfaceFormatsKHR", &vkres, 
		vkGetPhysicalDeviceSurfaceFormatsKHR(vkpd[PD_IDX], vksurf[0], 
			&pd_surf_fmt_cnt, vksurf_fmt) );
		for(int i = 0; i < pd_surf_fmt_cnt; i++) { 
			iv("VkFormat", 			vksurf_fmt[i].format, 		i); 
			iv("VkColorSpaceKHR", 	vksurf_fmt[i].colorSpace, 	i);  }

	for(int i = 0; i < pd_surf_fmt_cnt; i++) {
		if(	SURF_FMT == UINT32_MAX 
		&&	vksurf_fmt[i].colorSpace 	== VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
		&&	vksurf_fmt[i].format 		== VK_FORMAT_B8G8R8A8_UNORM 			) {
			SURF_FMT = i; } }

	uint32_t pd_surf_presmode_cnt = UINT32_MAX;
	vr("vkGetPhysicalDeviceSurfacePresentModesKHR", &vkres, 
		vkGetPhysicalDeviceSurfacePresentModesKHR(	vkpd[PD_IDX], vksurf[0],
													&pd_surf_presmode_cnt, NULL			) );
		ov("PhysicalDeviceSurfacePresentModes", pd_surf_fmt_cnt);
	VkPresentModeKHR vkpresmode[pd_surf_presmode_cnt];
	vr("vkGetPhysicalDeviceSurfacePresentModesKHR", &vkres, 
		vkGetPhysicalDeviceSurfacePresentModesKHR(	vkpd[PD_IDX], vksurf[0],
													&pd_surf_presmode_cnt, vkpresmode	) );
		for(int i = 0; i < pd_surf_presmode_cnt; i++) { 
			iv("VkPresentModeKHR", 	vkpresmode[i], i);  }

	VkBool32 pd_surf_supported[1];
	vr("vkGetPhysicalDeviceSurfaceSupportKHR", &vkres, 
		vkGetPhysicalDeviceSurfaceSupportKHR(	vkpd[PD_IDX], GQF_IDX, vksurf[0], 
												&pd_surf_supported[0] 				) );
		ov("Surface Supported", (pd_surf_supported[0]?"TRUE":"FALSE"));

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "SHADERS");				/**/
	///////////////////////////////////////////////////

	ShaderCodeInfo shader_info_vert[VERT_FLS];
	for(int i = 0; i < VERT_FLS; i++) {
		rv("getShaderCodeInfo");
		shader_info_vert[i] = getShaderCodeInfo(filepath_vert[i]); }

	VkShaderModuleCreateInfo vkshademod_vert_info[VERT_FLS];
	for(int i = 0; i < VERT_FLS; i++) {
		vkshademod_vert_info[i].sType		= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	nf(&vkshademod_vert_info[i]);
		vkshademod_vert_info[i].codeSize	= shader_info_vert[i].shaderBytes;
		vkshademod_vert_info[i].pCode		= 
			reinterpret_cast<const uint32_t*>(shader_info_vert[i].shaderData.data());
		iv("Vertex shaderFilename", 	 shader_info_vert[i].shaderFilename, 					i);
		iv("Vertex shaderBytes", 		 shader_info_vert[i].shaderBytes, 						i);
		iv("Vertex shaderBytesValid", 	(shader_info_vert[i].shaderBytesValid?"TRUE":"FALSE"), 	i); }

	ShaderCodeInfo shader_info_frag[FRAG_FLS];
	for(int i = 0; i < FRAG_FLS; i++) {
		rv("getShaderCodeInfo");
		shader_info_frag[i] = getShaderCodeInfo(filepath_frag[i]); }

	VkShaderModuleCreateInfo vkshademod_frag_info[FRAG_FLS];
	for(int i = 0; i < FRAG_FLS; i++) {
		vkshademod_frag_info[i].sType		= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	nf(&vkshademod_frag_info[i]);
		vkshademod_frag_info[i].codeSize	= shader_info_frag[i].shaderBytes;
		vkshademod_frag_info[i].pCode		= 
			reinterpret_cast<const uint32_t*>(shader_info_frag[i].shaderData.data());
		iv("Fragment shaderFilename", 	 shader_info_frag[i].shaderFilename, 					i);
		iv("Fragment shaderBytes", 		 shader_info_frag[i].shaderBytes, 						i);
		iv("Fragment shaderBytesValid", (shader_info_frag[i].shaderBytesValid?"TRUE":"FALSE"), 	i); }

	VkShaderModule vkshademod_vert[VERT_FLS];
	for(int i = 0; i < VERT_FLS; i++) {
		va("vkCreateShaderModule", &vkres, vkshademod_vert[i],
			vkCreateShaderModule(vkld[0], &vkshademod_vert_info[i], NULL, &vkshademod_vert[i]) ); }

	VkShaderModule vkshademod_frag[FRAG_FLS];
	for(int i = 0; i < FRAG_FLS; i++) {
		va("vkCreateShaderModule", &vkres, vkshademod_frag[i],
			vkCreateShaderModule(vkld[0], &vkshademod_frag_info[i], NULL, &vkshademod_frag[i]) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "SWAPCHAIN");				/**/
	///////////////////////////////////////////////////

	VkSwapchainCreateInfoKHR vkswap_info[1];
		vkswap_info[0].sType	= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	nf(&vkswap_info[0]);
		vkswap_info[0].surface					= vksurf[0];
		vkswap_info[0].minImageCount			= vksurf_ables[0].minImageCount;
		vkswap_info[0].imageFormat				= vksurf_fmt[SURF_FMT].format;
		vkswap_info[0].imageColorSpace			= vksurf_fmt[SURF_FMT].colorSpace;
		vkswap_info[0].imageExtent				= vksurf_ables[0].currentExtent;
		vkswap_info[0].imageArrayLayers			= 1;
		vkswap_info[0].imageUsage				= vksurf_ables[0].supportedUsageFlags;
		vkswap_info[0].imageSharingMode			= VK_SHARING_MODE_EXCLUSIVE;
		vkswap_info[0].queueFamilyIndexCount	= 1;
		vkswap_info[0].pQueueFamilyIndices		= &GQF_IDX;
		vkswap_info[0].preTransform				= vksurf_ables[0].currentTransform;
		vkswap_info[0].compositeAlpha			= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		vkswap_info[0].presentMode				= VK_PRESENT_MODE_IMMEDIATE_KHR;
		vkswap_info[0].clipped					= VK_FALSE;
		vkswap_info[0].oldSwapchain				= VK_NULL_HANDLE;

	VkSwapchainKHR vkswap[1];
	va("vkCreateSwapchainKHR", &vkres, vkswap[0],
		vkCreateSwapchainKHR(vkld[0], &vkswap_info[0], NULL, &vkswap[0]) );

	uint32_t swap_img_cnt = UINT32_MAX;
	vr("vkGetSwapchainImagesKHR", &vkres, 
		vkGetSwapchainImagesKHR(vkld[0], vkswap[0], &swap_img_cnt, NULL) );
		ov("SwapchainImages", swap_img_cnt);

	VkImage vkswap_img[swap_img_cnt];
	vr("vkGetSwapchainImagesKHR", &vkres, 
		vkGetSwapchainImagesKHR(vkld[0], vkswap[0], &swap_img_cnt, vkswap_img) );
	ov("vkGetSwapchainImagesKHR", vkswap_img[0]);
	ov("vkGetSwapchainImagesKHR", vkswap_img[1]);

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "IMAGE LAYERS");			/**/
	///////////////////////////////////////////////////

	VkExtent3D vkext3d_work;
		vkext3d_work.width 	= vksurf_ables[0].currentExtent.width;
		vkext3d_work.height = vksurf_ables[0].currentExtent.height;
		vkext3d_work.depth 	= 1;

	VkImageCreateInfo vkimg_info_work;
		vkimg_info_work.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	nf(&vkimg_info_work);
		vkimg_info_work.imageType 				= VK_IMAGE_TYPE_2D;
		vkimg_info_work.format 					= VK_FORMAT_R16G16B16A16_UNORM;
		vkimg_info_work.extent 					= vkext3d_work;
		vkimg_info_work.mipLevels 				= 1;
		vkimg_info_work.arrayLayers 			= 1;
		vkimg_info_work.samples 				= VK_SAMPLE_COUNT_1_BIT;
		vkimg_info_work.tiling 					= VK_IMAGE_TILING_OPTIMAL;
		vkimg_info_work.usage 					= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
												| VK_IMAGE_USAGE_SAMPLED_BIT
												| VK_IMAGE_USAGE_TRANSFER_SRC_BIT
												| VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		vkimg_info_work.sharingMode 			= VK_SHARING_MODE_EXCLUSIVE;
		vkimg_info_work.queueFamilyIndexCount 	= 0;
		vkimg_info_work.pQueueFamilyIndices 	= NULL;
		vkimg_info_work.initialLayout 			= VK_IMAGE_LAYOUT_UNDEFINED;

	VkImage vkimg_work[2];
	va("vkCreateImage", &vkres, vkimg_work[0],
		vkCreateImage(vkld[0], &vkimg_info_work, NULL, &vkimg_work[0]) );
	va("vkCreateImage", &vkres, vkimg_work[1],
		vkCreateImage(vkld[0], &vkimg_info_work, NULL, &vkimg_work[1]) );

	VkPhysicalDeviceMemoryProperties vkpd_memprops;
	rv("vkGetPhysicalDeviceMemoryProperties");
		vkGetPhysicalDeviceMemoryProperties(vkpd[PD_IDX], &vkpd_memprops);

	ov("memoryTypeCount", 	vkpd_memprops.memoryTypeCount					);
	for(int i = 0; i < 		vkpd_memprops.memoryTypeCount; 				i++	) {
		iv("propertyFlags", vkpd_memprops.memoryTypes[i].propertyFlags,	i	); 
		iv("heapIndex", 	vkpd_memprops.memoryTypes[i].heapIndex, 	i	); }

	ov("memoryHeapCount", 	vkpd_memprops.memoryHeapCount				);
	for(int i = 0; i < 		vkpd_memprops.memoryHeapCount; 				i++	) {
		iv("size", 			vkpd_memprops.memoryHeaps[i].size,			i	); 
		iv("flags", 		vkpd_memprops.memoryHeaps[i].flags,			i	); }

	int mem_index[2];
	VkMemoryRequirements 	vkmemreqs[2];
	rv("vkGetImageMemoryRequirements");
		vkGetImageMemoryRequirements(vkld[0], vkimg_work[0], &vkmemreqs[0]);
		iv("memreq size", 			vkmemreqs[0].size, 0);
		iv("memreq alignment", 		vkmemreqs[0].alignment, 0);
		iv("memreq memoryTypeBits", vkmemreqs[0].memoryTypeBits, 0);
		mem_index[0] = findProperties( &vkpd_memprops, vkmemreqs[0].memoryTypeBits, 0x00000001 );
		iv("memoryTypeIndex", mem_index[0], 0);

	rv("vkGetImageMemoryRequirements");
		vkGetImageMemoryRequirements(vkld[0], vkimg_work[1], &vkmemreqs[1]);
		iv("memreq size", 			vkmemreqs[1].size, 1);
		iv("memreq alignment", 		vkmemreqs[1].alignment, 1);
		iv("memreq memoryTypeBits", vkmemreqs[1].memoryTypeBits, 1);
		mem_index[1] = findProperties( &vkpd_memprops, vkmemreqs[1].memoryTypeBits, 0x00000001 );
		iv("memoryTypeIndex", mem_index[1], 1);

	VkMemoryAllocateInfo vkmemalloc_info[2];
		vkmemalloc_info[0].sType			= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		vkmemalloc_info[0].pNext			= NULL;
		vkmemalloc_info[0].allocationSize	= vkmemreqs[0].size;
		vkmemalloc_info[0].memoryTypeIndex	= mem_index[0];
		vkmemalloc_info[1].sType			= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		vkmemalloc_info[1].pNext			= NULL;
		vkmemalloc_info[1].allocationSize	= vkmemreqs[1].size;
		vkmemalloc_info[1].memoryTypeIndex	= mem_index[1];

	VkDeviceMemory vkdevmemo[2];
	va("vkAllocateMemory", &vkres, vkdevmemo[0],
		vkAllocateMemory(vkld[0], &vkmemalloc_info[0], NULL, &vkdevmemo[0]) );
	va("vkAllocateMemory", &vkres, vkdevmemo[1],
		vkAllocateMemory(vkld[0], &vkmemalloc_info[1], NULL, &vkdevmemo[1]) );

	vr("vkBindImageMemory", &vkres, 
		vkBindImageMemory(vkld[0], vkimg_work[0], vkdevmemo[0], 0) );
	vr("vkBindImageMemory", &vkres, 
		vkBindImageMemory(vkld[0], vkimg_work[1], vkdevmemo[1], 0) );
	
	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "SCREENSHOT");				/**/
	///////////////////////////////////////////////////

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "COMMAND BUFFERS");		/**/
	///////////////////////////////////////////////////

	VkCommandPoolCreateInfo vkcompool_info[1];
		vkcompool_info[0].sType	= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	nf(&vkcompool_info[0]);
		vkcompool_info[0].queueFamilyIndex	= GQF_IDX;

	VkCommandPool vkcompool[1];
	va("vkCreateCommandPool", &vkres, vkcompool[0],
		vkCreateCommandPool(vkld[0], &vkcompool_info[0], NULL, &vkcompool[0]) );

	VkCommandBufferAllocateInfo vkcombuf_alloc_info[1];
		vkcombuf_alloc_info[0].sType	= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		vkcombuf_alloc_info[0].pNext	= NULL;
		vkcombuf_alloc_info[0].commandPool			= vkcompool[0];
		vkcombuf_alloc_info[0].level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		vkcombuf_alloc_info[0].commandBufferCount	= swap_img_cnt;

	VkCommandBuffer vkcombuf_work_init[2];
	for(int i = 0; i < 2; i++) {
		vr("vkAllocateCommandBuffers", &vkres, 
			vkAllocateCommandBuffers(vkld[0], &vkcombuf_alloc_info[0], vkcombuf_work_init) ); }

	VkCommandBuffer vkcombuf_work[2];
	for(int i = 0; i < 2; i++) {
		vr("vkAllocateCommandBuffers", &vkres, 
			vkAllocateCommandBuffers(vkld[0], &vkcombuf_alloc_info[0], vkcombuf_work) ); }

	VkCommandBuffer vkcombuf_pres_init[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkAllocateCommandBuffers", &vkres, 
			vkAllocateCommandBuffers(vkld[0], &vkcombuf_alloc_info[0], vkcombuf_pres_init) ); }

	VkCommandBuffer vkcombuf_pres[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkAllocateCommandBuffers", &vkres, 
			vkAllocateCommandBuffers(vkld[0], &vkcombuf_alloc_info[0], vkcombuf_pres) ); }

	VkCommandBufferBeginInfo vkcombufbegin_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
			vkcombufbegin_info[i].sType	= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		nf(&vkcombufbegin_info[i]);
			vkcombufbegin_info[i].pInheritanceInfo	= NULL; }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "SYNC");					/**/
	///////////////////////////////////////////////////

	VkQueue vkq[1];
	rv("vkGetDeviceQueue");
		vkGetDeviceQueue(vkld[0], GQF_IDX, 0, &vkq[0]);

	VkSemaphoreCreateInfo vksemaph_info[1];
		vksemaph_info[0].sType	= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	nf(&vksemaph_info[0]);

	VkSemaphore vksemaph_image[1];
	va("vkCreateSemaphore", &vkres, vksemaph_image[0],
		vkCreateSemaphore(vkld[0], &vksemaph_info[0], NULL, &vksemaph_image[0]) );

	VkSemaphore vksemaph_rendr[1];
	va("vkCreateSemaphore", &vkres, vksemaph_rendr[0],
		vkCreateSemaphore(vkld[0], &vksemaph_info[0], NULL, &vksemaph_rendr[0]) );

	VkFenceCreateInfo vkfence_info[1];
		vkfence_info[0].sType	= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	nf(&vkfence_info[0]);

	VkFence vkfence_aqimg[1];
	va("vkCreateFence", &vkres, vkfence_aqimg[0],
		vkCreateFence(vkld[0], &vkfence_info[0], NULL, &vkfence_aqimg[0]) );

	VkPipelineStageFlags qsubwait	= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	uint32_t aqimg_idx[1];

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RENDERPASS INIT");		/**/
	///////////////////////////////////////////////////

	VkRect2D vkrect2d;
		vkrect2d.offset.x	= 0;
		vkrect2d.offset.y	= 0;
		vkrect2d.extent		= vksurf_ables[0].currentExtent;

	VkClearValue vkclearval[4];
		vkclearval[0].color 	= { 1.0f, 0.0f, 0.0f, 1.0f };
		vkclearval[1].color 	= { 0.0f, 0.0f, 1.0f, 1.0f };
		vkclearval[2].color 	= { 1.0f, 1.0f, 1.0f, 1.0f };
		vkclearval[3].color 	= { 0.0f, 1.0f, 1.0f, 1.0f };

	VkImageSubresourceRange vkimgsubrange;
		vkimgsubrange.aspectMask 		= VK_IMAGE_ASPECT_COLOR_BIT;
		vkimgsubrange.baseMipLevel 		= 0;
		vkimgsubrange.levelCount 		= 1;
		vkimgsubrange.baseArrayLayer 	= 0;
		vkimgsubrange.layerCount 		= 1;

	VkImageSubresourceLayers vkimgsublayer;
		vkimgsublayer.aspectMask 		= VK_IMAGE_ASPECT_COLOR_BIT;
		vkimgsublayer.mipLevel 			= 0;
		vkimgsublayer.baseArrayLayer 	= 0;
		vkimgsublayer.layerCount 		= 1;

	VkPipelineRasterizationStateCreateInfo vkpiperastinfo;
		vkpiperastinfo.sType	= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	nf(&vkpiperastinfo);
		vkpiperastinfo.depthClampEnable			= VK_FALSE;
		vkpiperastinfo.rasterizerDiscardEnable	= VK_FALSE;
		vkpiperastinfo.polygonMode				= VK_POLYGON_MODE_FILL;
		vkpiperastinfo.cullMode					= VK_CULL_MODE_NONE;
		vkpiperastinfo.frontFace				= VK_FRONT_FACE_CLOCKWISE;
		vkpiperastinfo.depthBiasEnable			= VK_FALSE;
		vkpiperastinfo.depthBiasConstantFactor	= 0.0f;
		vkpiperastinfo.depthBiasClamp			= 0.0f;
		vkpiperastinfo.depthBiasSlopeFactor		= 0.0f;
		vkpiperastinfo.lineWidth				= 1.0f;

	VkViewport vkviewport;
		vkviewport.x			= 0;
		vkviewport.y			= 0;
		vkviewport.width		= vksurf_ables[0].currentExtent.width*VP_SCALE;
		vkviewport.height		= vksurf_ables[0].currentExtent.height*VP_SCALE;
		vkviewport.minDepth		= 0.0f;
		vkviewport.maxDepth		= 1.0f;
	VkPipelineViewportStateCreateInfo vkpipeviewport_info;
		vkpipeviewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	nf(&vkpipeviewport_info);
		vkpipeviewport_info.viewportCount	= 1;
		vkpipeviewport_info.pViewports		= &vkviewport;
		vkpipeviewport_info.scissorCount	= 1;
		vkpipeviewport_info.pScissors		= &vkrect2d;

	VkPipelineMultisampleStateCreateInfo vkpipems_info;
		vkpipems_info.sType	= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	nf(&vkpipems_info);
		vkpipems_info.rasterizationSamples	= VK_SAMPLE_COUNT_1_BIT;
		vkpipems_info.sampleShadingEnable	= VK_FALSE;
		vkpipems_info.minSampleShading		= 0.0f;
		vkpipems_info.pSampleMask			= NULL;
		vkpipems_info.alphaToCoverageEnable	= VK_FALSE;
		vkpipems_info.alphaToOneEnable		= VK_FALSE;

	VkPipelineVertexInputStateCreateInfo vkpipevertinput_info;
		vkpipevertinput_info.sType	= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	nf(&vkpipevertinput_info);
		vkpipevertinput_info.vertexBindingDescriptionCount		= 0;
		vkpipevertinput_info.pVertexBindingDescriptions			= NULL;
		vkpipevertinput_info.vertexAttributeDescriptionCount	= 0;
		vkpipevertinput_info.pVertexAttributeDescriptions		= NULL;

	VkPipelineInputAssemblyStateCreateInfo vkpipeinputass_info;
		vkpipeinputass_info.sType	= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	nf(&vkpipeinputass_info);
		vkpipeinputass_info.topology				= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		vkpipeinputass_info.primitiveRestartEnable	= VK_FALSE;

	VkPipelineColorBlendAttachmentState vkpipecolblendatt;
		vkpipecolblendatt.blendEnable			= VK_FALSE;
		vkpipecolblendatt.srcColorBlendFactor	= VK_BLEND_FACTOR_ZERO;
		vkpipecolblendatt.dstColorBlendFactor	= VK_BLEND_FACTOR_ZERO;
		vkpipecolblendatt.colorBlendOp			= VK_BLEND_OP_ADD;
		vkpipecolblendatt.srcAlphaBlendFactor	= VK_BLEND_FACTOR_ZERO;
		vkpipecolblendatt.dstAlphaBlendFactor	= VK_BLEND_FACTOR_ZERO;
		vkpipecolblendatt.alphaBlendOp			= VK_BLEND_OP_ADD;
		vkpipecolblendatt.colorWriteMask		= 15;

	VkPipelineColorBlendStateCreateInfo vkpipecolblend;
		vkpipecolblend.sType	= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	nf(&vkpipecolblend);
		vkpipecolblend.logicOpEnable		= VK_FALSE;
		vkpipecolblend.logicOp				= VK_LOGIC_OP_NO_OP;
		vkpipecolblend.attachmentCount		= 1;
		vkpipecolblend.pAttachments			= &vkpipecolblendatt;
		vkpipecolblend.blendConstants[0]	= 1.0f;
		vkpipecolblend.blendConstants[1]	= 1.0f;
		vkpipecolblend.blendConstants[2]	= 1.0f;
		vkpipecolblend.blendConstants[3]	= 1.0f;

	VkPipelineShaderStageCreateInfo vkgfxpipe_ss_info[2];
		vkgfxpipe_ss_info[0].sType	= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	nf(&vkgfxpipe_ss_info[0]);
		vkgfxpipe_ss_info[0].stage					= VK_SHADER_STAGE_VERTEX_BIT;
		vkgfxpipe_ss_info[0].module					= vkshademod_vert[0];
		vkgfxpipe_ss_info[0].pName					= "main";
		vkgfxpipe_ss_info[0].pSpecializationInfo	= NULL;
		vkgfxpipe_ss_info[1].sType	= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	nf(&vkgfxpipe_ss_info[1]);
		vkgfxpipe_ss_info[1].stage					= VK_SHADER_STAGE_FRAGMENT_BIT;
		vkgfxpipe_ss_info[1].module					= vkshademod_frag[0];
		vkgfxpipe_ss_info[1].pName					= "main";
		vkgfxpipe_ss_info[1].pSpecializationInfo	= NULL;

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RECORD WORK_INIT");		/**/
	///////////////////////////////////////////////////

	VkAttachmentDescription vkattdesc_work_init[2];
		vkattdesc_work_init[0].flags 			= 0;
		vkattdesc_work_init[0].format 			= vkimg_info_work.format;
		vkattdesc_work_init[0].samples 			= VK_SAMPLE_COUNT_1_BIT;
		vkattdesc_work_init[0].loadOp 			= VK_ATTACHMENT_LOAD_OP_CLEAR;
		vkattdesc_work_init[0].storeOp 			= VK_ATTACHMENT_STORE_OP_STORE;
		vkattdesc_work_init[0].stencilLoadOp 	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkattdesc_work_init[0].stencilStoreOp 	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkattdesc_work_init[0].initialLayout 	= VK_IMAGE_LAYOUT_UNDEFINED;
		vkattdesc_work_init[0].finalLayout 		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vkattdesc_work_init[1].flags 			= 0;
		vkattdesc_work_init[1].format 			= vkimg_info_work.format;
		vkattdesc_work_init[1].samples 			= VK_SAMPLE_COUNT_1_BIT;
		vkattdesc_work_init[1].loadOp 			= VK_ATTACHMENT_LOAD_OP_CLEAR;
		vkattdesc_work_init[1].storeOp 			= VK_ATTACHMENT_STORE_OP_STORE;
		vkattdesc_work_init[1].stencilLoadOp 	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkattdesc_work_init[1].stencilStoreOp 	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkattdesc_work_init[1].initialLayout 	= VK_IMAGE_LAYOUT_UNDEFINED;
		vkattdesc_work_init[1].finalLayout 		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentReference vkattref_work_init;
		vkattref_work_init.attachment 	= 0;
		vkattref_work_init.layout 		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription vksubpassdesc_work_init;
		vksubpassdesc_work_init.flags 						= 0;
		vksubpassdesc_work_init.pipelineBindPoint 			= VK_PIPELINE_BIND_POINT_GRAPHICS;
		vksubpassdesc_work_init.inputAttachmentCount 		= 0;
		vksubpassdesc_work_init.pInputAttachments 			= NULL;
		vksubpassdesc_work_init.colorAttachmentCount 		= 1;
		vksubpassdesc_work_init.pColorAttachments 			= &vkattref_work_init;
		vksubpassdesc_work_init.pResolveAttachments 		= NULL;
		vksubpassdesc_work_init.pDepthStencilAttachment 	= NULL;
		vksubpassdesc_work_init.preserveAttachmentCount 	= 0;
		vksubpassdesc_work_init.pPreserveAttachments 		= NULL;

	VkRenderPassCreateInfo vkrendpass_info_work_init[2];
		vkrendpass_info_work_init[0].sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&vkrendpass_info_work_init[0]);
		vkrendpass_info_work_init[0].attachmentCount 	= 1;
		vkrendpass_info_work_init[0].pAttachments 		= &vkattdesc_work_init[0];
		vkrendpass_info_work_init[0].subpassCount 		= 1;
		vkrendpass_info_work_init[0].pSubpasses 		= &vksubpassdesc_work_init;
		vkrendpass_info_work_init[0].dependencyCount 	= 0;
		vkrendpass_info_work_init[0].pDependencies 		= NULL;
		vkrendpass_info_work_init[1].sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&vkrendpass_info_work_init[1]);
		vkrendpass_info_work_init[1].attachmentCount 	= 1;
		vkrendpass_info_work_init[1].pAttachments 		= &vkattdesc_work_init[1];
		vkrendpass_info_work_init[1].subpassCount 		= 1;
		vkrendpass_info_work_init[1].pSubpasses 		= &vksubpassdesc_work_init;
		vkrendpass_info_work_init[1].dependencyCount 	= 0;
		vkrendpass_info_work_init[1].pDependencies 		= NULL;

	VkRenderPass vkrendpass_work_init[2];
	va("vkCreateRenderPass", &vkres, vkrendpass_work_init[0],
		vkCreateRenderPass(vkld[0], &vkrendpass_info_work_init[0], NULL, &vkrendpass_work_init[0]) );
	va("vkCreateRenderPass", &vkres, vkrendpass_work_init[1],
		vkCreateRenderPass(vkld[0], &vkrendpass_info_work_init[1], NULL, &vkrendpass_work_init[1]) );

	VkImageViewCreateInfo vkimgview_info_work[2];
		vkimgview_info_work[0].sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	nf(&vkimgview_info_work[0]);
		vkimgview_info_work[0].image 				= vkimg_work[0];
		vkimgview_info_work[0].viewType 			= VK_IMAGE_VIEW_TYPE_2D;
		vkimgview_info_work[0].format 				= vkimg_info_work.format;
		vkimgview_info_work[0].components.r			= VK_COMPONENT_SWIZZLE_IDENTITY;
		vkimgview_info_work[0].components.g			= VK_COMPONENT_SWIZZLE_IDENTITY;
		vkimgview_info_work[0].components.b			= VK_COMPONENT_SWIZZLE_IDENTITY;
		vkimgview_info_work[0].components.a			= VK_COMPONENT_SWIZZLE_IDENTITY;
		vkimgview_info_work[0].subresourceRange 	= vkimgsubrange;
		vkimgview_info_work[1].sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	nf(&vkimgview_info_work[1]);
		vkimgview_info_work[1].image 				= vkimg_work[1];
		vkimgview_info_work[1].viewType 			= VK_IMAGE_VIEW_TYPE_2D;
		vkimgview_info_work[1].format 				= vkimg_info_work.format;
		vkimgview_info_work[1].components.r			= VK_COMPONENT_SWIZZLE_IDENTITY;
		vkimgview_info_work[1].components.g			= VK_COMPONENT_SWIZZLE_IDENTITY;
		vkimgview_info_work[1].components.b			= VK_COMPONENT_SWIZZLE_IDENTITY;
		vkimgview_info_work[1].components.a			= VK_COMPONENT_SWIZZLE_IDENTITY;
		vkimgview_info_work[1].subresourceRange 	= vkimgsubrange;

	VkImageView vkimgview_work[2];
	va("vkCreateImageView", &vkres, vkimgview_work[0],
		vkCreateImageView(vkld[0], &vkimgview_info_work[0], NULL, &vkimgview_work[0]) );
	va("vkCreateImageView", &vkres, vkimgview_work[1],
		vkCreateImageView(vkld[0], &vkimgview_info_work[1], NULL, &vkimgview_work[1]) );

	VkFramebufferCreateInfo vkframebuff_info_work_init[2];
		vkframebuff_info_work_init[0].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	nf(&vkframebuff_info_work_init[0]);
		vkframebuff_info_work_init[0].renderPass 		= vkrendpass_work_init[0];
		vkframebuff_info_work_init[0].attachmentCount 	= 1;
		vkframebuff_info_work_init[0].pAttachments 		= &vkimgview_work[0];
		vkframebuff_info_work_init[0].width 			= vksurf_ables[0].currentExtent.width;
		vkframebuff_info_work_init[0].height 			= vksurf_ables[0].currentExtent.height;
		vkframebuff_info_work_init[0].layers 			= 1;
		vkframebuff_info_work_init[1].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	nf(&vkframebuff_info_work_init[1]);
		vkframebuff_info_work_init[1].renderPass 		= vkrendpass_work_init[1];
		vkframebuff_info_work_init[1].attachmentCount 	= 1;
		vkframebuff_info_work_init[1].pAttachments 		= &vkimgview_work[1];
		vkframebuff_info_work_init[1].width 			= vksurf_ables[0].currentExtent.width;
		vkframebuff_info_work_init[1].height 			= vksurf_ables[0].currentExtent.height;
		vkframebuff_info_work_init[1].layers 			= 1;

	VkFramebuffer vkframebuff_work_init[2];
	va("vkCreateFramebuffer", &vkres, vkframebuff_work_init[0],
		vkCreateFramebuffer(vkld[0], &vkframebuff_info_work_init[0], NULL, &vkframebuff_work_init[0]) );
	va("vkCreateFramebuffer", &vkres, vkframebuff_work_init[1],
		vkCreateFramebuffer(vkld[0], &vkframebuff_info_work_init[1], NULL, &vkframebuff_work_init[1]) );

	VkRenderPassBeginInfo vkrpbegininfo_work_init[2];
		vkrpbegininfo_work_init[0].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegininfo_work_init[0].pNext 				= NULL;
		vkrpbegininfo_work_init[0].renderPass 			= vkrendpass_work_init[0];
		vkrpbegininfo_work_init[0].framebuffer 			= vkframebuff_work_init[0];
		vkrpbegininfo_work_init[0].renderArea 			= vkrect2d;
		vkrpbegininfo_work_init[0].clearValueCount 		= 1;
		vkrpbegininfo_work_init[0].pClearValues 		= &vkclearval[2];
		vkrpbegininfo_work_init[1].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegininfo_work_init[1].pNext 				= NULL;
		vkrpbegininfo_work_init[1].renderPass 			= vkrendpass_work_init[1];
		vkrpbegininfo_work_init[1].framebuffer 			= vkframebuff_work_init[1];
		vkrpbegininfo_work_init[1].renderArea 			= vkrect2d;
		vkrpbegininfo_work_init[1].clearValueCount 		= 1;
		vkrpbegininfo_work_init[1].pClearValues 		= &vkclearval[3];

	for(int i = 0; i < 2; i++) {
		vr("vkBeginCommandBuffer", &vkres, 
			vkBeginCommandBuffer(vkcombuf_work_init[i], &vkcombufbegin_info[i]) );

			rv("vkCmdBeginRenderPass");
				vkCmdBeginRenderPass (
					vkcombuf_work_init[i], &vkrpbegininfo_work_init[i], VK_SUBPASS_CONTENTS_INLINE );

			rv("vkCmdEndRenderPass");
				vkCmdEndRenderPass(vkcombuf_work_init[i]);

		vr("vkEndCommandBuffer", &vkres, 
			vkEndCommandBuffer(vkcombuf_work_init[i]) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "SUBMIT WORK_INIT");		/**/
	///////////////////////////////////////////////////

	for(int i = 0; i < 2; i++) {
		if(valid) {
			rv("vkcombuf_work_init");
			VkSubmitInfo vksub_info[1];
				vksub_info[0].sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO;
				vksub_info[0].pNext					= NULL;
				vksub_info[0].waitSemaphoreCount	= 0;
				vksub_info[0].pWaitSemaphores		= NULL;
				vksub_info[0].pWaitDstStageMask		= NULL;
				vksub_info[0].commandBufferCount	= 1;
				vksub_info[0].pCommandBuffers		= &vkcombuf_work_init[i];
				vksub_info[0].signalSemaphoreCount	= 0;
				vksub_info[0].pSignalSemaphores		= NULL;
			vr("vkQueueSubmit", &vkres, 
				vkQueueSubmit(vkq[0], 1, vksub_info, VK_NULL_HANDLE) ); } }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RECORD WORK_LOOP");		/**/
	///////////////////////////////////////////////////

	VkAttachmentDescription vkattdesc_work[2];
		vkattdesc_work[0].flags 			= 0;
		vkattdesc_work[0].format 			= vkimg_info_work.format;
		vkattdesc_work[0].samples 			= VK_SAMPLE_COUNT_1_BIT;
		vkattdesc_work[0].loadOp 			= VK_ATTACHMENT_LOAD_OP_LOAD;
		vkattdesc_work[0].storeOp 			= VK_ATTACHMENT_STORE_OP_STORE;
		vkattdesc_work[0].stencilLoadOp 	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkattdesc_work[0].stencilStoreOp 	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkattdesc_work[0].initialLayout 	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vkattdesc_work[0].finalLayout 		= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vkattdesc_work[1].flags 			= 0;
		vkattdesc_work[1].format 			= vkimg_info_work.format;
		vkattdesc_work[1].samples 			= VK_SAMPLE_COUNT_1_BIT;
		vkattdesc_work[1].loadOp 			= VK_ATTACHMENT_LOAD_OP_LOAD;
		vkattdesc_work[1].storeOp 			= VK_ATTACHMENT_STORE_OP_STORE;
		vkattdesc_work[1].stencilLoadOp 	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkattdesc_work[1].stencilStoreOp 	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkattdesc_work[1].initialLayout 	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vkattdesc_work[1].finalLayout 		= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

	VkAttachmentReference vkattref_work;
		vkattref_work.attachment 	= 0;
		vkattref_work.layout 		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription vksubpassdesc_work;
		vksubpassdesc_work.flags 						= 0;
		vksubpassdesc_work.pipelineBindPoint 			= VK_PIPELINE_BIND_POINT_GRAPHICS;
		vksubpassdesc_work.inputAttachmentCount 		= 0;
		vksubpassdesc_work.pInputAttachments 			= NULL;
		vksubpassdesc_work.colorAttachmentCount 		= 1;
		vksubpassdesc_work.pColorAttachments 			= &vkattref_work;
		vksubpassdesc_work.pResolveAttachments 			= NULL;
		vksubpassdesc_work.pDepthStencilAttachment 		= NULL;
		vksubpassdesc_work.preserveAttachmentCount 		= 0;
		vksubpassdesc_work.pPreserveAttachments 		= NULL;

	VkRenderPassCreateInfo vkrendpass_info_work[2];
		vkrendpass_info_work[0].sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&vkrendpass_info_work[0]);
		vkrendpass_info_work[0].attachmentCount 	= 1;
		vkrendpass_info_work[0].pAttachments 		= &vkattdesc_work[0];
		vkrendpass_info_work[0].subpassCount 		= 1;
		vkrendpass_info_work[0].pSubpasses 			= &vksubpassdesc_work;
		vkrendpass_info_work[0].dependencyCount 	= 0;
		vkrendpass_info_work[0].pDependencies 		= NULL;
		vkrendpass_info_work[1].sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&vkrendpass_info_work[1]);
		vkrendpass_info_work[1].attachmentCount 	= 1;
		vkrendpass_info_work[1].pAttachments 		= &vkattdesc_work[1];
		vkrendpass_info_work[1].subpassCount 		= 1;
		vkrendpass_info_work[1].pSubpasses 			= &vksubpassdesc_work;
		vkrendpass_info_work[1].dependencyCount 	= 0;
		vkrendpass_info_work[1].pDependencies 		= NULL;

	VkRenderPass vkrendpass_work[2];
	va("vkCreateRenderPass", &vkres, vkrendpass_work[0],
		vkCreateRenderPass(vkld[0], &vkrendpass_info_work[0], NULL, &vkrendpass_work[0]) );
	va("vkCreateRenderPass", &vkres, vkrendpass_work[1],
		vkCreateRenderPass(vkld[0], &vkrendpass_info_work[1], NULL, &vkrendpass_work[1]) );

	VkFramebufferCreateInfo vkframebuff_info_work[2];
		vkframebuff_info_work[0].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	nf(&vkframebuff_info_work[0]);
		vkframebuff_info_work[0].renderPass 		= vkrendpass_work[0];
		vkframebuff_info_work[0].attachmentCount 	= 1;
		vkframebuff_info_work[0].pAttachments 		= &vkimgview_work[0];
		vkframebuff_info_work[0].width 				= vksurf_ables[0].currentExtent.width;
		vkframebuff_info_work[0].height 			= vksurf_ables[0].currentExtent.height;
		vkframebuff_info_work[0].layers 			= 1;
		vkframebuff_info_work[1].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	nf(&vkframebuff_info_work[1]);
		vkframebuff_info_work[1].renderPass 		= vkrendpass_work[1];
		vkframebuff_info_work[1].attachmentCount 	= 1;
		vkframebuff_info_work[1].pAttachments 		= &vkimgview_work[1];
		vkframebuff_info_work[1].width 				= vksurf_ables[0].currentExtent.width;
		vkframebuff_info_work[1].height 			= vksurf_ables[0].currentExtent.height;
		vkframebuff_info_work[1].layers 			= 1;

	VkFramebuffer vkframebuff_work[2];
	va("vkCreateFramebuffer", &vkres, vkframebuff_work[0],
		vkCreateFramebuffer(vkld[0], &vkframebuff_info_work[0], NULL, &vkframebuff_work[0]) );
	va("vkCreateFramebuffer", &vkres, vkframebuff_work[1],
		vkCreateFramebuffer(vkld[0], &vkframebuff_info_work[1], NULL, &vkframebuff_work[1]) );

	VkRenderPassBeginInfo vkrpbegininfo_work[2];
		vkrpbegininfo_work[0].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegininfo_work[0].pNext 				= NULL;
		vkrpbegininfo_work[0].renderPass 			= vkrendpass_work[0];
		vkrpbegininfo_work[0].framebuffer 			= vkframebuff_work[0];
		vkrpbegininfo_work[0].renderArea 			= vkrect2d;
		vkrpbegininfo_work[0].clearValueCount 		= 1;
		vkrpbegininfo_work[0].pClearValues 			= &vkclearval[2];
		vkrpbegininfo_work[1].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegininfo_work[1].pNext 				= NULL;
		vkrpbegininfo_work[1].renderPass 			= vkrendpass_work[1];
		vkrpbegininfo_work[1].framebuffer 			= vkframebuff_work[1];
		vkrpbegininfo_work[1].renderArea 			= vkrect2d;
		vkrpbegininfo_work[1].clearValueCount 		= 1;
		vkrpbegininfo_work[1].pClearValues 			= &vkclearval[3];

	const int WORKIMGS = 2;

	VkDescriptorSetLayoutBinding vkdescsetlaybind[2];
		vkdescsetlaybind[0].binding					= 1;
		vkdescsetlaybind[0].descriptorType			= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		vkdescsetlaybind[0].descriptorCount			= 1;
		vkdescsetlaybind[0].stageFlags				= VK_SHADER_STAGE_FRAGMENT_BIT;
		vkdescsetlaybind[0].pImmutableSamplers		= NULL;
		vkdescsetlaybind[1].binding					= 0;
		vkdescsetlaybind[1].descriptorType			= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		vkdescsetlaybind[1].descriptorCount			= 1;
		vkdescsetlaybind[1].stageFlags				= VK_SHADER_STAGE_FRAGMENT_BIT;
		vkdescsetlaybind[1].pImmutableSamplers		= NULL;

	VkDescriptorSetLayoutCreateInfo vkdescset_info;
		vkdescset_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	nf(&vkdescset_info);
		vkdescset_info.bindingCount		= 2;
		vkdescset_info.pBindings		= vkdescsetlaybind;

	VkDescriptorSetLayout vkdescsetlay;
	va("vkCreateDescriptorSetLayout", &vkres, vkdescsetlay,
		vkCreateDescriptorSetLayout(vkld[0], &vkdescset_info, NULL, &vkdescsetlay) );

	VkDescriptorPoolSize vkdescpoolsize[2];
		vkdescpoolsize[0].type 				= vkdescsetlaybind[0].descriptorType;
		vkdescpoolsize[0].descriptorCount 	= WORKIMGS;
		vkdescpoolsize[1].type 				= vkdescsetlaybind[1].descriptorType;
		vkdescpoolsize[1].descriptorCount 	= WORKIMGS;

	VkDescriptorPoolCreateInfo vkdescpool_info[2];
		vkdescpool_info[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	nf(&vkdescpool_info[0]);
		vkdescpool_info[0].maxSets 			= WORKIMGS;
		vkdescpool_info[0].poolSizeCount 	= 1;
		vkdescpool_info[0].pPoolSizes 		= &vkdescpoolsize[0];
		vkdescpool_info[1].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	nf(&vkdescpool_info[1]);
		vkdescpool_info[1].maxSets 			= WORKIMGS;
		vkdescpool_info[1].poolSizeCount 	= 1;
		vkdescpool_info[1].pPoolSizes 		= &vkdescpoolsize[1];

	VkDescriptorPool vkdescpool[2];
	va("vkCreateDescriptorPool", &vkres, vkdescpool[0],
		vkCreateDescriptorPool(vkld[0], &vkdescpool_info[0], NULL, &vkdescpool[0]) );
	va("vkCreateDescriptorPool", &vkres, vkdescpool[1],
		vkCreateDescriptorPool(vkld[0], &vkdescpool_info[1], NULL, &vkdescpool[1]) );

	VkDescriptorSetAllocateInfo vkdescsetallo_info[2];
		vkdescsetallo_info[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		vkdescsetallo_info[0].pNext 				= NULL;
		vkdescsetallo_info[0].descriptorPool 		= vkdescpool[0];
		vkdescsetallo_info[0].descriptorSetCount 	= 1;
		vkdescsetallo_info[0].pSetLayouts 			= &vkdescsetlay;
		vkdescsetallo_info[1].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		vkdescsetallo_info[1].pNext 				= NULL;
		vkdescsetallo_info[1].descriptorPool 		= vkdescpool[1];
		vkdescsetallo_info[1].descriptorSetCount 	= 1;
		vkdescsetallo_info[1].pSetLayouts 			= &vkdescsetlay;

	VkDescriptorSet vkdescset[2];
	va("vkAllocateDescriptorSets", &vkres, vkdescset[0],
		vkAllocateDescriptorSets(vkld[0], &vkdescsetallo_info[0], &vkdescset[0]) );
	va("vkAllocateDescriptorSets", &vkres, vkdescset[1],
		vkAllocateDescriptorSets(vkld[0], &vkdescsetallo_info[1], &vkdescset[1]) );

	VkSamplerCreateInfo vksampler_info;
		vksampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	nf(&vksampler_info);
		vksampler_info.magFilter				= VK_FILTER_NEAREST;
		vksampler_info.minFilter				= VK_FILTER_NEAREST;
		vksampler_info.mipmapMode				= VK_SAMPLER_MIPMAP_MODE_NEAREST;
		vksampler_info.addressModeU				= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		vksampler_info.addressModeV				= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		vksampler_info.addressModeW				= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		vksampler_info.mipLodBias				= 1.0f;
		vksampler_info.anisotropyEnable			= VK_FALSE;
		vksampler_info.maxAnisotropy			= 1.0f;
		vksampler_info.compareEnable			= VK_FALSE;
		vksampler_info.compareOp				= VK_COMPARE_OP_NEVER;
		vksampler_info.minLod					= 1.0f;
		vksampler_info.maxLod					= 1.0f;
		vksampler_info.borderColor				= VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		vksampler_info.unnormalizedCoordinates	= VK_FALSE;

	VkSampler vksampler;
	va("vkCreateSampler", &vkres, vksampler,
		vkCreateSampler(vkld[0], &vksampler_info, NULL, &vksampler) );

	VkDescriptorImageInfo vkdescimg_info[2];
		vkdescimg_info[0].sampler			= vksampler;
		vkdescimg_info[0].imageView			= vkimgview_work[1]; // Reference the other image
		vkdescimg_info[0].imageLayout		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vkdescimg_info[1].sampler			= vksampler;
		vkdescimg_info[1].imageView			= vkimgview_work[0]; // Reference the other image
		vkdescimg_info[1].imageLayout		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet vkwritedescset[2];
		vkwritedescset[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vkwritedescset[0].pNext 				= NULL;
		vkwritedescset[0].dstSet 				= vkdescset[0];
		vkwritedescset[0].dstBinding 			= 1;
		vkwritedescset[0].dstArrayElement 		= 0;
		vkwritedescset[0].descriptorCount 		= 1;
		vkwritedescset[0].descriptorType 		= vkdescsetlaybind[0].descriptorType;
		vkwritedescset[0].pImageInfo 			= &vkdescimg_info[0];
		vkwritedescset[0].pBufferInfo 			= NULL;
		vkwritedescset[0].pTexelBufferView 		= NULL;
		vkwritedescset[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vkwritedescset[1].pNext 				= NULL;
		vkwritedescset[1].dstSet 				= vkdescset[1];
		vkwritedescset[1].dstBinding 			= 1;
		vkwritedescset[1].dstArrayElement 		= 0;
		vkwritedescset[1].descriptorCount 		= 1;
		vkwritedescset[1].descriptorType 		= vkdescsetlaybind[0].descriptorType;
		vkwritedescset[1].pImageInfo 			= &vkdescimg_info[1];
		vkwritedescset[1].pBufferInfo 			= NULL;
		vkwritedescset[1].pTexelBufferView 		= NULL;

	VkDeviceSize vkdevsize;
		vkdevsize = sizeof(uint32_t) * 3;
	ov("UniBuf size", vkdevsize);

	VkBufferCreateInfo vkbuff_info;
		vkbuff_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	nf(&vkbuff_info);
		vkbuff_info.size 						= vkdevsize;
		vkbuff_info.usage 						= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		vkbuff_info.sharingMode 				= VK_SHARING_MODE_EXCLUSIVE;
		vkbuff_info.queueFamilyIndexCount 		= 1;
		vkbuff_info.pQueueFamilyIndices 		= &GQF_IDX;

	VkBuffer vkbuff;
	va("vkCreateBuffer", &vkres, vkbuff,
		vkCreateBuffer(vkld[0], &vkbuff_info, NULL, &vkbuff) );

	VkDescriptorBufferInfo vkDescBuff_info;
		vkDescBuff_info.buffer 		= vkbuff;
		vkDescBuff_info.offset 		= 0;
		vkDescBuff_info.range 		= VK_WHOLE_SIZE;

	VkWriteDescriptorSet vkwritedescset_ub[2];
		vkwritedescset_ub[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vkwritedescset_ub[0].pNext 					= NULL;
		vkwritedescset_ub[0].dstSet 				= vkdescset[0];
		vkwritedescset_ub[0].dstBinding 			= 0;
		vkwritedescset_ub[0].dstArrayElement 		= 0;
		vkwritedescset_ub[0].descriptorCount 		= 1;
		vkwritedescset_ub[0].descriptorType 		= vkdescsetlaybind[1].descriptorType;
		vkwritedescset_ub[0].pImageInfo 			= NULL;
		vkwritedescset_ub[0].pBufferInfo 			= &vkDescBuff_info;
		vkwritedescset_ub[0].pTexelBufferView 		= NULL;
		vkwritedescset_ub[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vkwritedescset_ub[1].pNext 					= NULL;
		vkwritedescset_ub[1].dstSet 				= vkdescset[1];
		vkwritedescset_ub[1].dstBinding 			= 0;
		vkwritedescset_ub[1].dstArrayElement 		= 0;
		vkwritedescset_ub[1].descriptorCount 		= 1;
		vkwritedescset_ub[1].descriptorType 		= vkdescsetlaybind[1].descriptorType;
		vkwritedescset_ub[1].pImageInfo 			= NULL;
		vkwritedescset_ub[1].pBufferInfo 			= &vkDescBuff_info;
		vkwritedescset_ub[1].pTexelBufferView 		= NULL;

	int mem_index_ub = UINT32_MAX;
	for(int i = 0; i < vkpd_memprops.memoryTypeCount; i++) {
		if( vkpd_memprops.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		&&	mem_index_ub == UINT32_MAX ) {
			mem_index_ub = i; } }
	ov("mem_index_ub", mem_index_ub);

	VkMemoryRequirements vkmemreqs_ub;
	rv("vkGetBufferMemoryRequirements");
		vkGetBufferMemoryRequirements(vkld[0], vkbuff, &vkmemreqs_ub);
		ov("memreq size", 			vkmemreqs_ub.size);
		ov("memreq alignment", 		vkmemreqs_ub.alignment);
		ov("memreq memoryTypeBits", vkmemreqs_ub.memoryTypeBits);

	VkMemoryAllocateInfo vkmemallo_info;
		vkmemallo_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		vkmemallo_info.pNext			= NULL;
		vkmemallo_info.allocationSize	= vkmemreqs_ub.size;
		vkmemallo_info.memoryTypeIndex	= mem_index_ub;

	VkDeviceMemory vkdevmem_ub;
	vr("vkAllocateMemory", &vkres, 
		vkAllocateMemory(vkld[0], &vkmemallo_info, NULL, &vkdevmem_ub) );

	vr("vkBindBufferMemory", &vkres, 
		vkBindBufferMemory(vkld[0], vkbuff, vkdevmem_ub, 0) );

	WSize 	window_size;
			window_size.app_w	= APP_W;
			window_size.app_h	= APP_H;
			window_size.divs	= 1;
			window_size.unused  = 15;
	MInfo 	mouse_info;
			mouse_info.mouse_x 	= 0;
			mouse_info.mouse_y 	= 0;
			mouse_info.mouse_c 	= 0;
			mouse_info.unused 	= 15;

	UniBuf ub;
		ub.wsize = wsize_pack( window_size );
		ub.frame = uint32_t(0);
		ub.minfo = minfo_pack( mouse_info  );

	ov("UniBuf Size", sizeof(ub));

	void *pvoid_memmap;
	va("vkMapMemory", &vkres, pvoid_memmap,
		vkMapMemory(vkld[0], vkdevmem_ub, vkDescBuff_info.offset, vkDescBuff_info.range, 0, &pvoid_memmap) );

	rv("memcpy");
		memcpy(pvoid_memmap, &ub, sizeof(ub));

	rv("vkUpdateDescriptorSets");
		vkUpdateDescriptorSets(vkld[0], 1, &vkwritedescset[0], 0, NULL);
	rv("vkUpdateDescriptorSets");
		vkUpdateDescriptorSets(vkld[0], 1, &vkwritedescset[1], 0, NULL);

	rv("vkUpdateDescriptorSets");
		vkUpdateDescriptorSets(vkld[0], 1, &vkwritedescset_ub[0], 0, NULL);
	rv("vkUpdateDescriptorSets");
		vkUpdateDescriptorSets(vkld[0], 1, &vkwritedescset_ub[1], 0, NULL);

	VkPipelineLayoutCreateInfo vkpipelay_info_work;
		vkpipelay_info_work.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	nf(&vkpipelay_info_work);
		vkpipelay_info_work.setLayoutCount 				= 1;
		vkpipelay_info_work.pSetLayouts 				= &vkdescsetlay;
		vkpipelay_info_work.pushConstantRangeCount 		= 0;
		vkpipelay_info_work.pPushConstantRanges 		= NULL;

	VkPipelineLayout vkpipelay_work;
	va("vkCreatePipelineLayout", &vkres, vkpipelay_work,
		vkCreatePipelineLayout(vkld[0], &vkpipelay_info_work, NULL, &vkpipelay_work) );

	VkGraphicsPipelineCreateInfo vkgfxpipe_info_work[2];
		vkgfxpipe_info_work[0].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	nf(&vkgfxpipe_info_work[0]);
		vkgfxpipe_info_work[0].stageCount 				= 2;
		vkgfxpipe_info_work[0].pStages 					= vkgfxpipe_ss_info;
		vkgfxpipe_info_work[0].pVertexInputState 		= &vkpipevertinput_info;
		vkgfxpipe_info_work[0].pInputAssemblyState 		= &vkpipeinputass_info;
		vkgfxpipe_info_work[0].pTessellationState 		= NULL;
		vkgfxpipe_info_work[0].pViewportState 			= &vkpipeviewport_info;
		vkgfxpipe_info_work[0].pRasterizationState 		= &vkpiperastinfo;
		vkgfxpipe_info_work[0].pMultisampleState 		= &vkpipems_info;
		vkgfxpipe_info_work[0].pDepthStencilState 		= NULL;
		vkgfxpipe_info_work[0].pColorBlendState 		= &vkpipecolblend;
		vkgfxpipe_info_work[0].pDynamicState 			= NULL;
		vkgfxpipe_info_work[0].layout 					= vkpipelay_work;
		vkgfxpipe_info_work[0].renderPass 				= vkrendpass_work[0];
		vkgfxpipe_info_work[0].subpass 					= 0;
		vkgfxpipe_info_work[0].basePipelineHandle 		= VK_NULL_HANDLE;
		vkgfxpipe_info_work[0].basePipelineIndex 		= -1;
		vkgfxpipe_info_work[1].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	nf(&vkgfxpipe_info_work[1]);
		vkgfxpipe_info_work[1].stageCount 				= 2;
		vkgfxpipe_info_work[1].pStages 					= vkgfxpipe_ss_info;
		vkgfxpipe_info_work[1].pVertexInputState 		= &vkpipevertinput_info;
		vkgfxpipe_info_work[1].pInputAssemblyState 		= &vkpipeinputass_info;
		vkgfxpipe_info_work[1].pTessellationState 		= NULL;
		vkgfxpipe_info_work[1].pViewportState 			= &vkpipeviewport_info;
		vkgfxpipe_info_work[1].pRasterizationState 		= &vkpiperastinfo;
		vkgfxpipe_info_work[1].pMultisampleState 		= &vkpipems_info;
		vkgfxpipe_info_work[1].pDepthStencilState 		= NULL;
		vkgfxpipe_info_work[1].pColorBlendState 		= &vkpipecolblend;
		vkgfxpipe_info_work[1].pDynamicState 			= NULL;
		vkgfxpipe_info_work[1].layout 					= vkpipelay_work;
		vkgfxpipe_info_work[1].renderPass 				= vkrendpass_work[1];
		vkgfxpipe_info_work[1].subpass 					= 0;
		vkgfxpipe_info_work[1].basePipelineHandle 		= VK_NULL_HANDLE;
		vkgfxpipe_info_work[1].basePipelineIndex 		= -1;

	VkPipeline vkpipe_work[2];
	va("vkCreateGraphicsPipelines", &vkres, vkpipe_work[0],
		vkCreateGraphicsPipelines(vkld[0], VK_NULL_HANDLE, 1, &vkgfxpipe_info_work[0], NULL, &vkpipe_work[0]) );
	va("vkCreateGraphicsPipelines", &vkres, vkpipe_work[1],
		vkCreateGraphicsPipelines(vkld[0], VK_NULL_HANDLE, 1, &vkgfxpipe_info_work[1], NULL, &vkpipe_work[1]) );

	for(int i = 0; i < 2; i++) {
		vr("vkBeginCommandBuffer", &vkres, 
			vkBeginCommandBuffer(vkcombuf_work[i], &vkcombufbegin_info[i]) );

			rv("vkCmdBeginRenderPass");
				vkCmdBeginRenderPass (
					vkcombuf_work[i], &vkrpbegininfo_work[i], VK_SUBPASS_CONTENTS_INLINE );

				rv("vkCmdBindPipeline");
					vkCmdBindPipeline (
						vkcombuf_work[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkpipe_work[i] );

				rv("vkCmdBindDescriptorSets");
					vkCmdBindDescriptorSets (
						vkcombuf_work[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkpipelay_work,
						0, 1, &vkdescset[i], 0, NULL );

				rv("vkCmdDraw");
					vkCmdDraw (
						vkcombuf_work[i], 3, 1, 0, 0 );

			rv("vkCmdEndRenderPass");
				vkCmdEndRenderPass(vkcombuf_work[i]);

		vr("vkEndCommandBuffer", &vkres, 
			vkEndCommandBuffer(vkcombuf_work[i]) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RECORD PRES_INIT");		/**/
	///////////////////////////////////////////////////

	VkAttachmentDescription vkattdesc_init;
		vkattdesc_init.flags 			= 0;
		vkattdesc_init.format 			= vksurf_fmt[SURF_FMT].format;
		vkattdesc_init.samples 			= VK_SAMPLE_COUNT_1_BIT;
		vkattdesc_init.loadOp 			= VK_ATTACHMENT_LOAD_OP_CLEAR;
		vkattdesc_init.storeOp 			= VK_ATTACHMENT_STORE_OP_STORE;
		vkattdesc_init.stencilLoadOp 	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkattdesc_init.stencilStoreOp 	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkattdesc_init.initialLayout 	= VK_IMAGE_LAYOUT_UNDEFINED;
		vkattdesc_init.finalLayout 		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference vkattref_init;
		vkattref_init.attachment 	= 0;
		vkattref_init.layout 		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription vksubpassdesc_init;
		vksubpassdesc_init.flags 						= 0;
		vksubpassdesc_init.pipelineBindPoint 			= VK_PIPELINE_BIND_POINT_GRAPHICS;
		vksubpassdesc_init.inputAttachmentCount 		= 0;
		vksubpassdesc_init.pInputAttachments 			= NULL;
		vksubpassdesc_init.colorAttachmentCount 		= 1;
		vksubpassdesc_init.pColorAttachments 			= &vkattref_init;
		vksubpassdesc_init.pResolveAttachments 			= NULL;
		vksubpassdesc_init.pDepthStencilAttachment 		= NULL;
		vksubpassdesc_init.preserveAttachmentCount 		= 0;
		vksubpassdesc_init.pPreserveAttachments 		= NULL;

	VkRenderPassCreateInfo vkrendpass_info_init;
		vkrendpass_info_init.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&vkrendpass_info_init);
		vkrendpass_info_init.attachmentCount 	= 1;
		vkrendpass_info_init.pAttachments 		= &vkattdesc_init;
		vkrendpass_info_init.subpassCount 		= 1;
		vkrendpass_info_init.pSubpasses 		= &vksubpassdesc_init;
		vkrendpass_info_init.dependencyCount 	= 0;
		vkrendpass_info_init.pDependencies 		= NULL;
	VkRenderPass vkrendpass_init;
	va("vkCreateRenderPass", &vkres, vkrendpass_init,
		vkCreateRenderPass(vkld[0], &vkrendpass_info_init, NULL, &vkrendpass_init) );

	VkImageViewCreateInfo vkimgview_info[2];
		vkimgview_info[0].sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	nf(&vkimgview_info[0]);
		vkimgview_info[0].image 				= vkswap_img[0];
		vkimgview_info[0].viewType 				= VK_IMAGE_VIEW_TYPE_2D;
		vkimgview_info[0].format 				= vksurf_fmt[SURF_FMT].format;
		vkimgview_info[0].components.r			= VK_COMPONENT_SWIZZLE_IDENTITY;
		vkimgview_info[0].components.g			= VK_COMPONENT_SWIZZLE_IDENTITY;
		vkimgview_info[0].components.b			= VK_COMPONENT_SWIZZLE_IDENTITY;
		vkimgview_info[0].components.a			= VK_COMPONENT_SWIZZLE_IDENTITY;
		vkimgview_info[0].subresourceRange 		= vkimgsubrange;
		vkimgview_info[1].sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	nf(&vkimgview_info[1]);
		vkimgview_info[1].image 				= vkswap_img[1];
		vkimgview_info[1].viewType 				= VK_IMAGE_VIEW_TYPE_2D;
		vkimgview_info[1].format 				= vksurf_fmt[SURF_FMT].format;
		vkimgview_info[1].components.r			= VK_COMPONENT_SWIZZLE_IDENTITY;
		vkimgview_info[1].components.g			= VK_COMPONENT_SWIZZLE_IDENTITY;
		vkimgview_info[1].components.b			= VK_COMPONENT_SWIZZLE_IDENTITY;
		vkimgview_info[1].components.a			= VK_COMPONENT_SWIZZLE_IDENTITY;
		vkimgview_info[1].subresourceRange 		= vkimgsubrange;

	VkImageView vkimgview[2];
	va("vkCreateImageView", &vkres, vkimgview[0],
		vkCreateImageView(vkld[0], &vkimgview_info[0], NULL, &vkimgview[0]) );
	va("vkCreateImageView", &vkres, vkimgview[1],
		vkCreateImageView(vkld[0], &vkimgview_info[1], NULL, &vkimgview[1]) );

	VkFramebufferCreateInfo vkframebuff_info_init[2];
		vkframebuff_info_init[0].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	nf(&vkframebuff_info_init[0]);
		vkframebuff_info_init[0].renderPass 		= vkrendpass_init;
		vkframebuff_info_init[0].attachmentCount 	= 1;
		vkframebuff_info_init[0].pAttachments 		= &vkimgview[0];
		vkframebuff_info_init[0].width 				= vksurf_ables[0].currentExtent.width;
		vkframebuff_info_init[0].height 			= vksurf_ables[0].currentExtent.height;
		vkframebuff_info_init[0].layers 			= 1;
		vkframebuff_info_init[1].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	nf(&vkframebuff_info_init[1]);
		vkframebuff_info_init[1].renderPass 		= vkrendpass_init;
		vkframebuff_info_init[1].attachmentCount 	= 1;
		vkframebuff_info_init[1].pAttachments 		= &vkimgview[1];
		vkframebuff_info_init[1].width 				= vksurf_ables[0].currentExtent.width;
		vkframebuff_info_init[1].height 			= vksurf_ables[0].currentExtent.height;
		vkframebuff_info_init[1].layers 			= 1;

	VkFramebuffer vkframebuff_init[2];
	va("vkCreateFramebuffer", &vkres, vkframebuff_init[0],
		vkCreateFramebuffer(vkld[0], &vkframebuff_info_init[0], NULL, &vkframebuff_init[0]) );
	va("vkCreateFramebuffer", &vkres, vkframebuff_init[1],
		vkCreateFramebuffer(vkld[0], &vkframebuff_info_init[1], NULL, &vkframebuff_init[1]) );

	VkRenderPassBeginInfo vkrpbegininfo_pres_init[2];
		vkrpbegininfo_pres_init[0].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegininfo_pres_init[0].pNext 				= NULL;
		vkrpbegininfo_pres_init[0].renderPass 			= vkrendpass_init;
		vkrpbegininfo_pres_init[0].framebuffer 			= vkframebuff_init[0];
		vkrpbegininfo_pres_init[0].renderArea 			= vkrect2d;
		vkrpbegininfo_pres_init[0].clearValueCount 		= 1;
		vkrpbegininfo_pres_init[0].pClearValues 		= &vkclearval[0];
		vkrpbegininfo_pres_init[1].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegininfo_pres_init[1].pNext 				= NULL;
		vkrpbegininfo_pres_init[1].renderPass 			= vkrendpass_init;
		vkrpbegininfo_pres_init[1].framebuffer 			= vkframebuff_init[1];
		vkrpbegininfo_pres_init[1].renderArea 			= vkrect2d;
		vkrpbegininfo_pres_init[1].clearValueCount 		= 1;
		vkrpbegininfo_pres_init[1].pClearValues 		= &vkclearval[1];

	for(int i = 0; i < 2; i++) {
		vr("vkBeginCommandBuffer", &vkres, 
			vkBeginCommandBuffer(vkcombuf_pres_init[i], &vkcombufbegin_info[i]) );

			rv("vkCmdBeginRenderPass");
				vkCmdBeginRenderPass (
					vkcombuf_pres_init[i], &vkrpbegininfo_pres_init[i], VK_SUBPASS_CONTENTS_INLINE );

			rv("vkCmdEndRenderPass");
				vkCmdEndRenderPass(vkcombuf_pres_init[i]);

		vr("vkEndCommandBuffer", &vkres, 
			vkEndCommandBuffer(vkcombuf_pres_init[i]) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "SUBMIT PRES_INIT");		/**/
	///////////////////////////////////////////////////

	for(int i = 0; i < 2; i++) {
		if(valid) {
			rv("vkcombuf_pres_init");
			VkSubmitInfo vksub_info[1];
				vksub_info[0].sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO;
				vksub_info[0].pNext					= NULL;
				vksub_info[0].waitSemaphoreCount	= 0;
				vksub_info[0].pWaitSemaphores		= NULL;
				vksub_info[0].pWaitDstStageMask		= NULL;
				vksub_info[0].commandBufferCount	= 1;
				vksub_info[0].pCommandBuffers		= &vkcombuf_pres_init[i];
				vksub_info[0].signalSemaphoreCount	= 0;
				vksub_info[0].pSignalSemaphores		= NULL;
			vr("vkQueueSubmit", &vkres, 
				vkQueueSubmit(vkq[0], 1, vksub_info, VK_NULL_HANDLE) ); } }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RECORD PRES_LOOP");		/**/
	///////////////////////////////////////////////////

	VkAttachmentDescription vkattdesc;
		vkattdesc.flags 			= 0;
		vkattdesc.format 			= vksurf_fmt[SURF_FMT].format;
		vkattdesc.samples 			= VK_SAMPLE_COUNT_1_BIT;
		vkattdesc.loadOp 			= VK_ATTACHMENT_LOAD_OP_LOAD;
		vkattdesc.storeOp 			= VK_ATTACHMENT_STORE_OP_STORE;
		vkattdesc.stencilLoadOp 	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkattdesc.stencilStoreOp 	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkattdesc.initialLayout 	= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		vkattdesc.finalLayout 		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference vkattref_pres;
		vkattref_pres.attachment 	= 0;
		vkattref_pres.layout 		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription vksubpassdesc_pres;
		vksubpassdesc_pres.flags 						= 0;
		vksubpassdesc_pres.pipelineBindPoint 			= VK_PIPELINE_BIND_POINT_GRAPHICS;
		vksubpassdesc_pres.inputAttachmentCount 		= 0;
		vksubpassdesc_pres.pInputAttachments 			= NULL;
		vksubpassdesc_pres.colorAttachmentCount 		= 1;
		vksubpassdesc_pres.pColorAttachments 			= &vkattref_pres;
		vksubpassdesc_pres.pResolveAttachments 			= NULL;
		vksubpassdesc_pres.pDepthStencilAttachment 		= NULL;
		vksubpassdesc_pres.preserveAttachmentCount 		= 0;
		vksubpassdesc_pres.pPreserveAttachments 		= NULL;

	VkRenderPassCreateInfo vkrendpass_info;
		vkrendpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&vkrendpass_info);
		vkrendpass_info.attachmentCount 	= 1;
		vkrendpass_info.pAttachments 		= &vkattdesc;
		vkrendpass_info.subpassCount 		= 1;
		vkrendpass_info.pSubpasses 			= &vksubpassdesc_pres;
		vkrendpass_info.dependencyCount 	= 0;
		vkrendpass_info.pDependencies 		= NULL;

	VkRenderPass vkrendpass;
	va("vkCreateRenderPass", &vkres, vkrendpass,
		vkCreateRenderPass(vkld[0], &vkrendpass_info, NULL, &vkrendpass) );

	VkFramebufferCreateInfo vkframebuff_info[2];
		vkframebuff_info[0].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	nf(&vkframebuff_info[0]);
		vkframebuff_info[0].renderPass 			= vkrendpass;
		vkframebuff_info[0].attachmentCount 	= 1;
		vkframebuff_info[0].pAttachments 		= &vkimgview[0];
		vkframebuff_info[0].width 				= vksurf_ables[0].currentExtent.width;
		vkframebuff_info[0].height 				= vksurf_ables[0].currentExtent.height;
		vkframebuff_info[0].layers 				= 1;
		vkframebuff_info[1].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	nf(&vkframebuff_info[1]);
		vkframebuff_info[1].renderPass 			= vkrendpass;
		vkframebuff_info[1].attachmentCount 	= 1;
		vkframebuff_info[1].pAttachments 		= &vkimgview[1];
		vkframebuff_info[1].width 				= vksurf_ables[0].currentExtent.width;
		vkframebuff_info[1].height 				= vksurf_ables[0].currentExtent.height;
		vkframebuff_info[1].layers 				= 1;

	VkFramebuffer vkframebuff[2];
	va("vkCreateFramebuffer", &vkres, vkframebuff[0],
		vkCreateFramebuffer(vkld[0], &vkframebuff_info[0], NULL, &vkframebuff[0]) );
	va("vkCreateFramebuffer", &vkres, vkframebuff[1],
		vkCreateFramebuffer(vkld[0], &vkframebuff_info[1], NULL, &vkframebuff[1]) );

	VkRenderPassBeginInfo vkrpbegininfo_pres[2];
		vkrpbegininfo_pres[0].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegininfo_pres[0].pNext 				= NULL;
		vkrpbegininfo_pres[0].renderPass 			= vkrendpass;
		vkrpbegininfo_pres[0].framebuffer 			= vkframebuff[0];
		vkrpbegininfo_pres[0].renderArea 			= vkrect2d;
		vkrpbegininfo_pres[0].clearValueCount 		= 1;
		vkrpbegininfo_pres[0].pClearValues 			= &vkclearval[0];
		vkrpbegininfo_pres[1].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegininfo_pres[1].pNext 				= NULL;
		vkrpbegininfo_pres[1].renderPass 			= vkrendpass;
		vkrpbegininfo_pres[1].framebuffer 			= vkframebuff[1];
		vkrpbegininfo_pres[1].renderArea 			= vkrect2d;
		vkrpbegininfo_pres[1].clearValueCount 		= 1;
		vkrpbegininfo_pres[1].pClearValues 			= &vkclearval[1];

	VkAttachmentDescription vkattdesc_pres_prs2src;
		vkattdesc_pres_prs2src.flags 			= 0;
		vkattdesc_pres_prs2src.format 			= vksurf_fmt[SURF_FMT].format;
		vkattdesc_pres_prs2src.samples 			= VK_SAMPLE_COUNT_1_BIT;
		vkattdesc_pres_prs2src.loadOp 			= VK_ATTACHMENT_LOAD_OP_LOAD;
		vkattdesc_pres_prs2src.storeOp 			= VK_ATTACHMENT_STORE_OP_STORE;
		vkattdesc_pres_prs2src.stencilLoadOp 	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkattdesc_pres_prs2src.stencilStoreOp 	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkattdesc_pres_prs2src.initialLayout 	= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		vkattdesc_pres_prs2src.finalLayout 		= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

	VkAttachmentReference vkattref_pres_prs2src;
		vkattref_pres_prs2src.attachment 	= 0;
		vkattref_pres_prs2src.layout 		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription vksubpassdesc_pres_prs2src;
		vksubpassdesc_pres_prs2src.flags 						= 0;
		vksubpassdesc_pres_prs2src.pipelineBindPoint 			= VK_PIPELINE_BIND_POINT_GRAPHICS;
		vksubpassdesc_pres_prs2src.inputAttachmentCount 		= 0;
		vksubpassdesc_pres_prs2src.pInputAttachments 			= NULL;
		vksubpassdesc_pres_prs2src.colorAttachmentCount 		= 1;
		vksubpassdesc_pres_prs2src.pColorAttachments 			= &vkattref_pres_prs2src;
		vksubpassdesc_pres_prs2src.pResolveAttachments 			= NULL;
		vksubpassdesc_pres_prs2src.pDepthStencilAttachment 		= NULL;
		vksubpassdesc_pres_prs2src.preserveAttachmentCount 		= 0;
		vksubpassdesc_pres_prs2src.pPreserveAttachments 		= NULL;

	VkRenderPassCreateInfo vkrendpass_info_pres_prs2src;
		vkrendpass_info_pres_prs2src.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&vkrendpass_info_pres_prs2src);
		vkrendpass_info_pres_prs2src.attachmentCount 	= 1;
		vkrendpass_info_pres_prs2src.pAttachments 		= &vkattdesc_pres_prs2src;
		vkrendpass_info_pres_prs2src.subpassCount 		= 1;
		vkrendpass_info_pres_prs2src.pSubpasses 		= &vksubpassdesc_pres_prs2src;
		vkrendpass_info_pres_prs2src.dependencyCount 	= 0;
		vkrendpass_info_pres_prs2src.pDependencies 		= NULL;

	VkRenderPass vkrendpass_pres_prs2src;
	va("vkCreateRenderPass", &vkres, vkrendpass_pres_prs2src,
		vkCreateRenderPass(vkld[0], &vkrendpass_info_pres_prs2src, NULL, &vkrendpass_pres_prs2src) );

	VkFramebufferCreateInfo vkframebuff_info_pres_prs2src[2];
		vkframebuff_info_pres_prs2src[0].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	nf(&vkframebuff_info_pres_prs2src[0]);
		vkframebuff_info_pres_prs2src[0].renderPass 		= vkrendpass_pres_prs2src;
		vkframebuff_info_pres_prs2src[0].attachmentCount 	= 1;
		vkframebuff_info_pres_prs2src[0].pAttachments 		= &vkimgview[0];
		vkframebuff_info_pres_prs2src[0].width 				= vksurf_ables[0].currentExtent.width;
		vkframebuff_info_pres_prs2src[0].height 			= vksurf_ables[0].currentExtent.height;
		vkframebuff_info_pres_prs2src[0].layers 			= 1;
		vkframebuff_info_pres_prs2src[1].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	nf(&vkframebuff_info_pres_prs2src[1]);
		vkframebuff_info_pres_prs2src[1].renderPass 		= vkrendpass_pres_prs2src;
		vkframebuff_info_pres_prs2src[1].attachmentCount 	= 1;
		vkframebuff_info_pres_prs2src[1].pAttachments 		= &vkimgview[1];
		vkframebuff_info_pres_prs2src[1].width 				= vksurf_ables[0].currentExtent.width;
		vkframebuff_info_pres_prs2src[1].height 			= vksurf_ables[0].currentExtent.height;
		vkframebuff_info_pres_prs2src[1].layers 			= 1;

	VkFramebuffer vkframebuff_pres_prs2src[2];
	va("vkCreateFramebuffer", &vkres, vkframebuff_pres_prs2src[0],
		vkCreateFramebuffer(vkld[0], &vkframebuff_info_pres_prs2src[0], NULL, &vkframebuff_pres_prs2src[0]) );
	va("vkCreateFramebuffer", &vkres, vkframebuff_pres_prs2src[1],
		vkCreateFramebuffer(vkld[0], &vkframebuff_info_pres_prs2src[1], NULL, &vkframebuff_pres_prs2src[1]) );

	VkRenderPassBeginInfo vkrpbegininfo_pres_prs2src[2];
		vkrpbegininfo_pres_prs2src[0].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegininfo_pres_prs2src[0].pNext 				= NULL;
		vkrpbegininfo_pres_prs2src[0].renderPass 			= vkrendpass_pres_prs2src;
		vkrpbegininfo_pres_prs2src[0].framebuffer 			= vkframebuff_pres_prs2src[0];
		vkrpbegininfo_pres_prs2src[0].renderArea 			= vkrect2d;
		vkrpbegininfo_pres_prs2src[0].clearValueCount 		= 1;
		vkrpbegininfo_pres_prs2src[0].pClearValues 			= &vkclearval[0];
		vkrpbegininfo_pres_prs2src[1].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegininfo_pres_prs2src[1].pNext 				= NULL;
		vkrpbegininfo_pres_prs2src[1].renderPass 			= vkrendpass_pres_prs2src;
		vkrpbegininfo_pres_prs2src[1].framebuffer 			= vkframebuff_pres_prs2src[1];
		vkrpbegininfo_pres_prs2src[1].renderArea 			= vkrect2d;
		vkrpbegininfo_pres_prs2src[1].clearValueCount 		= 1;
		vkrpbegininfo_pres_prs2src[1].pClearValues 			= &vkclearval[1];

	VkAttachmentDescription vkattdesc_pres_src2dst;
		vkattdesc_pres_src2dst.flags 			= 0;
		vkattdesc_pres_src2dst.format 			= vksurf_fmt[SURF_FMT].format;
		vkattdesc_pres_src2dst.samples 			= VK_SAMPLE_COUNT_1_BIT;
		vkattdesc_pres_src2dst.loadOp 			= VK_ATTACHMENT_LOAD_OP_LOAD;
		vkattdesc_pres_src2dst.storeOp 			= VK_ATTACHMENT_STORE_OP_STORE;
		vkattdesc_pres_src2dst.stencilLoadOp 	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkattdesc_pres_src2dst.stencilStoreOp 	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkattdesc_pres_src2dst.initialLayout 	= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vkattdesc_pres_src2dst.finalLayout 		= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

	VkAttachmentReference vkattref_pres_src2dst;
		vkattref_pres_src2dst.attachment 	= 0;
		vkattref_pres_src2dst.layout 		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription vksubpassdesc_pres_src2dst;
		vksubpassdesc_pres_src2dst.flags 						= 0;
		vksubpassdesc_pres_src2dst.pipelineBindPoint 			= VK_PIPELINE_BIND_POINT_GRAPHICS;
		vksubpassdesc_pres_src2dst.inputAttachmentCount 		= 0;
		vksubpassdesc_pres_src2dst.pInputAttachments 			= NULL;
		vksubpassdesc_pres_src2dst.colorAttachmentCount 		= 1;
		vksubpassdesc_pres_src2dst.pColorAttachments 			= &vkattref_pres_src2dst;
		vksubpassdesc_pres_src2dst.pResolveAttachments 			= NULL;
		vksubpassdesc_pres_src2dst.pDepthStencilAttachment 		= NULL;
		vksubpassdesc_pres_src2dst.preserveAttachmentCount 		= 0;
		vksubpassdesc_pres_src2dst.pPreserveAttachments 		= NULL;

	VkRenderPassCreateInfo vkrendpass_info_pres_src2dst;
		vkrendpass_info_pres_src2dst.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&vkrendpass_info_pres_src2dst);
		vkrendpass_info_pres_src2dst.attachmentCount 	= 1;
		vkrendpass_info_pres_src2dst.pAttachments 		= &vkattdesc_pres_src2dst;
		vkrendpass_info_pres_src2dst.subpassCount 		= 1;
		vkrendpass_info_pres_src2dst.pSubpasses 		= &vksubpassdesc_pres_src2dst;
		vkrendpass_info_pres_src2dst.dependencyCount 	= 0;
		vkrendpass_info_pres_src2dst.pDependencies 		= NULL;

	VkRenderPass vkrendpass_pres_src2dst;
	va("vkCreateRenderPass", &vkres, vkrendpass_pres_src2dst,
		vkCreateRenderPass(vkld[0], &vkrendpass_info_pres_src2dst, NULL, &vkrendpass_pres_src2dst) );

	VkFramebufferCreateInfo vkframebuff_info_pres_src2dst[2];
		vkframebuff_info_pres_src2dst[0].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	nf(&vkframebuff_info_pres_src2dst[0]);
		vkframebuff_info_pres_src2dst[0].renderPass 		= vkrendpass_pres_src2dst;
		vkframebuff_info_pres_src2dst[0].attachmentCount 	= 1;
		vkframebuff_info_pres_src2dst[0].pAttachments 		= &vkimgview[0];
		vkframebuff_info_pres_src2dst[0].width 				= vksurf_ables[0].currentExtent.width;
		vkframebuff_info_pres_src2dst[0].height 			= vksurf_ables[0].currentExtent.height;
		vkframebuff_info_pres_src2dst[0].layers 			= 1;
		vkframebuff_info_pres_src2dst[1].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	nf(&vkframebuff_info_pres_src2dst[1]);
		vkframebuff_info_pres_src2dst[1].renderPass 		= vkrendpass_pres_src2dst;
		vkframebuff_info_pres_src2dst[1].attachmentCount 	= 1;
		vkframebuff_info_pres_src2dst[1].pAttachments 		= &vkimgview[1];
		vkframebuff_info_pres_src2dst[1].width 				= vksurf_ables[0].currentExtent.width;
		vkframebuff_info_pres_src2dst[1].height 			= vksurf_ables[0].currentExtent.height;
		vkframebuff_info_pres_src2dst[1].layers 			= 1;

	VkFramebuffer vkframebuff_pres_src2dst[2];
	va("vkCreateFramebuffer", &vkres, vkframebuff_pres_src2dst[0],
		vkCreateFramebuffer(vkld[0], &vkframebuff_info_pres_src2dst[0], NULL, &vkframebuff_pres_src2dst[0]) );
	va("vkCreateFramebuffer", &vkres, vkframebuff_pres_src2dst[1],
		vkCreateFramebuffer(vkld[0], &vkframebuff_info_pres_src2dst[1], NULL, &vkframebuff_pres_src2dst[1]) );

	VkRenderPassBeginInfo vkrpbegininfo_pres_src2dst[2];
		vkrpbegininfo_pres_src2dst[0].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegininfo_pres_src2dst[0].pNext 				= NULL;
		vkrpbegininfo_pres_src2dst[0].renderPass 			= vkrendpass_pres_src2dst;
		vkrpbegininfo_pres_src2dst[0].framebuffer 			= vkframebuff_pres_src2dst[0];
		vkrpbegininfo_pres_src2dst[0].renderArea 			= vkrect2d;
		vkrpbegininfo_pres_src2dst[0].clearValueCount 		= 1;
		vkrpbegininfo_pres_src2dst[0].pClearValues 			= &vkclearval[0];
		vkrpbegininfo_pres_src2dst[1].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegininfo_pres_src2dst[1].pNext 				= NULL;
		vkrpbegininfo_pres_src2dst[1].renderPass 			= vkrendpass_pres_src2dst;
		vkrpbegininfo_pres_src2dst[1].framebuffer 			= vkframebuff_pres_src2dst[1];
		vkrpbegininfo_pres_src2dst[1].renderArea 			= vkrect2d;
		vkrpbegininfo_pres_src2dst[1].clearValueCount 		= 1;
		vkrpbegininfo_pres_src2dst[1].pClearValues 			= &vkclearval[1];

	VkAttachmentDescription vkattdesc_work_src2sro;
		vkattdesc_work_src2sro.flags 			= 0;
		vkattdesc_work_src2sro.format 			= vkimg_info_work.format;
		vkattdesc_work_src2sro.samples 			= VK_SAMPLE_COUNT_1_BIT;
		vkattdesc_work_src2sro.loadOp 			= VK_ATTACHMENT_LOAD_OP_LOAD;
		vkattdesc_work_src2sro.storeOp 			= VK_ATTACHMENT_STORE_OP_STORE;
		vkattdesc_work_src2sro.stencilLoadOp 	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkattdesc_work_src2sro.stencilStoreOp 	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkattdesc_work_src2sro.initialLayout 	= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vkattdesc_work_src2sro.finalLayout 		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentReference vkattref_work_src2sro;
		vkattref_work_src2sro.attachment 	= 0;
		vkattref_work_src2sro.layout 		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription vksubpassdesc_work_src2sro;
		vksubpassdesc_work_src2sro.flags 						= 0;
		vksubpassdesc_work_src2sro.pipelineBindPoint 			= VK_PIPELINE_BIND_POINT_GRAPHICS;
		vksubpassdesc_work_src2sro.inputAttachmentCount 		= 0;
		vksubpassdesc_work_src2sro.pInputAttachments 			= NULL;
		vksubpassdesc_work_src2sro.colorAttachmentCount 		= 1;
		vksubpassdesc_work_src2sro.pColorAttachments 			= &vkattref_work_src2sro;
		vksubpassdesc_work_src2sro.pResolveAttachments 			= NULL;
		vksubpassdesc_work_src2sro.pDepthStencilAttachment 		= NULL;
		vksubpassdesc_work_src2sro.preserveAttachmentCount 		= 0;
		vksubpassdesc_work_src2sro.pPreserveAttachments 		= NULL;

	VkRenderPassCreateInfo vkrendpass_info_work_src2sro;
		vkrendpass_info_work_src2sro.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&vkrendpass_info_work_src2sro);
		vkrendpass_info_work_src2sro.attachmentCount 	= 1;
		vkrendpass_info_work_src2sro.pAttachments 		= &vkattdesc_work_src2sro;
		vkrendpass_info_work_src2sro.subpassCount 		= 1;
		vkrendpass_info_work_src2sro.pSubpasses 		= &vksubpassdesc_work_src2sro;
		vkrendpass_info_work_src2sro.dependencyCount 	= 0;
		vkrendpass_info_work_src2sro.pDependencies 		= NULL;

	VkRenderPass vkrendpass_work_src2sro;
	va("vkCreateRenderPass", &vkres, vkrendpass_work_src2sro,
		vkCreateRenderPass(vkld[0], &vkrendpass_info_work_src2sro, NULL, &vkrendpass_work_src2sro) );

	VkFramebufferCreateInfo vkframebuff_info_work_src2sro[2];
		vkframebuff_info_work_src2sro[0].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	nf(&vkframebuff_info_work_src2sro[0]);
		vkframebuff_info_work_src2sro[0].renderPass 		= vkrendpass_work_src2sro;
		vkframebuff_info_work_src2sro[0].attachmentCount 	= 1;
		vkframebuff_info_work_src2sro[0].pAttachments 		= &vkimgview_work[0];
		vkframebuff_info_work_src2sro[0].width 				= vksurf_ables[0].currentExtent.width;
		vkframebuff_info_work_src2sro[0].height 			= vksurf_ables[0].currentExtent.height;
		vkframebuff_info_work_src2sro[0].layers 			= 1;
		vkframebuff_info_work_src2sro[1].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	nf(&vkframebuff_info_work_src2sro[1]);
		vkframebuff_info_work_src2sro[1].renderPass 		= vkrendpass_work_src2sro;
		vkframebuff_info_work_src2sro[1].attachmentCount 	= 1;
		vkframebuff_info_work_src2sro[1].pAttachments 		= &vkimgview_work[1];
		vkframebuff_info_work_src2sro[1].width 				= vksurf_ables[0].currentExtent.width;
		vkframebuff_info_work_src2sro[1].height 			= vksurf_ables[0].currentExtent.height;
		vkframebuff_info_work_src2sro[1].layers 			= 1;

	VkFramebuffer vkframebuff_work_src2sro[2];
	va("vkCreateFramebuffer", &vkres, vkframebuff_work_src2sro[0],
		vkCreateFramebuffer(vkld[0], &vkframebuff_info_work_src2sro[0], NULL, &vkframebuff_work_src2sro[0]) );
	va("vkCreateFramebuffer", &vkres, vkframebuff_work_src2sro[1],
		vkCreateFramebuffer(vkld[0], &vkframebuff_info_work_src2sro[1], NULL, &vkframebuff_work_src2sro[1]) );

	VkRenderPassBeginInfo vkrpbegininfo_work_src2sro[2];
		vkrpbegininfo_work_src2sro[0].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegininfo_work_src2sro[0].pNext 				= NULL;
		vkrpbegininfo_work_src2sro[0].renderPass 			= vkrendpass_work_src2sro;
		vkrpbegininfo_work_src2sro[0].framebuffer 			= vkframebuff_work_src2sro[0];
		vkrpbegininfo_work_src2sro[0].renderArea 			= vkrect2d;
		vkrpbegininfo_work_src2sro[0].clearValueCount 		= 1;
		vkrpbegininfo_work_src2sro[0].pClearValues 			= &vkclearval[0];
		vkrpbegininfo_work_src2sro[1].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegininfo_work_src2sro[1].pNext 				= NULL;
		vkrpbegininfo_work_src2sro[1].renderPass 			= vkrendpass_work_src2sro;
		vkrpbegininfo_work_src2sro[1].framebuffer 			= vkframebuff_work_src2sro[1];
		vkrpbegininfo_work_src2sro[1].renderArea 			= vkrect2d;
		vkrpbegininfo_work_src2sro[1].clearValueCount 		= 1;
		vkrpbegininfo_work_src2sro[1].pClearValues 			= &vkclearval[1];

	VkPipelineLayoutCreateInfo vkpipelay_info;
		vkpipelay_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	nf(&vkpipelay_info);
		vkpipelay_info.setLayoutCount 			= 0;
		vkpipelay_info.pSetLayouts 				= NULL;
		vkpipelay_info.pushConstantRangeCount 	= 0;
		vkpipelay_info.pPushConstantRanges 		= NULL;
	VkPipelineLayout vkpipelay;
	va("vkCreatePipelineLayout", &vkres, vkpipelay,
		vkCreatePipelineLayout(vkld[0], &vkpipelay_info, NULL, &vkpipelay) );

	VkGraphicsPipelineCreateInfo vkgfxpipe_info_pres;
		vkgfxpipe_info_pres.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	nf(&vkgfxpipe_info_pres);
		vkgfxpipe_info_pres.stageCount 				= 1;
		vkgfxpipe_info_pres.pStages 				= vkgfxpipe_ss_info;
		vkgfxpipe_info_pres.pVertexInputState 		= &vkpipevertinput_info;
		vkgfxpipe_info_pres.pInputAssemblyState 	= &vkpipeinputass_info;
		vkgfxpipe_info_pres.pTessellationState 		= NULL;
		vkgfxpipe_info_pres.pViewportState 			= &vkpipeviewport_info;
		vkgfxpipe_info_pres.pRasterizationState 	= &vkpiperastinfo;
		vkgfxpipe_info_pres.pMultisampleState 		= &vkpipems_info;
		vkgfxpipe_info_pres.pDepthStencilState 		= NULL;
		vkgfxpipe_info_pres.pColorBlendState 		= &vkpipecolblend;
		vkgfxpipe_info_pres.pDynamicState 			= NULL;
		vkgfxpipe_info_pres.layout 					= vkpipelay;
		vkgfxpipe_info_pres.renderPass 				= vkrendpass;
		vkgfxpipe_info_pres.subpass 				= 0;
		vkgfxpipe_info_pres.basePipelineHandle 		= VK_NULL_HANDLE;
		vkgfxpipe_info_pres.basePipelineIndex 		= -1;

	VkPipeline vkpipe_pres;
	va("vkCreateGraphicsPipelines", &vkres, vkpipe_pres,
		vkCreateGraphicsPipelines(vkld[0], VK_NULL_HANDLE, 1, &vkgfxpipe_info_pres, NULL, &vkpipe_pres) );

	VkImageBlit vkimgblit;
		vkimgblit.srcSubresource 		= vkimgsublayer;
		vkimgblit.srcOffsets[0].x 		= 0;
		vkimgblit.srcOffsets[0].y 		= 0;
		vkimgblit.srcOffsets[0].z 		= 0;
		vkimgblit.srcOffsets[1].x 		= vksurf_ables[0].currentExtent.width;
		vkimgblit.srcOffsets[1].y 		= vksurf_ables[0].currentExtent.height;
		vkimgblit.srcOffsets[1].z 		= 1;
		vkimgblit.dstSubresource 		= vkimgsublayer;
		vkimgblit.dstOffsets[0].x 		= 0;
		vkimgblit.dstOffsets[0].y 		= 0;
		vkimgblit.dstOffsets[0].z 		= 0;
		vkimgblit.dstOffsets[1].x 		= vksurf_ables[0].currentExtent.width;
		vkimgblit.dstOffsets[1].y 		= vksurf_ables[0].currentExtent.height;
		vkimgblit.dstOffsets[1].z 		= 1;

	VkBufferImageCopy vkbuffimgcopy;
		vkbuffimgcopy.bufferOffset 			= 0;
		vkbuffimgcopy.bufferRowLength 		= vksurf_ables[0].currentExtent.width;
		vkbuffimgcopy.bufferImageHeight 	= vksurf_ables[0].currentExtent.height;
		vkbuffimgcopy.imageSubresource 		= vkimgsublayer;
		vkbuffimgcopy.imageOffset.x 		= 0;
		vkbuffimgcopy.imageOffset.y 		= 0;
		vkbuffimgcopy.imageOffset.z 		= 0;
		vkbuffimgcopy.imageExtent.width 	= vksurf_ables[0].currentExtent.width;
		vkbuffimgcopy.imageExtent.height 	= vksurf_ables[0].currentExtent.height;
		vkbuffimgcopy.imageExtent.depth		= 1;

	VkBufferCreateInfo vkbuff_info_save;
		vkbuff_info_save.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	nf(&vkbuff_info_save);
		vkbuff_info_save.size 						= 3145728;
		vkbuff_info_save.usage 						= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		vkbuff_info_save.sharingMode 				= VK_SHARING_MODE_EXCLUSIVE;
		vkbuff_info_save.queueFamilyIndexCount 		= 1;
		vkbuff_info_save.pQueueFamilyIndices 		= &GQF_IDX;

	VkBuffer vkbuff_save;
	va("vkCreateBuffer", &vkres, vkbuff_save,
		vkCreateBuffer(vkld[0], &vkbuff_info_save, NULL, &vkbuff_save) );

	int mem_index_save = UINT32_MAX;
	for(int i = 0; i < vkpd_memprops.memoryTypeCount; i++) {
		if( vkpd_memprops.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		&&	mem_index_save == UINT32_MAX ) {
			mem_index_save = i; } }
	ov("mem_index_save", mem_index_save);

	VkMemoryRequirements vkmemreqs_save;
	rv("vkGetBufferMemoryRequirements");
		vkGetBufferMemoryRequirements(vkld[0], vkbuff_save, &vkmemreqs_save);
		ov("memreq size", 			vkmemreqs_save.size);
		ov("memreq alignment", 		vkmemreqs_save.alignment);
		ov("memreq memoryTypeBits", vkmemreqs_save.memoryTypeBits);

	VkMemoryAllocateInfo vkmemallo_info_save;
		vkmemallo_info_save.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		vkmemallo_info_save.pNext			= NULL;
		vkmemallo_info_save.allocationSize	= vkmemreqs_save.size;
		vkmemallo_info_save.memoryTypeIndex	= mem_index_save;

	VkDeviceMemory vkdevmem_save;
	vr("vkAllocateMemory", &vkres, 
		vkAllocateMemory(vkld[0], &vkmemallo_info_save, NULL, &vkdevmem_save) );

	vr("vkBindBufferMemory", &vkres, 
		vkBindBufferMemory(vkld[0], vkbuff_save, vkdevmem_save, 0) );

	for(int i = 0; i < 2; i++) {
		vr("vkBeginCommandBuffer", &vkres, 
			vkBeginCommandBuffer(vkcombuf_pres[i], &vkcombufbegin_info[i]) );

			rv("vkCmdBeginRenderPass");
				vkCmdBeginRenderPass (
					vkcombuf_pres[i], &vkrpbegininfo_pres_prs2src[i], VK_SUBPASS_CONTENTS_INLINE );
			rv("vkCmdEndRenderPass");
				vkCmdEndRenderPass(vkcombuf_pres[i]);

			rv("vkCmdCopyImageToBuffer");
				vkCmdCopyImageToBuffer (
					vkcombuf_pres[i], vkswap_img[i], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					vkbuff_save, 1, &vkbuffimgcopy );

			rv("vkCmdBeginRenderPass");
					vkCmdBeginRenderPass (
						vkcombuf_pres[i], &vkrpbegininfo_pres_src2dst[i], VK_SUBPASS_CONTENTS_INLINE );
			rv("vkCmdEndRenderPass");
				vkCmdEndRenderPass(vkcombuf_pres[i]);

			rv("vkCmdBlitImage");
				vkCmdBlitImage (
					vkcombuf_pres[i], 
					vkimg_work[i], 	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					vkswap_img[i], 	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1, &vkimgblit, 	VK_FILTER_NEAREST );

			rv("vkCmdBeginRenderPass");
				vkCmdBeginRenderPass (
					vkcombuf_pres[i], &vkrpbegininfo_work_src2sro[i], VK_SUBPASS_CONTENTS_INLINE );
			rv("vkCmdEndRenderPass");
				vkCmdEndRenderPass(vkcombuf_pres[i]);

			rv("vkCmdBeginRenderPass");
				vkCmdBeginRenderPass (
					vkcombuf_pres[i], &vkrpbegininfo_pres[i], VK_SUBPASS_CONTENTS_INLINE );
				rv("vkCmdBindPipeline");
					vkCmdBindPipeline (
						vkcombuf_pres[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkpipe_pres );
				rv("vkCmdDraw");
					vkCmdDraw (
						vkcombuf_pres[i], 3, 1, 0, 0 );
			rv("vkCmdEndRenderPass");
				vkCmdEndRenderPass(vkcombuf_pres[i]);

		vr("vkEndCommandBuffer", &vkres, 
			vkEndCommandBuffer(vkcombuf_pres[i]) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "MAIN LOOP");				/**/
	///////////////////////////////////////////////////

	int ilimit = TEST_CYCLES;
	int idx = 0;
	int SCR_idx = 0;
	int pause = 0;
	int SCR_frameskip = -1;

	void* SCR_data;
	vr("vkMapMemory", &vkres, 
		vkMapMemory(vkld[0], vkdevmem_save, 0, VK_WHOLE_SIZE, 0, &SCR_data) );

	do {
    	auto start = std::chrono::high_resolution_clock::now();
		if(valid) {
			XEvent xe;
			XSelectInput( d, w, ButtonPressMask | KeyPressMask );
			if(XCheckWindowEvent(d, w, ButtonPressMask | KeyPressMask, &xe)) {
				if( xe.type == 2) {
					if( xe.xbutton.button == 65	) { pause = (pause) ? 0 : 1; }
					if( xe.xbutton.button == 10	) { window_size.divs = 1; 	 }
					if( xe.xbutton.button == 11	) { window_size.divs = 2; 	 }
					if( xe.xbutton.button == 12	) { window_size.divs = 4; 	 }
					if( xe.xbutton.button == 13	) { window_size.divs = 8; 	 }
					ub.wsize = wsize_pack( window_size ); }
					
				if( xe.type == 4) {
					if(	xe.xbutton.button == 1
					|| 	xe.xbutton.button == 2
					|| 	xe.xbutton.button == 3
					|| 	xe.xbutton.button == 4
					|| 	xe.xbutton.button == 5
					|| 	xe.xbutton.button == 8
					|| 	xe.xbutton.button == 9 	)  {
						mouse_info.mouse_x 	= xe.xbutton.x;
						mouse_info.mouse_y 	= xe.xbutton.y;
						mouse_info.mouse_c 	= xe.xbutton.button;
						mouse_info.unused 	= 15;
						ub.minfo = minfo_pack(mouse_info); } }
				ov("Input", xe.xbutton.button); } }

		if (!pause) {
			if(valid) {
				iv("i", idx, idx);
				ub.frame = uint32_t(idx);
				rv("memcpy");
					memcpy(pvoid_memmap, &ub, sizeof(ub));
				rv("nanosleep(NS_DELAY)");
					nanosleep((const struct timespec[]){{0, NS_DELAY}}, NULL);
				vr("vkResetFences", &vkres, 
					vkResetFences(vkld[0], 1, vkfence_aqimg) );
				if(valid) {
					vr("vkAcquireNextImageKHR", &vkres, 
						vkAcquireNextImageKHR(vkld[0], vkswap[0], UINT64_MAX, vksemaph_image[0], 
							VK_NULL_HANDLE, &aqimg_idx[0]) );
						iv("aqimg_idx", aqimg_idx[0], idx);
						iv("vkswap_img", vkswap_img[aqimg_idx[0]], idx);

					if(valid) {
						rv("vkcombuf_work");
						VkSubmitInfo vksub_info[1];
							vksub_info[0].sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO;
							vksub_info[0].pNext					= NULL;
							vksub_info[0].waitSemaphoreCount	= 0;
							vksub_info[0].pWaitSemaphores		= NULL;
							vksub_info[0].pWaitDstStageMask		= NULL;
							vksub_info[0].commandBufferCount	= 1;
							vksub_info[0].pCommandBuffers		= &vkcombuf_work[aqimg_idx[0]];
							vksub_info[0].signalSemaphoreCount	= 0;
							vksub_info[0].pSignalSemaphores		= NULL;
						vr("vkQueueSubmit", &vkres, 
							vkQueueSubmit(vkq[0], 1, vksub_info, VK_NULL_HANDLE) ); }

					if(valid) {
						rv("vkcombuf_pres");
						VkSubmitInfo vksub_info[1];
							vksub_info[0].sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO;
							vksub_info[0].pNext	= NULL;
							vksub_info[0].waitSemaphoreCount	= 1;
							vksub_info[0].pWaitSemaphores		= vksemaph_image;
							vksub_info[0].pWaitDstStageMask		= &qsubwait;
							vksub_info[0].commandBufferCount	= 1;
							vksub_info[0].pCommandBuffers		= &vkcombuf_pres[aqimg_idx[0]];
							vksub_info[0].signalSemaphoreCount	= 1;
							vksub_info[0].pSignalSemaphores		= vksemaph_rendr;
						vr("vkQueueSubmit", &vkres, 
							vkQueueSubmit(vkq[0], 1, vksub_info, vkfence_aqimg[0]) ); }
						
					if(valid) {
						do {
							vr("vkWaitForFences <100ms>", &vkres, 
								vkWaitForFences(vkld[0], 1, vkfence_aqimg, VK_TRUE, 100000000) );
						} while (vkres[vkres.size()-1] == VK_TIMEOUT);

						VkPresentInfoKHR vkpresent_info[1];
							vkpresent_info[0].sType 	= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
							vkpresent_info[0].pNext 	= NULL;
							vkpresent_info[0].waitSemaphoreCount	= 1;
							vkpresent_info[0].pWaitSemaphores 		= vksemaph_rendr;
							vkpresent_info[0].swapchainCount 		= 1;
							vkpresent_info[0].pSwapchains 			= vkswap;
							vkpresent_info[0].pImageIndices 		= aqimg_idx;
							vkpresent_info[0].pResults 				= NULL;

						if(valid) {
							vr("vkQueuePresentKHR", &vkres, 
								vkQueuePresentKHR(vkq[0], &vkpresent_info[0]) ); } } } }

					if(valid && idx%SCR_frameskip == 0 && SCR_frameskip >= 0) {
						std::string ppmfile = "out/PPM" + std::to_string(SCR_idx) + ".PGM";
						ov("Screenshot Out", ppmfile);
						std::ofstream file(ppmfile.c_str(), std::ios::out | std::ios::binary);
							file 	<<	"P6" 									<< "\n"
								 	<< 	vksurf_ables[0].currentExtent.width		<< " "
							 		<< 	vksurf_ables[0].currentExtent.height	<< " "
									<< 	"255" << "\n";
							for(int i = 0; i < vksurf_ables[0].currentExtent.height; i++) {
								for(int j = 0; j < vksurf_ables[0].currentExtent.width; j++) {
									for(int k = 2; k >= 0; k--) {
										file.write(
											(const char*)SCR_data
											+ ((j*4) + k)
											+  (i*4*vksurf_ables[0].currentExtent.width)
											, 1 ); } } }
						file.close(); 
						SCR_idx++; }
			idx++; } else { nanosleep((const struct timespec[]){{0, NS_DELAY*30}}, NULL); }
		if(loglevel != 0 && idx == 2) { loglevel = loglevel * -1; }
		if(idx%512 == 0) {
			auto finish = std::chrono::high_resolution_clock::now();
			std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(finish-start).count()/1000000.0 << "ms\n"; }
	} while (valid && (idx < ilimit || ilimit < 1));

	  ///////////////////////////////////////////////
	 /**/	hd("STAGE:", "EXIT APPLICATION");	/**/
	///////////////////////////////////////////////

	if(!valid) { hd("STAGE:", "ABORTED"); }

	rv("XCloseDisplay");
		XCloseDisplay(d);

	rv("return");
		return 0;
}



