#define VK_USE_PLATFORM_XLIB_KHR
#define VK_API_VERSION_1_2 VK_MAKE_VERSION(1, 2, 0)
#include <cstring>
#include <time.h>
#include <fstream>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <X11/Xlib.h>
#include <vulkan/vulkan.h>

bool valid = 1;
bool output = 0;

void vr(const std::string& id, std::vector<VkResult>* reslist, VkResult res) {
	reslist->push_back(res);
	uint32_t 	idx 		= reslist->size() - 1;
	std::string	idx_string 	= std::to_string(idx);
	uint32_t 	idx_sz		= idx_string.size();
	std::string res_string 	= std::to_string(res);
	if(idx_sz < 4) { for(int i = 0; i < 4-idx_sz; i++) { idx_string = " " + idx_string; } }
	if(output) {
		std::cout	
			<< "  " << idx_string 		<< ":\t"
			<< (res==0?" ":res_string) 	<< " \t"
			<< id 						<< "\n";
	}
}

void ov(const std::string& id, auto v) {
	int 		padlen	= 4;
	int 		pads	= 10;
	std::string pad 	= " ";
	int 		padsize = (pads*padlen - id.size()) - 3;
	for(int i = 0; i < padsize; i++) { pad = pad + "."; }
	if(output) {
	std::cout 
		<< "\tinfo:\t    "
		<< id 	<< pad 
		<< " [" << v	<< "]\n";
	}
}

void iv(const std::string& id, auto ov, int idx) {
	int 		padlen 	= 4;
	int 		pads 	= 9;
	std::string pad 	= " ";
	int 		padsize = (pads*padlen - id.size()) - 3;
	for(int i = 0; i < padsize; i++) { pad = pad + "."; }
	if(output) {
	std::cout 
		<< "\tinfo:\t    "
		<< idx 	<< "\t"
		<< id 	<< pad 
		<< " [" << ov	<< "]\n";
	}
}

void rv(const std::string& id) {
	if(output) {
	std::cout 
		<< "  void: \t" 
		<< id	<< "\n";
	}
}

void hd(const std::string& id, const std::string& msg) {
	std::string bar = "";
	for(int i = 0; i < 20; i++) { bar = bar + "____"; }
	if(output) {
	std::cout 
		<< bar 	<< "\n "
		<< id 	<< "\t"
		<< msg 	<< "\n";
	}
}

void nf(auto *Vk_obj) {
	Vk_obj->pNext = NULL;
	Vk_obj->flags = 0;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL 
	debugCallback(
				VkDebugUtilsMessageSeverityFlagBitsEXT	messageSeverity, 
				VkDebugUtilsMessageTypeFlagsEXT			messageType, 
		const 	VkDebugUtilsMessengerCallbackDataEXT* 	pCallbackData, 
		void* 											pUserData		) 
{
	std::string bar = "";
	for(int i = 0; i < 20; i++) { bar = bar + "####"; }

	std::string msg 	= pCallbackData->pMessage;
	std::string msg_fmt = "";
	for(int i = 0; i < msg.size()-1; i++) {
		char chr = msg[i];
		char chrhtml = msg[i+1];
		msg_fmt = 
			(chr == ':' && chrhtml != '/' 
			? msg_fmt+":\n\n" 
			: 
				(chr == '|' 
				? msg_fmt+"\n" 
				: msg_fmt+chr
				)
			);
	}
	msg_fmt = msg_fmt + msg[msg.size()-1];
	if(output) {
		std::cout 
			<< "\n\n"
			<< bar 		<< "\n "
			<< msg_fmt
			<< "\n" 	<< bar
			<< "\n\n";
	}
	valid = 0;
	return VK_FALSE; 
}

struct ShaderCodeInfo {
	std::string 		shaderFilename;
	std::vector<char>	shaderData;
	size_t 				shaderBytes;
	bool 				shaderBytesValid;
};

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
	return rfsc_info[0];
}

struct init_ub {
    float w;
	float h;
	float seed;
};

int main(void) {

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "CONFIG");		/**/
	///////////////////////////////////////

	const uint32_t 	APP_W 			= 1536;
	const uint32_t 	APP_H 			= 768;
	const long 		FPS 			= 60;
	const int 		TEST_CYCLES 	= 6000;

	uint32_t 		PD_IDX 			= UINT32_MAX;
	uint32_t 		GQF_IDX 		= UINT32_MAX;
	uint32_t		SURF_FMT 		= UINT32_MAX;
	uint32_t		UB_MEMTYPE 		= UINT32_MAX;

	const uint32_t	SHDRSTGS 		= 2;
	const long 		NS_DELAY 		= 1000000000 / FPS;
	const float 	TRIQUAD_SCALE 	= 1.0;
	const float 	VP_SCALE 		= TRIQUAD_SCALE + (1.0-TRIQUAD_SCALE) * 0.5;

	const uint32_t 	VERT_FLS 		= 1;
	const uint32_t 	FRAG_FLS 		= 2;
	const uint32_t 	INST_EXS 		= 3;
	const uint32_t 	LDEV_EXS 		= 1;
	const uint32_t 	VLID_LRS 		= 1;

	const char* 	filepath_vert		[VERT_FLS] =
		{	"./app/vert_TriQuad.spv" 					};
	const char* 	filepath_frag		[FRAG_FLS] =
		{	"./app/frag_init.spv",
			"./app/frag_automata0000.spv" 				};
	const char* 	instance_extensions	[INST_EXS] =
		{	"VK_KHR_surface", 
			"VK_KHR_xlib_surface", 
			"VK_EXT_debug_utils"						};
	const char* 	validation_layers	[VLID_LRS] =
		{	"VK_LAYER_KHRONOS_validation" 				};
	const char* 	device_extensions	[LDEV_EXS] =
		{	"VK_KHR_swapchain"							};

		ov("Window Width", 			APP_W		);
		ov("Window Height", 		APP_H		);
		ov("Render Cycles", 		TEST_CYCLES	);
		ov("FPS Target", 			FPS			);
		ov("Vertex Shaders", 		VERT_FLS	);
		ov("Fragment Shaders", 		FRAG_FLS	);

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "INIT");		/**/
	///////////////////////////////////////

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
			PD_IDX = i;
		}
	}

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
			GQF_IDX = i;
		}
	}

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

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "DISPLAY");	/**/
	///////////////////////////////////////

	rv("XOpenDisplay");
	Display *d = 
		XOpenDisplay(NULL);
		ov("DisplayString", DisplayString(d));

	Window rw = XDefaultRootWindow(d);
	rv("XCreateWindow");
	Window w = 
		XCreateWindow ( d, rw, 0, 0, APP_W, APP_H, 0, 
						CopyFromParent, CopyFromParent, CopyFromParent, 0, 0 );
		ov("Window", w);

	rv("XMapWindow");
		XMapWindow(d, w);

	rv("XFlush");
		XFlush(d);

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "SURFACE");	/**/
	///////////////////////////////////////

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
			iv("VkColorSpaceKHR", 	vksurf_fmt[i].colorSpace, 	i); 
		}


	for(int i = 0; i < pd_surf_fmt_cnt; i++) {
		if(	SURF_FMT == UINT32_MAX 
		&&	vksurf_fmt[i].colorSpace 	== VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
		&&	vksurf_fmt[i].format 		== VK_FORMAT_B8G8R8A8_UNORM 			) {
			SURF_FMT = i;
		}
	}

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
			iv("VkPresentModeKHR", 	vkpresmode[i], i); 
		}

	VkBool32 pd_surf_supported[1];
	vr("vkGetPhysicalDeviceSurfaceSupportKHR", &vkres, 
		vkGetPhysicalDeviceSurfaceSupportKHR(	vkpd[PD_IDX], GQF_IDX, vksurf[0], 
												&pd_surf_supported[0] 				) );
		ov("Surface Supported", (pd_surf_supported[0]?"TRUE":"FALSE"));

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "SHADERS");	/**/
	///////////////////////////////////////

	ShaderCodeInfo shader_info_vert[VERT_FLS];
	for(int i = 0; i < VERT_FLS; i++) {
		rv("getShaderCodeInfo");
		shader_info_vert[i] = getShaderCodeInfo(filepath_vert[i]);
	}

	VkShaderModuleCreateInfo vkshademod_vert_info[VERT_FLS];
	for(int i = 0; i < VERT_FLS; i++) {
		vkshademod_vert_info[i].sType		= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	nf(&vkshademod_vert_info[i]);
		vkshademod_vert_info[i].codeSize	= shader_info_vert[i].shaderBytes;
		vkshademod_vert_info[i].pCode		= 
			reinterpret_cast<const uint32_t*>(shader_info_vert[i].shaderData.data());
		iv("Vertex shaderFilename", 	 shader_info_vert[i].shaderFilename, 					i);
		iv("Vertex shaderBytes", 		 shader_info_vert[i].shaderBytes, 						i);
		iv("Vertex shaderBytesValid", 	(shader_info_vert[i].shaderBytesValid?"TRUE":"FALSE"), 	i);
	}

	ShaderCodeInfo shader_info_frag[FRAG_FLS];
	for(int i = 0; i < FRAG_FLS; i++) {
		rv("getShaderCodeInfo");
		shader_info_frag[i] = getShaderCodeInfo(filepath_frag[i]);
	}

	VkShaderModuleCreateInfo vkshademod_frag_info[FRAG_FLS];
	for(int i = 0; i < FRAG_FLS; i++) {
		vkshademod_frag_info[i].sType		= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	nf(&vkshademod_frag_info[i]);
		vkshademod_frag_info[i].codeSize	= shader_info_frag[i].shaderBytes;
		vkshademod_frag_info[i].pCode		= 
			reinterpret_cast<const uint32_t*>(shader_info_frag[i].shaderData.data());
		iv("Fragment shaderFilename", 	 shader_info_frag[i].shaderFilename, 					i);
		iv("Fragment shaderBytes", 		 shader_info_frag[i].shaderBytes, 						i);
		iv("Fragment shaderBytesValid", (shader_info_frag[i].shaderBytesValid?"TRUE":"FALSE"), 	i);
	}

	VkShaderModule vkshademod_vert[VERT_FLS];
	for(int i = 0; i < VERT_FLS; i++) {
		vr("vkCreateShaderModule", &vkres, 
			vkCreateShaderModule(vkld[0], &vkshademod_vert_info[i], NULL, &vkshademod_vert[i]) );
	}

	VkShaderModule vkshademod_frag[FRAG_FLS];
	for(int i = 0; i < FRAG_FLS; i++) {
		vr("vkCreateShaderModule", &vkres, 
			vkCreateShaderModule(vkld[0], &vkshademod_frag_info[i], NULL, &vkshademod_frag[i]) );
	}

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "PIPELINE");	/**/
	///////////////////////////////////////

	VkPipelineShaderStageCreateInfo vkgfxpipe_ss_info[SHDRSTGS];
		vkgfxpipe_ss_info[0].sType	= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	nf(&vkgfxpipe_ss_info[0]);
		vkgfxpipe_ss_info[0].stage					= VK_SHADER_STAGE_VERTEX_BIT;
		vkgfxpipe_ss_info[0].module					= vkshademod_vert[0];
		vkgfxpipe_ss_info[0].pName					= "main";
		vkgfxpipe_ss_info[0].pSpecializationInfo	= NULL;
		vkgfxpipe_ss_info[1].sType	= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	nf(&vkgfxpipe_ss_info[1]);
		vkgfxpipe_ss_info[1].stage					= VK_SHADER_STAGE_FRAGMENT_BIT;
		vkgfxpipe_ss_info[1].module					= vkshademod_frag[1];
		vkgfxpipe_ss_info[1].pName					= "main";
		vkgfxpipe_ss_info[1].pSpecializationInfo	= NULL;

	VkPipelineVertexInputStateCreateInfo vkgfxpipe_vertins_info[1];
		vkgfxpipe_vertins_info[0].sType	= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	nf(&vkgfxpipe_vertins_info[0]);
		vkgfxpipe_vertins_info[0].vertexBindingDescriptionCount		= 0;
		vkgfxpipe_vertins_info[0].pVertexBindingDescriptions		= NULL;
		vkgfxpipe_vertins_info[0].vertexAttributeDescriptionCount	= 0;
		vkgfxpipe_vertins_info[0].pVertexAttributeDescriptions		= NULL;

	VkPipelineInputAssemblyStateCreateInfo vkgfxpipe_ias_info[1];
		vkgfxpipe_ias_info[0].sType	= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	nf(&vkgfxpipe_ias_info[0]);
		vkgfxpipe_ias_info[0].topology					= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		vkgfxpipe_ias_info[0].primitiveRestartEnable	= VK_FALSE;

	VkViewport vkgfxpipe_viewport[1];
		vkgfxpipe_viewport[0].x			= 0;
		vkgfxpipe_viewport[0].y			= 0;
		vkgfxpipe_viewport[0].width		= vksurf_ables[0].currentExtent.width*VP_SCALE;
		vkgfxpipe_viewport[0].height	= vksurf_ables[0].currentExtent.height*VP_SCALE;
		vkgfxpipe_viewport[0].minDepth	= 0.0f;
		vkgfxpipe_viewport[0].maxDepth	= 1.0f;

	VkRect2D vkgfxpipe_sciz[1];
		vkgfxpipe_sciz[0].offset.x	= 0;
		vkgfxpipe_sciz[0].offset.y	= 0;
		vkgfxpipe_sciz[0].extent	= vksurf_ables[0].currentExtent;

	VkPipelineViewportStateCreateInfo vkgfxpipe_viewport_info[1];
		vkgfxpipe_viewport_info[0].sType	= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	nf(&vkgfxpipe_viewport_info[0]);
		vkgfxpipe_viewport_info[0].viewportCount	= 1;
		vkgfxpipe_viewport_info[0].pViewports		= vkgfxpipe_viewport;
		vkgfxpipe_viewport_info[0].scissorCount		= 1;
		vkgfxpipe_viewport_info[0].pScissors		= vkgfxpipe_sciz;

	VkPipelineRasterizationStateCreateInfo vkgfxpipe_rast_info[1];
		vkgfxpipe_rast_info[0].sType	= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	nf(&vkgfxpipe_rast_info[0]);
		vkgfxpipe_rast_info[0].depthClampEnable			= VK_FALSE;
		vkgfxpipe_rast_info[0].rasterizerDiscardEnable	= VK_FALSE;
		vkgfxpipe_rast_info[0].polygonMode				= VK_POLYGON_MODE_FILL;
		vkgfxpipe_rast_info[0].cullMode					= VK_CULL_MODE_NONE;
		vkgfxpipe_rast_info[0].frontFace				= VK_FRONT_FACE_CLOCKWISE;
		vkgfxpipe_rast_info[0].depthBiasEnable			= VK_FALSE;
		vkgfxpipe_rast_info[0].depthBiasConstantFactor	= 0.0f;
		vkgfxpipe_rast_info[0].depthBiasClamp			= 0.0f;
		vkgfxpipe_rast_info[0].depthBiasSlopeFactor		= 0.0f;
		vkgfxpipe_rast_info[0].lineWidth				= 1.0f;

	VkPipelineMultisampleStateCreateInfo vkgfxpipe_ms_info[1];
		vkgfxpipe_ms_info[0].sType	= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	nf(&vkgfxpipe_ms_info[0]);
		vkgfxpipe_ms_info[0].rasterizationSamples	= VK_SAMPLE_COUNT_1_BIT;
		vkgfxpipe_ms_info[0].sampleShadingEnable	= VK_FALSE;
		vkgfxpipe_ms_info[0].minSampleShading		= 0.0f;
		vkgfxpipe_ms_info[0].pSampleMask			= NULL;
		vkgfxpipe_ms_info[0].alphaToCoverageEnable	= VK_FALSE;
		vkgfxpipe_ms_info[0].alphaToOneEnable		= VK_FALSE;

	VkPipelineColorBlendAttachmentState vkgfxpipe_colblend_ats[1];
		vkgfxpipe_colblend_ats[0].blendEnable			= VK_FALSE;
		vkgfxpipe_colblend_ats[0].srcColorBlendFactor	= VK_BLEND_FACTOR_ZERO;
		vkgfxpipe_colblend_ats[0].dstColorBlendFactor	= VK_BLEND_FACTOR_ZERO;
		vkgfxpipe_colblend_ats[0].colorBlendOp			= VK_BLEND_OP_ADD;
		vkgfxpipe_colblend_ats[0].srcAlphaBlendFactor	= VK_BLEND_FACTOR_ZERO;
		vkgfxpipe_colblend_ats[0].dstAlphaBlendFactor	= VK_BLEND_FACTOR_ZERO;
		vkgfxpipe_colblend_ats[0].alphaBlendOp			= VK_BLEND_OP_ADD;
		vkgfxpipe_colblend_ats[0].colorWriteMask		= 15;

	VkPipelineColorBlendStateCreateInfo vkgfxpipe_colblend_info[1];
		vkgfxpipe_colblend_info[0].sType	= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	nf(&vkgfxpipe_colblend_info[0]);
		vkgfxpipe_colblend_info[0].logicOpEnable		= VK_FALSE;
		vkgfxpipe_colblend_info[0].logicOp				= VK_LOGIC_OP_NO_OP;
		vkgfxpipe_colblend_info[0].attachmentCount		= 1;
		vkgfxpipe_colblend_info[0].pAttachments			= vkgfxpipe_colblend_ats;
		vkgfxpipe_colblend_info[0].blendConstants[0]	= 1.0f;
		vkgfxpipe_colblend_info[0].blendConstants[1]	= 1.0f;
		vkgfxpipe_colblend_info[0].blendConstants[2]	= 1.0f;
		vkgfxpipe_colblend_info[0].blendConstants[3]	= 1.0f;

	VkDescriptorSetLayoutBinding vkgp_laydes_setbnd[2];
		vkgp_laydes_setbnd[0].binding				= 0;
		vkgp_laydes_setbnd[0].descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		vkgp_laydes_setbnd[0].descriptorCount		= 1;
		vkgp_laydes_setbnd[0].stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT;
		vkgp_laydes_setbnd[0].pImmutableSamplers	= NULL;
		vkgp_laydes_setbnd[1].binding				= 1;
		vkgp_laydes_setbnd[1].descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		vkgp_laydes_setbnd[1].descriptorCount		= 1;
		vkgp_laydes_setbnd[1].stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT;
		vkgp_laydes_setbnd[1].pImmutableSamplers	= NULL;

	VkDescriptorSetLayoutCreateInfo vkgp_laydes_info[1];
		vkgp_laydes_info[0].sType	= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	nf(&vkgp_laydes_info[0]);
		vkgp_laydes_info[0].bindingCount	= 2;
		vkgp_laydes_info[0].pBindings		= vkgp_laydes_setbnd;
		
	VkDescriptorSetLayout vkgp_laydes[2];
	vr("vkCreateDescriptorSetLayout", &vkres, 
		vkCreateDescriptorSetLayout(vkld[0], &vkgp_laydes_info[0], NULL, &vkgp_laydes[0]) );
	vr("vkCreateDescriptorSetLayout", &vkres, 
		vkCreateDescriptorSetLayout(vkld[0], &vkgp_laydes_info[0], NULL, &vkgp_laydes[1]) );

	VkPipelineLayoutCreateInfo vkgp_lay_info[2];
		vkgp_lay_info[0].sType	= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	nf(&vkgp_lay_info[0]);
		vkgp_lay_info[0].setLayoutCount			= 1;
		vkgp_lay_info[0].pSetLayouts			= vkgp_laydes;
		vkgp_lay_info[0].pushConstantRangeCount	= 0;
		vkgp_lay_info[0].pPushConstantRanges	= NULL;

	VkPipelineLayout vkgp_lay[1];
	vr("vkCreatePipelineLayout", &vkres, 
		vkCreatePipelineLayout(vkld[0], &vkgp_lay_info[0], NULL, &vkgp_lay[0]) );

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "RNDPASS");	/**/
	///////////////////////////////////////

		vkgfxpipe_ss_info[1].module					= vkshademod_frag[0];

	VkAttachmentDescription vkatd_init[1];
		vkatd_init[0].flags							= 0;
		vkatd_init[0].format						= vksurf_fmt[SURF_FMT].format;
		vkatd_init[0].samples						= VK_SAMPLE_COUNT_1_BIT;
		vkatd_init[0].loadOp						= VK_ATTACHMENT_LOAD_OP_CLEAR;
		vkatd_init[0].storeOp						= VK_ATTACHMENT_STORE_OP_STORE;
		vkatd_init[0].stencilLoadOp					= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkatd_init[0].stencilStoreOp				= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkatd_init[0].initialLayout					= VK_IMAGE_LAYOUT_UNDEFINED;
		vkatd_init[0].finalLayout					= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	VkAttachmentReference vkatref_init[1];
		vkatref_init[0].attachment					= 0;
		vkatref_init[0].layout						= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription vksubpass_init[1];
		vksubpass_init[0].flags						= 0;
		vksubpass_init[0].pipelineBindPoint			= VK_PIPELINE_BIND_POINT_GRAPHICS;
		vksubpass_init[0].inputAttachmentCount		= 0;
		vksubpass_init[0].pInputAttachments			= NULL;
		vksubpass_init[0].colorAttachmentCount		= 1;
		vksubpass_init[0].pColorAttachments			= &vkatref_init[0];
		vksubpass_init[0].pResolveAttachments		= NULL;
		vksubpass_init[0].pDepthStencilAttachment	= NULL;
		vksubpass_init[0].preserveAttachmentCount	= 0;
		vksubpass_init[0].pPreserveAttachments		= NULL;
	VkSubpassDependency vksubpass_init_dep[1];
		vksubpass_init_dep[0].srcSubpass			= VK_SUBPASS_EXTERNAL;
		vksubpass_init_dep[0].dstSubpass			= 0;
		vksubpass_init_dep[0].srcStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_init_dep[0].dstStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_init_dep[0].srcAccessMask			= 0;
		vksubpass_init_dep[0].dstAccessMask			= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vksubpass_init_dep[0].dependencyFlags		= 0;
	VkRenderPassCreateInfo vkrp_init_info[1];
		vkrp_init_info[0].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&vkrp_init_info[0]);
		vkrp_init_info[0].attachmentCount			= 1;
		vkrp_init_info[0].pAttachments				= vkatd_init;
		vkrp_init_info[0].subpassCount				= 1;
		vkrp_init_info[0].pSubpasses				= vksubpass_init;
		vkrp_init_info[0].dependencyCount			= 1;
		vkrp_init_info[0].pDependencies				= vksubpass_init_dep;
	VkRenderPass vkrp_init[1];
	vr("vkCreateRenderPass", &vkres, 
		vkCreateRenderPass(vkld[0], &vkrp_init_info[0], NULL, &vkrp_init[0]) );
	VkGraphicsPipelineCreateInfo vkgp_init_info[1];
		vkgp_init_info[0].sType	= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	nf(&vkgp_init_info[0]);
		vkgp_init_info[0].stageCount				= 2;
		vkgp_init_info[0].pStages					= vkgfxpipe_ss_info;
		vkgp_init_info[0].pVertexInputState			= &vkgfxpipe_vertins_info[0];
		vkgp_init_info[0].pInputAssemblyState		= &vkgfxpipe_ias_info[0];
		vkgp_init_info[0].pTessellationState		= NULL;
		vkgp_init_info[0].pViewportState			= &vkgfxpipe_viewport_info[0];
		vkgp_init_info[0].pRasterizationState		= &vkgfxpipe_rast_info[0];
		vkgp_init_info[0].pMultisampleState			= &vkgfxpipe_ms_info[0];
		vkgp_init_info[0].pDepthStencilState		= NULL;
		vkgp_init_info[0].pColorBlendState			= &vkgfxpipe_colblend_info[0];
		vkgp_init_info[0].pDynamicState				= NULL;
		vkgp_init_info[0].layout					= vkgp_lay[0];
		vkgp_init_info[0].renderPass				= vkrp_init[0];
		vkgp_init_info[0].subpass					= 0;
		vkgp_init_info[0].basePipelineHandle		= VK_NULL_HANDLE;
		vkgp_init_info[0].basePipelineIndex			= -1;
	VkPipeline vkgfxpipe_init[1];
	vr("vkCreateGraphicsPipelines", &vkres, 
		vkCreateGraphicsPipelines(
			vkld[0], VK_NULL_HANDLE, 1, vkgp_init_info, NULL, &vkgfxpipe_init[0] ) );

	VkAttachmentDescription vkatd_i2l[1];
		vkatd_i2l[0].flags							= 0;
		vkatd_i2l[0].format							= vksurf_fmt[SURF_FMT].format;
		vkatd_i2l[0].samples						= VK_SAMPLE_COUNT_1_BIT;
		vkatd_i2l[0].loadOp							= VK_ATTACHMENT_LOAD_OP_LOAD;
		vkatd_i2l[0].storeOp						= VK_ATTACHMENT_STORE_OP_STORE;
		vkatd_i2l[0].stencilLoadOp					= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkatd_i2l[0].stencilStoreOp					= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkatd_i2l[0].initialLayout					= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		vkatd_i2l[0].finalLayout					= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkAttachmentReference vkatref_i2l[1];
		vkatref_i2l[0].attachment					= 0;
		vkatref_i2l[0].layout						= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkSubpassDescription vksubpass_i2l[1];
		vksubpass_i2l[0].flags						= 0;
		vksubpass_i2l[0].pipelineBindPoint			= VK_PIPELINE_BIND_POINT_GRAPHICS;
		vksubpass_i2l[0].inputAttachmentCount		= 0;
		vksubpass_i2l[0].pInputAttachments			= NULL;
		vksubpass_i2l[0].colorAttachmentCount		= 0;
		vksubpass_i2l[0].pColorAttachments			= NULL;
		vksubpass_i2l[0].pResolveAttachments		= NULL;
		vksubpass_i2l[0].pDepthStencilAttachment	= NULL;
		vksubpass_i2l[0].preserveAttachmentCount	= 0;
		vksubpass_i2l[0].pPreserveAttachments		= NULL;
	VkSubpassDependency vksubpass_i2l_dep[1];
		vksubpass_i2l_dep[0].srcSubpass				= VK_SUBPASS_EXTERNAL;
		vksubpass_i2l_dep[0].dstSubpass				= 0;
		vksubpass_i2l_dep[0].srcStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_i2l_dep[0].dstStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_i2l_dep[0].srcAccessMask			= 0;
		vksubpass_i2l_dep[0].dstAccessMask			= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vksubpass_i2l_dep[0].dependencyFlags		= 0;
	VkRenderPassCreateInfo vkrp_i2l_info[1];
		vkrp_i2l_info[0].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&vkrp_i2l_info[0]);
		vkrp_i2l_info[0].attachmentCount			= 1;
		vkrp_i2l_info[0].pAttachments				= vkatd_i2l;
		vkrp_i2l_info[0].subpassCount				= 1;
		vkrp_i2l_info[0].pSubpasses					= vksubpass_i2l;
		vkrp_i2l_info[0].dependencyCount			= 1;
		vkrp_i2l_info[0].pDependencies				= vksubpass_i2l_dep;
	VkRenderPass vkrp_i2l[1];
	vr("vkCreateRenderPass", &vkres, 
		vkCreateRenderPass(vkld[0], &vkrp_i2l_info[0], NULL, &vkrp_i2l[0]) );
	VkGraphicsPipelineCreateInfo vkgp_i2l_info[1];
		vkgp_i2l_info[0].sType	= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	nf(&vkgp_i2l_info[0]);
		vkgp_i2l_info[0].stageCount					= 1;
		vkgp_i2l_info[0].pStages					= vkgfxpipe_ss_info;
		vkgp_i2l_info[0].pVertexInputState			= &vkgfxpipe_vertins_info[0];
		vkgp_i2l_info[0].pInputAssemblyState		= &vkgfxpipe_ias_info[0];
		vkgp_i2l_info[0].pTessellationState			= NULL;
		vkgp_i2l_info[0].pViewportState				= &vkgfxpipe_viewport_info[0];
		vkgp_i2l_info[0].pRasterizationState		= &vkgfxpipe_rast_info[0];
		vkgp_i2l_info[0].pMultisampleState			= &vkgfxpipe_ms_info[0];
		vkgp_i2l_info[0].pDepthStencilState			= NULL;
		vkgp_i2l_info[0].pColorBlendState			= &vkgfxpipe_colblend_info[0];
		vkgp_i2l_info[0].pDynamicState				= NULL;
		vkgp_i2l_info[0].layout						= vkgp_lay[0];
		vkgp_i2l_info[0].renderPass					= vkrp_i2l[0];
		vkgp_i2l_info[0].subpass					= 0;
		vkgp_i2l_info[0].basePipelineHandle			= VK_NULL_HANDLE;
		vkgp_i2l_info[0].basePipelineIndex			= -1;
	VkPipeline vkgfxpipe_i2l[1];
	vr("vkCreateGraphicsPipelines", &vkres, 
		vkCreateGraphicsPipelines(
			vkld[0], VK_NULL_HANDLE, 1, vkgp_i2l_info, NULL, &vkgfxpipe_i2l[0] ) );

		vkgfxpipe_ss_info[1].module					= vkshademod_frag[1];
	VkAttachmentDescription vkatd_loop_0[2];
		vkatd_loop_0[0].flags						= 0;
		vkatd_loop_0[0].format						= vksurf_fmt[SURF_FMT].format;
		vkatd_loop_0[0].samples						= VK_SAMPLE_COUNT_1_BIT;
		vkatd_loop_0[0].loadOp						= VK_ATTACHMENT_LOAD_OP_LOAD;
		vkatd_loop_0[0].storeOp						= VK_ATTACHMENT_STORE_OP_STORE;
		vkatd_loop_0[0].stencilLoadOp				= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkatd_loop_0[0].stencilStoreOp				= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkatd_loop_0[0].initialLayout				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		vkatd_loop_0[0].finalLayout					= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vkatd_loop_0[1].flags						= 0;
		vkatd_loop_0[1].format						= vksurf_fmt[SURF_FMT].format;
		vkatd_loop_0[1].samples						= VK_SAMPLE_COUNT_1_BIT;
		vkatd_loop_0[1].loadOp						= VK_ATTACHMENT_LOAD_OP_LOAD;
		vkatd_loop_0[1].storeOp						= VK_ATTACHMENT_STORE_OP_STORE;
		vkatd_loop_0[1].stencilLoadOp				= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkatd_loop_0[1].stencilStoreOp				= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkatd_loop_0[1].initialLayout				= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vkatd_loop_0[1].finalLayout					= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	VkAttachmentReference vkatref_loop_0[2];
		vkatref_loop_0[0].attachment				= 0;
		vkatref_loop_0[0].layout					= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vkatref_loop_0[1].attachment				= 1;
		vkatref_loop_0[1].layout					= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription vksubpass_loop_0[1];
		vksubpass_loop_0[0].flags					= 0;
		vksubpass_loop_0[0].pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		vksubpass_loop_0[0].inputAttachmentCount	= 1;
		vksubpass_loop_0[0].pInputAttachments		= &vkatref_loop_0[0];
		vksubpass_loop_0[0].colorAttachmentCount	= 1;
		vksubpass_loop_0[0].pColorAttachments		= &vkatref_loop_0[1];
		vksubpass_loop_0[0].pResolveAttachments		= NULL;
		vksubpass_loop_0[0].pDepthStencilAttachment	= NULL;
		vksubpass_loop_0[0].preserveAttachmentCount	= 0;
		vksubpass_loop_0[0].pPreserveAttachments	= NULL;
	VkSubpassDependency vksubpass_loop_0_dep[1];
		vksubpass_loop_0_dep[0].srcSubpass			= VK_SUBPASS_EXTERNAL;
		vksubpass_loop_0_dep[0].dstSubpass			= 0;
		vksubpass_loop_0_dep[0].srcStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_loop_0_dep[0].dstStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_loop_0_dep[0].srcAccessMask		= 0;
		vksubpass_loop_0_dep[0].dstAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vksubpass_loop_0_dep[0].dependencyFlags		= 0;
	VkRenderPassCreateInfo vkrp_loop_0_info[1];
		vkrp_loop_0_info[0].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&vkrp_loop_0_info[0]);
		vkrp_loop_0_info[0].attachmentCount			= 2;
		vkrp_loop_0_info[0].pAttachments			= vkatd_loop_0;
		vkrp_loop_0_info[0].subpassCount			= 1;
		vkrp_loop_0_info[0].pSubpasses				= vksubpass_loop_0;
		vkrp_loop_0_info[0].dependencyCount			= 1;
		vkrp_loop_0_info[0].pDependencies			= vksubpass_loop_0_dep;
	VkRenderPass vkrp_loop_0[1];
	vr("vkCreateRenderPass", &vkres, 
		vkCreateRenderPass(vkld[0], &vkrp_loop_0_info[0], NULL, &vkrp_loop_0[0]) );
	VkGraphicsPipelineCreateInfo vkgp_loop_0_info[1];
		vkgp_loop_0_info[0].sType	= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	nf(&vkgp_loop_0_info[0]);
		vkgp_loop_0_info[0].stageCount				= SHDRSTGS;
		vkgp_loop_0_info[0].pStages					= vkgfxpipe_ss_info;
		vkgp_loop_0_info[0].pVertexInputState		= &vkgfxpipe_vertins_info[0];
		vkgp_loop_0_info[0].pInputAssemblyState		= &vkgfxpipe_ias_info[0];
		vkgp_loop_0_info[0].pTessellationState		= NULL;
		vkgp_loop_0_info[0].pViewportState			= &vkgfxpipe_viewport_info[0];
		vkgp_loop_0_info[0].pRasterizationState		= &vkgfxpipe_rast_info[0];
		vkgp_loop_0_info[0].pMultisampleState		= &vkgfxpipe_ms_info[0];
		vkgp_loop_0_info[0].pDepthStencilState		= NULL;
		vkgp_loop_0_info[0].pColorBlendState		= &vkgfxpipe_colblend_info[0];
		vkgp_loop_0_info[0].pDynamicState			= NULL;
		vkgp_loop_0_info[0].layout					= vkgp_lay[0];
		vkgp_loop_0_info[0].renderPass				= vkrp_loop_0[0];
		vkgp_loop_0_info[0].subpass					= 0;
		vkgp_loop_0_info[0].basePipelineHandle		= VK_NULL_HANDLE;
		vkgp_loop_0_info[0].basePipelineIndex		= -1;
	VkPipeline vkgfxpipe_loop_0[1];
	vr("vkCreateGraphicsPipelines", &vkres, 
		vkCreateGraphicsPipelines(
			vkld[0], VK_NULL_HANDLE, 1, vkgp_loop_0_info, NULL, &vkgfxpipe_loop_0[0] ) );

	VkAttachmentDescription vkatd_loop_1[2];
		vkatd_loop_1[0].flags						= 0;
		vkatd_loop_1[0].format						= vksurf_fmt[SURF_FMT].format;
		vkatd_loop_1[0].samples						= VK_SAMPLE_COUNT_1_BIT;
		vkatd_loop_1[0].loadOp						= VK_ATTACHMENT_LOAD_OP_LOAD;
		vkatd_loop_1[0].storeOp						= VK_ATTACHMENT_STORE_OP_STORE;
		vkatd_loop_1[0].stencilLoadOp				= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkatd_loop_1[0].stencilStoreOp				= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkatd_loop_1[0].initialLayout				= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vkatd_loop_1[0].finalLayout					= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		vkatd_loop_1[1].flags						= 0;
		vkatd_loop_1[1].format						= vksurf_fmt[SURF_FMT].format;
		vkatd_loop_1[1].samples						= VK_SAMPLE_COUNT_1_BIT;
		vkatd_loop_1[1].loadOp						= VK_ATTACHMENT_LOAD_OP_LOAD;
		vkatd_loop_1[1].storeOp						= VK_ATTACHMENT_STORE_OP_STORE;
		vkatd_loop_1[1].stencilLoadOp				= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkatd_loop_1[1].stencilStoreOp				= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkatd_loop_1[1].initialLayout				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		vkatd_loop_1[1].finalLayout					= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkAttachmentReference vkatref_loop_1[2];
		vkatref_loop_1[0].attachment				= 0;
		vkatref_loop_1[0].layout					= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		vkatref_loop_1[1].attachment				= 1;
		vkatref_loop_1[1].layout					= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkSubpassDescription vksubpass_loop_1[1];
		vksubpass_loop_1[0].flags					= 0;
		vksubpass_loop_1[0].pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		vksubpass_loop_1[0].inputAttachmentCount	= 1;
		vksubpass_loop_1[0].pInputAttachments		= &vkatref_loop_1[1];
		vksubpass_loop_1[0].colorAttachmentCount	= 1;
		vksubpass_loop_1[0].pColorAttachments		= &vkatref_loop_1[0];
		vksubpass_loop_1[0].pResolveAttachments		= NULL;
		vksubpass_loop_1[0].pDepthStencilAttachment	= NULL;
		vksubpass_loop_1[0].preserveAttachmentCount	= 0;
		vksubpass_loop_1[0].pPreserveAttachments	= NULL;
	VkSubpassDependency vksubpass_loop_1_dep[1];
		vksubpass_loop_1_dep[0].srcSubpass			= VK_SUBPASS_EXTERNAL;
		vksubpass_loop_1_dep[0].dstSubpass			= 0;
		vksubpass_loop_1_dep[0].srcStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_loop_1_dep[0].dstStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_loop_1_dep[0].srcAccessMask		= 0;
		vksubpass_loop_1_dep[0].dstAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vksubpass_loop_1_dep[0].dependencyFlags		= 0;
	VkRenderPassCreateInfo vkrp_loop_1_info[1];
		vkrp_loop_1_info[0].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&vkrp_loop_1_info[0]);
		vkrp_loop_1_info[0].attachmentCount			= 2;
		vkrp_loop_1_info[0].pAttachments			= vkatd_loop_1;
		vkrp_loop_1_info[0].subpassCount			= 1;
		vkrp_loop_1_info[0].pSubpasses				= vksubpass_loop_1;
		vkrp_loop_1_info[0].dependencyCount			= 1;
		vkrp_loop_1_info[0].pDependencies			= vksubpass_loop_1_dep;
	VkRenderPass vkrp_loop_1[1];
	vr("vkCreateRenderPass", &vkres, 
		vkCreateRenderPass(vkld[0], &vkrp_loop_1_info[0], NULL, &vkrp_loop_1[0]) );
	VkGraphicsPipelineCreateInfo vkgp_loop_1_info[1];
		vkgp_loop_1_info[0].sType	= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	nf(&vkgp_loop_1_info[0]);
		vkgp_loop_1_info[0].stageCount				= SHDRSTGS;
		vkgp_loop_1_info[0].pStages					= vkgfxpipe_ss_info;
		vkgp_loop_1_info[0].pVertexInputState		= &vkgfxpipe_vertins_info[0];
		vkgp_loop_1_info[0].pInputAssemblyState		= &vkgfxpipe_ias_info[0];
		vkgp_loop_1_info[0].pTessellationState		= NULL;
		vkgp_loop_1_info[0].pViewportState			= &vkgfxpipe_viewport_info[0];
		vkgp_loop_1_info[0].pRasterizationState		= &vkgfxpipe_rast_info[0];
		vkgp_loop_1_info[0].pMultisampleState		= &vkgfxpipe_ms_info[0];
		vkgp_loop_1_info[0].pDepthStencilState		= NULL;
		vkgp_loop_1_info[0].pColorBlendState		= &vkgfxpipe_colblend_info[0];
		vkgp_loop_1_info[0].pDynamicState			= NULL;
		vkgp_loop_1_info[0].layout					= vkgp_lay[0];
		vkgp_loop_1_info[0].renderPass				= vkrp_loop_1[0];
		vkgp_loop_1_info[0].subpass					= 0;
		vkgp_loop_1_info[0].basePipelineHandle		= VK_NULL_HANDLE;
		vkgp_loop_1_info[0].basePipelineIndex		= -1;
	VkPipeline vkgfxpipe_loop_1[1];
	vr("vkCreateGraphicsPipelines", &vkres, 
		vkCreateGraphicsPipelines(
			vkld[0], VK_NULL_HANDLE, 1, vkgp_loop_1_info, NULL, &vkgfxpipe_loop_1[0] ) );

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "BUFFERS");	/**/
	///////////////////////////////////////

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
	vr("vkCreateSwapchainKHR", &vkres, 
		vkCreateSwapchainKHR(vkld[0], &vkswap_info[0], NULL, &vkswap[0]) );

	uint32_t swap_img_cnt = UINT32_MAX;
	vr("vkGetSwapchainImagesKHR", &vkres, 
		vkGetSwapchainImagesKHR(vkld[0], vkswap[0], &swap_img_cnt, NULL) );
		ov("SwapchainImages", swap_img_cnt);
	VkImage vkswap_img[swap_img_cnt];
	vr("vkGetSwapchainImagesKHR", &vkres, 
		vkGetSwapchainImagesKHR(vkld[0], vkswap[0], &swap_img_cnt, vkswap_img) );

	VkImageSubresourceRange vkimg_subres[1];
		vkimg_subres[0].aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		vkimg_subres[0].baseMipLevel	= 0;
		vkimg_subres[0].levelCount		= 1;
		vkimg_subres[0].baseArrayLayer	= 0;
		vkimg_subres[0].layerCount		= 1;

	VkImageViewCreateInfo vkimgview_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
			vkimgview_info[i].sType	= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		nf(&vkimgview_info[i]);
			vkimgview_info[i].image				= vkswap_img[i];
			vkimgview_info[i].viewType			= VK_IMAGE_VIEW_TYPE_2D;
			vkimgview_info[i].format			= vksurf_fmt[SURF_FMT].format;
			vkimgview_info[i].components.r		= VK_COMPONENT_SWIZZLE_IDENTITY;
			vkimgview_info[i].components.g		= VK_COMPONENT_SWIZZLE_IDENTITY;
			vkimgview_info[i].components.b		= VK_COMPONENT_SWIZZLE_IDENTITY;
			vkimgview_info[i].components.a		= VK_COMPONENT_SWIZZLE_IDENTITY;
			vkimgview_info[i].subresourceRange	= vkimg_subres[0];
	}

	VkImageView vkimgview[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkCreateImageView", &vkres, 
			vkCreateImageView(vkld[0], &vkimgview_info[i], NULL, &vkimgview[i]) );
	}

	VkFramebufferCreateInfo vkfbuf_init_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
			vkfbuf_init_info[i].sType	= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		nf(&vkfbuf_init_info[i]);
			vkfbuf_init_info[i].renderPass		= vkrp_init[0];
			vkfbuf_init_info[i].attachmentCount	= vkrp_init_info[0].attachmentCount;
			vkfbuf_init_info[i].pAttachments	= &vkimgview[i];
			vkfbuf_init_info[i].width			= vksurf_ables[0].currentExtent.width;
			vkfbuf_init_info[i].height			= vksurf_ables[0].currentExtent.height;
			vkfbuf_init_info[i].layers			= 1;
	}
	VkFramebuffer vkfbuf_init[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkCreateFramebuffer", &vkres, 
			vkCreateFramebuffer(vkld[0], &vkfbuf_init_info[i], NULL, &vkfbuf_init[i]) );
	}
	VkFramebufferCreateInfo vkfbuf_i2l_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
			vkfbuf_i2l_info[i].sType	= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		nf(&vkfbuf_i2l_info[i]);
			vkfbuf_i2l_info[i].renderPass		= vkrp_i2l[0];
			vkfbuf_i2l_info[i].attachmentCount	= vkrp_i2l_info[0].attachmentCount;
			vkfbuf_i2l_info[i].pAttachments		= &vkimgview[i];
			vkfbuf_i2l_info[i].width			= vksurf_ables[0].currentExtent.width;
			vkfbuf_i2l_info[i].height			= vksurf_ables[0].currentExtent.height;
			vkfbuf_i2l_info[i].layers			= 1;
	}
	VkFramebuffer vkfbuf_i2l[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkCreateFramebuffer", &vkres, 
			vkCreateFramebuffer(vkld[0], &vkfbuf_i2l_info[i], NULL, &vkfbuf_i2l[i]) );
	}

	VkFramebufferCreateInfo vkfbuf_loop_0_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
			vkfbuf_loop_0_info[i].sType	= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		nf(&vkfbuf_loop_0_info[i]);
			vkfbuf_loop_0_info[i].renderPass		= vkrp_loop_0[0];
			vkfbuf_loop_0_info[i].attachmentCount	= vkrp_loop_0_info[0].attachmentCount;
			vkfbuf_loop_0_info[i].pAttachments		= vkimgview;
			vkfbuf_loop_0_info[i].width				= vksurf_ables[0].currentExtent.width;
			vkfbuf_loop_0_info[i].height			= vksurf_ables[0].currentExtent.height;
			vkfbuf_loop_0_info[i].layers			= 1;
	}
	VkFramebuffer vkfbuf_loop_0[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkCreateFramebuffer", &vkres, 
			vkCreateFramebuffer(vkld[0], &vkfbuf_loop_0_info[i], NULL, &vkfbuf_loop_0[i]) );
	}
	VkFramebufferCreateInfo vkfbuf_loop_1_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
			vkfbuf_loop_1_info[i].sType	= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		nf(&vkfbuf_loop_1_info[i]);
			vkfbuf_loop_1_info[i].renderPass		= vkrp_loop_1[0];
			vkfbuf_loop_1_info[i].attachmentCount	= vkrp_loop_1_info[0].attachmentCount;
			vkfbuf_loop_1_info[i].pAttachments		= vkimgview;
			vkfbuf_loop_1_info[i].width				= vksurf_ables[0].currentExtent.width;
			vkfbuf_loop_1_info[i].height			= vksurf_ables[0].currentExtent.height;
			vkfbuf_loop_1_info[i].layers			= 1;
	}
	VkFramebuffer vkfbuf_loop_1[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkCreateFramebuffer", &vkres, 
			vkCreateFramebuffer(vkld[0], &vkfbuf_loop_1_info[i], NULL, &vkfbuf_loop_1[i]) );
	}

	VkClearValue vkclear_col_init[1];
		vkclear_col_init[0].color	= { 0.3f, 0.0f, 0.3f, 1.0f };
	VkClearValue vkclear_col_i2l[1];
		vkclear_col_i2l[0].color	= { 0.3f, 0.3f, 0.3f, 1.0f };
	VkClearValue vkclear_col_loop_0[1];
		vkclear_col_loop_0[0].color	= { 0.0f, 0.2f, 0.3f, 1.0f };
	VkClearValue vkclear_col_loop_1[1];
		vkclear_col_loop_1[0].color	= { 0.6f, 0.3f, 0.0f, 1.0f };

	VkRenderPassBeginInfo vkrpbegin_init_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vkrpbegin_init_info[i].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegin_init_info[i].pNext	= NULL;
		vkrpbegin_init_info[i].renderPass		= vkrp_init[0];
		vkrpbegin_init_info[i].framebuffer		= vkfbuf_init[i];
		vkrpbegin_init_info[i].renderArea		= vkgfxpipe_sciz[0];
		vkrpbegin_init_info[i].clearValueCount	= 1;
		vkrpbegin_init_info[i].pClearValues		= vkclear_col_init;
	}
	VkRenderPassBeginInfo vkrpbegin_i2l_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vkrpbegin_i2l_info[i].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegin_i2l_info[i].pNext	= NULL;
		vkrpbegin_i2l_info[i].renderPass		= vkrp_i2l[0];
		vkrpbegin_i2l_info[i].framebuffer		= vkfbuf_i2l[i];
		vkrpbegin_i2l_info[i].renderArea		= vkgfxpipe_sciz[0];
		vkrpbegin_i2l_info[i].clearValueCount	= 1;
		vkrpbegin_i2l_info[i].pClearValues		= vkclear_col_i2l;
	}

	VkRenderPassBeginInfo vkrpbegin_loop_0_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vkrpbegin_loop_0_info[i].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegin_loop_0_info[i].pNext	= NULL;
		vkrpbegin_loop_0_info[i].renderPass			= vkrp_loop_0[0];
		vkrpbegin_loop_0_info[i].framebuffer		= vkfbuf_loop_0[i];
		vkrpbegin_loop_0_info[i].renderArea			= vkgfxpipe_sciz[0];
		vkrpbegin_loop_0_info[i].clearValueCount	= 1;
		vkrpbegin_loop_0_info[i].pClearValues		= vkclear_col_loop_0;
	}
	VkRenderPassBeginInfo vkrpbegin_loop_1_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vkrpbegin_loop_1_info[i].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegin_loop_1_info[i].pNext	= NULL;
		vkrpbegin_loop_1_info[i].renderPass			= vkrp_loop_1[0];
		vkrpbegin_loop_1_info[i].framebuffer		= vkfbuf_loop_1[i];
		vkrpbegin_loop_1_info[i].renderArea			= vkgfxpipe_sciz[0];
		vkrpbegin_loop_1_info[i].clearValueCount	= 1;
		vkrpbegin_loop_1_info[i].pClearValues		= vkclear_col_loop_1;
	}

	VkCommandPoolCreateInfo vkcompool_info[1];
		vkcompool_info[0].sType	= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	nf(&vkcompool_info[0]);
		vkcompool_info[0].queueFamilyIndex	= GQF_IDX;

	VkCommandPool vkcompool[1];
	vr("vkCreateCommandPool", &vkres, 
		vkCreateCommandPool(vkld[0], &vkcompool_info[0], NULL, &vkcompool[0]) );

	VkCommandBufferAllocateInfo vkcombuf_alloc_info[1];
		vkcombuf_alloc_info[0].sType	= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		vkcombuf_alloc_info[0].pNext	= NULL;
		vkcombuf_alloc_info[0].commandPool			= vkcompool[0];
		vkcombuf_alloc_info[0].level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		vkcombuf_alloc_info[0].commandBufferCount	= swap_img_cnt;

	VkCommandBuffer vkcombuf_init[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkAllocateCommandBuffers", &vkres, 
			vkAllocateCommandBuffers(vkld[0], &vkcombuf_alloc_info[0], vkcombuf_init) );
	}

	VkCommandBuffer vkcombuf_i2l[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkAllocateCommandBuffers", &vkres, 
			vkAllocateCommandBuffers(vkld[0], &vkcombuf_alloc_info[0], vkcombuf_i2l) );
	}

	VkCommandBuffer vkcombuf_loop[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkAllocateCommandBuffers", &vkres, 
			vkAllocateCommandBuffers(vkld[0], &vkcombuf_alloc_info[0], vkcombuf_loop) );
	}

	VkCommandBufferBeginInfo vkcombufbegin_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
			vkcombufbegin_info[i].sType	= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		nf(&vkcombufbegin_info[i]);
			vkcombufbegin_info[i].pInheritanceInfo	= NULL;
	}

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "DESCRIP");	/**/
	///////////////////////////////////////

	VkSamplerCreateInfo vksmplr_info[1];
		vksmplr_info[0].sType	= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	nf(&vksmplr_info[0]);
		vksmplr_info[0].magFilter				= VK_FILTER_NEAREST;
		vksmplr_info[0].minFilter				= VK_FILTER_NEAREST;
		vksmplr_info[0].mipmapMode				= VK_SAMPLER_MIPMAP_MODE_NEAREST;
		vksmplr_info[0].addressModeU			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		vksmplr_info[0].addressModeV			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		vksmplr_info[0].addressModeW			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		vksmplr_info[0].mipLodBias				= 1.0f;
		vksmplr_info[0].anisotropyEnable		= VK_FALSE;
		vksmplr_info[0].maxAnisotropy			= 1.0f;
		vksmplr_info[0].compareEnable			= VK_FALSE;
		vksmplr_info[0].compareOp				= VK_COMPARE_OP_NEVER;
		vksmplr_info[0].minLod					= 1.0f;
		vksmplr_info[0].maxLod					= 1.0f;
		vksmplr_info[0].borderColor				= VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		vksmplr_info[0].unnormalizedCoordinates	= VK_FALSE;

	VkSampler vksmplr[1];
	vr("vkCreateSampler", &vkres, 
		vkCreateSampler(vkld[0], &vksmplr_info[0], NULL, &vksmplr[0]) );

	VkDescriptorPoolSize vkdescpool_size[2];
		vkdescpool_size[0].type				= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		vkdescpool_size[0].descriptorCount	= 1;
		vkdescpool_size[1].type				= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		vkdescpool_size[1].descriptorCount	= 1;

	VkDescriptorPoolCreateInfo vkdescpool_info[1];
		vkdescpool_info[0].sType	= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	nf(&vkdescpool_info[0]);
		vkdescpool_info[0].maxSets			= 1;
		vkdescpool_info[0].poolSizeCount	= 2;
		vkdescpool_info[0].pPoolSizes		= vkdescpool_size;

	VkDescriptorPool vkdescpool[2];
	vr("vkCreateDescriptorPool", &vkres, 
		vkCreateDescriptorPool(vkld[0], &vkdescpool_info[0], NULL, &vkdescpool[0]) );
	vr("vkCreateDescriptorPool", &vkres, 
		vkCreateDescriptorPool(vkld[0], &vkdescpool_info[0], NULL, &vkdescpool[1]) );

	VkDescriptorSetAllocateInfo vkdescset_alloc_info[1];
		vkdescset_alloc_info[0].sType	= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		vkdescset_alloc_info[0].pNext	= NULL;
		vkdescset_alloc_info[0].descriptorPool		= vkdescpool[0];
		vkdescset_alloc_info[0].descriptorSetCount	= vkdescpool_info[0].maxSets;
		vkdescset_alloc_info[0].pSetLayouts			= vkgp_laydes;
		vkdescset_alloc_info[1].sType	= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		vkdescset_alloc_info[1].pNext	= NULL;
		vkdescset_alloc_info[1].descriptorPool		= vkdescpool[1];
		vkdescset_alloc_info[1].descriptorSetCount	= vkdescpool_info[0].maxSets;
		vkdescset_alloc_info[1].pSetLayouts			= vkgp_laydes;

	VkDescriptorSet vkdescset_0[1];
	vr("vkAllocateDescriptorSets", &vkres, 
		vkAllocateDescriptorSets(vkld[0], &vkdescset_alloc_info[0], &vkdescset_0[0]) );
	VkDescriptorSet vkdescset_1[1];
	vr("vkAllocateDescriptorSets", &vkres, 
		vkAllocateDescriptorSets(vkld[0], &vkdescset_alloc_info[1], &vkdescset_1[0]) );

	VkDescriptorImageInfo vkdesc_img_info_0[1];
		vkdesc_img_info_0[0].sampler		= vksmplr[0];
		vkdesc_img_info_0[0].imageView		= vkimgview[1];
		vkdesc_img_info_0[0].imageLayout	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkDescriptorImageInfo vkdesc_img_info_1[1];
		vkdesc_img_info_1[0].sampler		= vksmplr[0];
		vkdesc_img_info_1[0].imageView		= vkimgview[0];
		vkdesc_img_info_1[0].imageLayout	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet vkdescset_write_0[2];
		vkdescset_write_0[0].sType	= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vkdescset_write_0[0].pNext	= NULL;
		vkdescset_write_0[0].dstSet				= vkdescset_0[0];
		vkdescset_write_0[0].dstBinding			= 0;
		vkdescset_write_0[0].dstArrayElement	= 0;
		vkdescset_write_0[0].descriptorCount	= 1;
		vkdescset_write_0[0].descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		vkdescset_write_0[0].pImageInfo			= &vkdesc_img_info_0[0];
		vkdescset_write_0[0].pBufferInfo		= NULL;
		vkdescset_write_0[0].pTexelBufferView	= NULL;

	VkWriteDescriptorSet vkdescset_write_1[2];
		vkdescset_write_1[0].sType	= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vkdescset_write_1[0].pNext	= NULL;
		vkdescset_write_1[0].dstSet				= vkdescset_1[0];
		vkdescset_write_1[0].dstBinding			= 0;
		vkdescset_write_1[0].dstArrayElement	= 0;
		vkdescset_write_1[0].descriptorCount	= 1;
		vkdescset_write_1[0].descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		vkdescset_write_1[0].pImageInfo			= &vkdesc_img_info_1[0];
		vkdescset_write_1[0].pBufferInfo		= NULL;
		vkdescset_write_1[0].pTexelBufferView	= NULL;

	VkDeviceSize vk_devsz_buff[1];
		vk_devsz_buff[0]	= sizeof(float) * 3;
		ov("Uniform init_ub size", vk_devsz_buff[0]);
		vk_devsz_buff[0] = (vk_devsz_buff[0] > 256 ? vk_devsz_buff[0] : 256);
		ov("Uniform init_ub size", vk_devsz_buff[0]);

	VkBufferCreateInfo vkbuff_ub_info[1];
		vkbuff_ub_info[0].sType	= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	nf(&vkbuff_ub_info[0]);
		vkbuff_ub_info[0].size						= vk_devsz_buff[0];
		vkbuff_ub_info[0].usage						= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		vkbuff_ub_info[0].sharingMode				= vkswap_info[0].imageSharingMode;
		vkbuff_ub_info[0].queueFamilyIndexCount		= 0;
		vkbuff_ub_info[0].pQueueFamilyIndices		= NULL;

	VkBuffer vkbuff_ub[1];
	vr("vkCreateBuffer", &vkres, 
		vkCreateBuffer(vkld[0], &vkbuff_ub_info[0], NULL, &vkbuff_ub[0]) );

	VkMemoryRequirements vkmemreq_vkbuff_ub[1];
	rv("vkGetBufferMemoryRequirements");
		vkGetBufferMemoryRequirements(vkld[0], vkbuff_ub[0], &vkmemreq_vkbuff_ub[0]);
		ov("Uniform init_u0 size", 				vkmemreq_vkbuff_ub[0].size				);
		ov("Uniform init_u0 alignment", 		vkmemreq_vkbuff_ub[0].alignment			);
		ov("Uniform init_u0 memoryTypeBits", 	vkmemreq_vkbuff_ub[0].memoryTypeBits	);

	VkDescriptorBufferInfo vkdescbuf_ub_info[1];
		vkdescbuf_ub_info[0].buffer		= vkbuff_ub[0];
		vkdescbuf_ub_info[0].offset		= 0;
		vkdescbuf_ub_info[0].range		= vkmemreq_vkbuff_ub[0].size;

		vkdescset_write_0[1].sType	= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vkdescset_write_0[1].pNext	= NULL;
		vkdescset_write_0[1].dstSet				= vkdescset_0[0];
		vkdescset_write_0[1].dstBinding			= 1;
		vkdescset_write_0[1].dstArrayElement	= 0;
		vkdescset_write_0[1].descriptorCount	= 1;
		vkdescset_write_0[1].descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		vkdescset_write_0[1].pImageInfo			= NULL;
		vkdescset_write_0[1].pBufferInfo		= vkdescbuf_ub_info;
		vkdescset_write_0[1].pTexelBufferView	= NULL;

		vkdescset_write_1[1].sType	= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vkdescset_write_1[1].pNext	= NULL;
		vkdescset_write_1[1].dstSet				= vkdescset_1[0];
		vkdescset_write_1[1].dstBinding			= 1;
		vkdescset_write_1[1].dstArrayElement	= 0;
		vkdescset_write_1[1].descriptorCount	= 1;
		vkdescset_write_1[1].descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		vkdescset_write_1[1].pImageInfo			= NULL;
		vkdescset_write_1[1].pBufferInfo		= vkdescbuf_ub_info;
		vkdescset_write_1[1].pTexelBufferView	= NULL;

	VkPhysicalDeviceMemoryProperties vkpd_memprops[1];
	rv("vkGetPhysicalDeviceMemoryProperties");
		vkGetPhysicalDeviceMemoryProperties(vkpd[PD_IDX], &vkpd_memprops[0]);
		ov("memoryTypeCount", 	vkpd_memprops[0].memoryTypeCount					);
		for(int i = 0; i < 		vkpd_memprops[0].memoryTypeCount; 				i++	) {
			iv("propertyFlags", vkpd_memprops[0].memoryTypes[i].propertyFlags,	i	); 
			iv("heapIndex", 	vkpd_memprops[0].memoryTypes[i].heapIndex, 		i	); }
		ov("memoryHeapCount", 	vkpd_memprops[0].memoryHeapCount				);
		for(int i = 0; i < 		vkpd_memprops[0].memoryHeapCount; 				i++	) {
			iv("size", 			vkpd_memprops[0].memoryHeaps[i].size,			i	); 
			iv("flags", 		vkpd_memprops[0].memoryHeaps[i].flags,			i	); }

		for(int i = 0; i < 		vkpd_memprops[0].memoryTypeCount; 				i++	) {
			if( vkpd_memprops[0].memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			&&	UB_MEMTYPE == UINT32_MAX ) {
				UB_MEMTYPE = i; } }

	VkMemoryAllocateInfo vkbuff_ub_memallo_info[1];
		vkbuff_ub_memallo_info[0].sType	= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		vkbuff_ub_memallo_info[0].pNext	= NULL;
		vkbuff_ub_memallo_info[0].allocationSize	= vk_devsz_buff[0];
		vkbuff_ub_memallo_info[0].memoryTypeIndex	= UB_MEMTYPE;

	VkDeviceMemory vkbuff_ub_devmem[1];
	vr("vkAllocateMemory", &vkres, 
		vkAllocateMemory(vkld[0], &vkbuff_ub_memallo_info[0], NULL, &vkbuff_ub_devmem[0]) );

	void *pvoid_memmap;
	vr("vkMapMemory", &vkres, 
		vkMapMemory(vkld[0], vkbuff_ub_devmem[0], 
					vkdescbuf_ub_info[0].offset, 
					vkdescbuf_ub_info[0].range,
					0, &pvoid_memmap) );

	init_ub ubo_init_ub[1];
		ubo_init_ub[0].w 	= float(APP_W);
		ubo_init_ub[0].h 	= float(APP_H);
		ubo_init_ub[0].seed = float(1.0);

	ov("sizeof(ubo_init_ub[0])", sizeof(ubo_init_ub[0]));

	rv("memcpy");
		memcpy(pvoid_memmap, &ubo_init_ub[0], sizeof(ubo_init_ub[0]));

	vr("vkBindBufferMemory", &vkres, 
		vkBindBufferMemory(vkld[0], vkbuff_ub[0], vkbuff_ub_devmem[0], vkdescbuf_ub_info[0].offset) );

	rv("vkUpdateDescriptorSets");
		vkUpdateDescriptorSets(vkld[0], 2, vkdescset_write_0, 0, NULL);
	rv("vkUpdateDescriptorSets");
		vkUpdateDescriptorSets(vkld[0], 2, vkdescset_write_1, 0, NULL);

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "CMDINIT");	/**/
	///////////////////////////////////////

	for(int i = 0; i < swap_img_cnt; i++) {

		vr("vkBeginCommandBuffer", &vkres, 
			vkBeginCommandBuffer(vkcombuf_init[i], &vkcombufbegin_info[i]) );

			rv("vkCmdBeginRenderPass");
				vkCmdBeginRenderPass (
					vkcombuf_init[i], &vkrpbegin_init_info[i], VK_SUBPASS_CONTENTS_INLINE );

				rv("vkCmdBindPipeline");
					vkCmdBindPipeline (
						vkcombuf_init[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkgfxpipe_init[0] );

				rv("vkCmdBindDescriptorSets");
					vkCmdBindDescriptorSets (
						vkcombuf_init[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkgp_lay[0],
						0, 1, vkdescset_0, 0, NULL );

				rv("vkCmdDraw");
					vkCmdDraw (
						vkcombuf_init[i], 3, 1, 0, 0 );

			rv("vkCmdEndRenderPass");
				vkCmdEndRenderPass(vkcombuf_init[i]);

		vr("vkEndCommandBuffer", &vkres, 
			vkEndCommandBuffer(vkcombuf_init[i]) );

	}

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "SYNC");		/**/
	///////////////////////////////////////

	VkQueue vkq[1];
	rv("vkGetDeviceQueue");
		vkGetDeviceQueue(vkld[0], GQF_IDX, 0, &vkq[0]);

	VkSemaphoreCreateInfo vksemaph_info[1];
		vksemaph_info[0].sType	= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	nf(&vksemaph_info[0]);

	VkSemaphore vksemaph_image[1];
	VkSemaphore vksemaph_rendr[1];

	VkFenceCreateInfo vkfence_info[1];
		vkfence_info[0].sType	= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	nf(&vkfence_info[0]);

	VkFence vkfence_aqimg[1];
	vr("vkCreateFence", &vkres, 
		vkCreateFence(vkld[0], &vkfence_info[0], NULL, &vkfence_aqimg[0]) );

	VkPipelineStageFlags qsubwait	= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	uint32_t aqimg_idx[1];

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "DO_INIT");	/**/
	///////////////////////////////////////

	for(int i = 0; i < swap_img_cnt; i++) {
		if(valid) {

			rv("nanosleep(NS_DELAY)");
				nanosleep((const struct timespec[]){{0, NS_DELAY}}, NULL);

			VkSubmitInfo vksub_info[1];
				vksub_info[0].sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO;
				vksub_info[0].pNext	= NULL;
				vksub_info[0].waitSemaphoreCount	= 0;
				vksub_info[0].pWaitSemaphores		= NULL;
				vksub_info[0].pWaitDstStageMask		= NULL;
				vksub_info[0].commandBufferCount	= 1;
				vksub_info[0].pCommandBuffers		= &vkcombuf_init[i];
				vksub_info[0].signalSemaphoreCount	= 0;
				vksub_info[0].pSignalSemaphores		= NULL;

			vr("vkQueueSubmit", &vkres, 
				vkQueueSubmit(vkq[0], 1, vksub_info, VK_NULL_HANDLE) );
		}
	}

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "CMDI2L");		/**/
	///////////////////////////////////////

	vr("vkBeginCommandBuffer", &vkres, 
		vkBeginCommandBuffer(vkcombuf_i2l[0], &vkcombufbegin_info[0]) );

		rv("vkCmdBeginRenderPass");
			vkCmdBeginRenderPass (
				vkcombuf_i2l[0], &vkrpbegin_i2l_info[0], VK_SUBPASS_CONTENTS_INLINE );

		rv("vkCmdEndRenderPass");
			vkCmdEndRenderPass(vkcombuf_i2l[0]);

	vr("vkEndCommandBuffer", &vkres, 
		vkEndCommandBuffer(vkcombuf_i2l[0]) );
/**
	vr("vkBeginCommandBuffer", &vkres, 
		vkBeginCommandBuffer(vkcombuf_i2l[1], &vkcombufbegin_info[1]) );

		rv("vkCmdBeginRenderPass");
			vkCmdBeginRenderPass (
				vkcombuf_i2l[1], &vkrpbegin_i2l_info[1], VK_SUBPASS_CONTENTS_INLINE );

		rv("vkCmdEndRenderPass");
			vkCmdEndRenderPass(vkcombuf_i2l[1]);

	vr("vkEndCommandBuffer", &vkres, 
		vkEndCommandBuffer(vkcombuf_i2l[1]) );
/**/
	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "DO_I2L");		/**/
	///////////////////////////////////////

	for(int i = 0; i < 1; i++) {
		if(valid) {
			rv("nanosleep(NS_DELAY)");
				nanosleep((const struct timespec[]){{0, NS_DELAY}}, NULL);

			VkSubmitInfo vksub_info[1];
				vksub_info[0].sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO;
				vksub_info[0].pNext	= NULL;
				vksub_info[0].waitSemaphoreCount	= 0;
				vksub_info[0].pWaitSemaphores		= NULL;
				vksub_info[0].pWaitDstStageMask		= NULL;
				vksub_info[0].commandBufferCount	= 1;
				vksub_info[0].pCommandBuffers		= &vkcombuf_i2l[i];
				vksub_info[0].signalSemaphoreCount	= 0;
				vksub_info[0].pSignalSemaphores		= NULL;

			vr("vkQueueSubmit", &vkres, 
				vkQueueSubmit(vkq[0], 1, vksub_info, VK_NULL_HANDLE) );
		}
	}

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "CMDLOOP");	/**/
	///////////////////////////////////////

	for(int i = 0; i < 1; i++) {

		vr("vkBeginCommandBuffer", &vkres, 
			vkBeginCommandBuffer(vkcombuf_loop[i], &vkcombufbegin_info[i]) );

			rv("vkCmdBeginRenderPass");
				vkCmdBeginRenderPass (
					vkcombuf_loop[i], &vkrpbegin_loop_1_info[i], VK_SUBPASS_CONTENTS_INLINE );

				rv("vkCmdBindPipeline");
					vkCmdBindPipeline (
						vkcombuf_loop[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkgfxpipe_loop_1[0] );

				rv("vkCmdBindDescriptorSets");
					vkCmdBindDescriptorSets (
						vkcombuf_loop[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkgp_lay[0],
						0, 1, vkdescset_0, 0, NULL );

				rv("vkCmdDraw");
					vkCmdDraw (
						vkcombuf_loop[i], 3, 1, 0, 0 );

			rv("vkCmdEndRenderPass");
				vkCmdEndRenderPass(vkcombuf_loop[i]);

		vr("vkEndCommandBuffer", &vkres, 
			vkEndCommandBuffer(vkcombuf_loop[i]) );

	}

	for(int i = 1; i < 2; i++) {

		vr("vkBeginCommandBuffer", &vkres, 
			vkBeginCommandBuffer(vkcombuf_loop[i], &vkcombufbegin_info[i]) );

			rv("vkCmdBeginRenderPass");
				vkCmdBeginRenderPass (
					vkcombuf_loop[i], &vkrpbegin_loop_0_info[i], VK_SUBPASS_CONTENTS_INLINE );

				rv("vkCmdBindPipeline");
					vkCmdBindPipeline (
						vkcombuf_loop[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkgfxpipe_loop_0[0] );

				rv("vkCmdBindDescriptorSets");
					vkCmdBindDescriptorSets (
						vkcombuf_loop[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkgp_lay[0],
						0, 1, vkdescset_1, 0, NULL );

				rv("vkCmdDraw");
					vkCmdDraw (
						vkcombuf_loop[i], 3, 1, 0, 0 );

			rv("vkCmdEndRenderPass");
				vkCmdEndRenderPass(vkcombuf_loop[i]);

		vr("vkEndCommandBuffer", &vkres, 
			vkEndCommandBuffer(vkcombuf_loop[i]) );

	}

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "DO_LOOP");	/**/
	///////////////////////////////////////

//	for(int i = 0; i < swap_img_cnt * TEST_CYCLES; i++) {
	int i = 0;
	do {
		if(valid) {
			rv("nanosleep(NS_DELAY)");
				nanosleep((const struct timespec[]){{0, NS_DELAY}}, NULL);

			vr("vkResetFences", &vkres, 
				vkResetFences(vkld[0], 1, vkfence_aqimg) );

			vr("vkCreateSemaphore", &vkres, 
				vkCreateSemaphore(vkld[0], &vksemaph_info[0], NULL, &vksemaph_image[0]) );

			if(valid) {
				vr("vkAcquireNextImageKHR", &vkres, 
					vkAcquireNextImageKHR(vkld[0], vkswap[0], UINT64_MAX, vksemaph_image[0], 
						VK_NULL_HANDLE, &aqimg_idx[0]) );
					iv("aqimg_idx", aqimg_idx[0], i);
			
				vr("vkCreateSemaphore", &vkres, 
					vkCreateSemaphore(vkld[0], &vksemaph_info[0], NULL, &vksemaph_rendr[0]) );

				VkSubmitInfo vksub_info[1];
					vksub_info[0].sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO;
					vksub_info[0].pNext	= NULL;
					vksub_info[0].waitSemaphoreCount	= 1;
					vksub_info[0].pWaitSemaphores		= vksemaph_image;
					vksub_info[0].pWaitDstStageMask		= &qsubwait;
					vksub_info[0].commandBufferCount	= 1;
					vksub_info[0].pCommandBuffers		= &vkcombuf_loop[aqimg_idx[0]];
					vksub_info[0].signalSemaphoreCount	= 1;
					vksub_info[0].pSignalSemaphores		= vksemaph_rendr;

				if(valid) {
					vr("vkQueueSubmit", &vkres, 
						vkQueueSubmit(vkq[0], 1, vksub_info, vkfence_aqimg[0]) );
					
					int res_idx = vkres.size();
					do {
						vr("vkWaitForFences <100ms>", &vkres, 
							vkWaitForFences(vkld[0], 1, vkfence_aqimg, VK_TRUE, 100000000) );
							res_idx = vkres.size()-1;
					} while (vkres[res_idx] == VK_TIMEOUT);

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
							vkQueuePresentKHR(vkq[0], &vkpresent_info[0]) );
					}
				}
			}
		}
		i++;
	} while (valid);

	if(!valid) {
		hd("STAGE:", "ABORTED");
	}

	rv("XCloseDisplay");
		XCloseDisplay(d);

	rv("return");
		return 0;
}



