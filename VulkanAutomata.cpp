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

void vr(const std::string& id, std::vector<VkResult>* reslist, VkResult res) {
	reslist->push_back(res);
	uint32_t 	idx 		= reslist->size() - 1;
	std::string	idx_string 	= std::to_string(idx);
	uint32_t 	idx_sz		= idx_string.size();
	std::string res_string 	= std::to_string(res);
	if(idx_sz < 4) { for(int i = 0; i < 4-idx_sz; i++) { idx_string = " " + idx_string; } }
/**/
	std::cout	
		<< "  " << idx_string 		<< ":\t"
		<< (res==0?" ":res_string) 	<< " \t"
		<< id 						<< "\n";
/**/
}

void ov(const std::string& id, auto v) {
	int 		padlen	= 4;
	int 		pads	= 10;
	std::string pad 	= " ";
	int 		padsize = (pads*padlen - id.size()) - 3;
	for(int i = 0; i < padsize; i++) { pad = pad + "."; }
/**/
	std::cout 
		<< "\tinfo:\t    "
		<< id 	<< pad 
		<< " [" << v	<< "]\n";
/**/
}

void iv(const std::string& id, auto ov, int idx) {
	int 		padlen 	= 4;
	int 		pads 	= 9;
	std::string pad 	= " ";
	int 		padsize = (pads*padlen - id.size()) - 3;
	for(int i = 0; i < padsize; i++) { pad = pad + "."; }
/**/
	std::cout 
		<< "\tinfo:\t    "
		<< idx 	<< "\t"
		<< id 	<< pad 
		<< " [" << ov	<< "]\n";
/**/
}

void rv(const std::string& id) {
/**/
	std::cout 
		<< "  void: \t" 
		<< id	<< "\n";
/**/
}

void hd(const std::string& id, const std::string& msg) {
	std::string bar = "";
	for(int i = 0; i < 20; i++) { bar = bar + "____"; }
/**/	
	std::cout 
		<< bar 	<< "\n "
		<< id 	<< "\t"
		<< msg 	<< "\n";
/**/
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

	std::string msg_killer = "";
	for(int i = msg.size()-6; i < msg.size()-1; i++) {
		msg_killer = msg_killer + msg[i]; }
//	if(	msg_killer != "01296" && msg_killer != "01403") {
/**/
		std::cout 
			<< "\n\n"
			<< bar 		<< "\n "
			<< msg_fmt
			<< "\n" 	<< bar
			<< "\n\n";
/**/
//	}

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

struct init_u0 {
    float uw;
	float uh;
	float us;
};

int main(void) {

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "CONFIG");		/**/
	///////////////////////////////////////

	const uint32_t 	APP_W 			= 1024;
	const uint32_t 	APP_H 			= 768;
	const long 		FPS 			= 60;
	const int 		TEST_CYCLES 	= 120;

	uint32_t 		PD_IDX 			= UINT32_MAX;
	uint32_t 		GQF_IDX 		= UINT32_MAX;
	uint32_t		SURF_FMT 		= UINT32_MAX;

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
										| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT 
										| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		vkdum_info[0].messageType		= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
										| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		vkdum_info[0].pfnUserCallback	= debugCallback;
		ov("messageSeverity", 	vkdum_info[0].messageSeverity	);
		ov("messageType", 		vkdum_info[0].messageType		);

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
		ov("VkImageUsageFlags", 	vksurf_ables[0].supportedUsageFlags		);
		ov("maxImageArrayLayers", 	vksurf_ables[0].maxImageArrayLayers		);

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

	VkDescriptorSetLayoutBinding vkgp_laydes_setbnd[1];
		vkgp_laydes_setbnd[0].binding				= 0;
		vkgp_laydes_setbnd[0].descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		vkgp_laydes_setbnd[0].descriptorCount		= 1;
		vkgp_laydes_setbnd[0].stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT;
		vkgp_laydes_setbnd[0].pImmutableSamplers	= NULL;

	VkDescriptorSetLayoutCreateInfo vkgp_laydes_info[1];
		vkgp_laydes_info[0].sType	= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	nf(&vkgp_laydes_info[0]);
		vkgp_laydes_info[0].bindingCount	= 1;
		vkgp_laydes_info[0].pBindings		= vkgp_laydes_setbnd;
		
	VkDescriptorSetLayout vkgp_laydes[2];
	vr("vkCreateDescriptorSetLayout", &vkres, 
		vkCreateDescriptorSetLayout(vkld[0], &vkgp_laydes_info[0], NULL, &vkgp_laydes[0]) );
	vr("vkCreateDescriptorSetLayout", &vkres, 
		vkCreateDescriptorSetLayout(vkld[0], &vkgp_laydes_info[0], NULL, &vkgp_laydes[1]) );


	VkPipelineLayoutCreateInfo vkgp_lay_info[1];
		vkgp_lay_info[0].sType	= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	nf(&vkgp_lay_info[0]);
		vkgp_lay_info[0].setLayoutCount			= 1;
		vkgp_lay_info[0].pSetLayouts			= vkgp_laydes;
		vkgp_lay_info[0].pushConstantRangeCount	= 0;
		vkgp_lay_info[0].pPushConstantRanges	= NULL;

	VkPipelineLayout vkgp_lay[1];
	vr("vkCreatePipelineLayout", &vkres, 
		vkCreatePipelineLayout(vkld[0], &vkgp_lay_info[0], NULL, vkgp_lay) );

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "RNDPASS");	/**/
	///////////////////////////////////////

	VkAttachmentDescription vkatd_loop[1];
		vkatd_loop[0].flags				= 0;
		vkatd_loop[0].format			= vksurf_fmt[SURF_FMT].format;
		vkatd_loop[0].samples			= VK_SAMPLE_COUNT_1_BIT;
		vkatd_loop[0].loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
		vkatd_loop[0].storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
		vkatd_loop[0].stencilLoadOp		= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vkatd_loop[0].stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vkatd_loop[0].initialLayout		= VK_IMAGE_LAYOUT_UNDEFINED;
		vkatd_loop[0].finalLayout		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference vkatref_loop[1];
		vkatref_loop[0].attachment	= 0;
		vkatref_loop[0].layout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription vksubpass_loop[1];
		vksubpass_loop[0].flags						= 0;
		vksubpass_loop[0].pipelineBindPoint			= VK_PIPELINE_BIND_POINT_GRAPHICS;
		vksubpass_loop[0].inputAttachmentCount		= 0;
		vksubpass_loop[0].pInputAttachments			= NULL;
		vksubpass_loop[0].colorAttachmentCount		= 1;
		vksubpass_loop[0].pColorAttachments			= vkatref_loop;
		vksubpass_loop[0].pResolveAttachments		= NULL;
		vksubpass_loop[0].pDepthStencilAttachment	= NULL;
		vksubpass_loop[0].preserveAttachmentCount	= 0;
		vksubpass_loop[0].pPreserveAttachments		= NULL;

	VkSubpassDependency vksubpass_loop_dep[1];
		vksubpass_loop_dep[0].srcSubpass		= VK_SUBPASS_EXTERNAL;
		vksubpass_loop_dep[0].dstSubpass		= 0;
		vksubpass_loop_dep[0].srcStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_loop_dep[0].dstStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		vksubpass_loop_dep[0].srcAccessMask		= 0;
		vksubpass_loop_dep[0].dstAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vksubpass_loop_dep[0].dependencyFlags	= 0;

	VkRenderPassCreateInfo vkrp_loop_info[1];
		vkrp_loop_info[0].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	nf(&vkrp_loop_info[0]);
		vkrp_loop_info[0].attachmentCount	= 1;
		vkrp_loop_info[0].pAttachments		= vkatd_loop;
		vkrp_loop_info[0].subpassCount		= 1;
		vkrp_loop_info[0].pSubpasses		= vksubpass_loop;
		vkrp_loop_info[0].dependencyCount	= 1;
		vkrp_loop_info[0].pDependencies		= vksubpass_loop_dep;

	VkRenderPass vkrp_loop[1];
	vr("vkCreateRenderPass", &vkres, 
		vkCreateRenderPass(vkld[0], &vkrp_loop_info[0], NULL, &vkrp_loop[0]) );

	VkGraphicsPipelineCreateInfo vkgp_info[1];
		vkgp_info[0].sType	= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	nf(&vkgp_info[0]);
		vkgp_info[0].stageCount				= SHDRSTGS;
		vkgp_info[0].pStages				= vkgfxpipe_ss_info;
		vkgp_info[0].pVertexInputState		= &vkgfxpipe_vertins_info[0];
		vkgp_info[0].pInputAssemblyState	= &vkgfxpipe_ias_info[0];
		vkgp_info[0].pTessellationState		= NULL;
		vkgp_info[0].pViewportState			= &vkgfxpipe_viewport_info[0];
		vkgp_info[0].pRasterizationState	= &vkgfxpipe_rast_info[0];
		vkgp_info[0].pMultisampleState		= &vkgfxpipe_ms_info[0];
		vkgp_info[0].pDepthStencilState		= NULL;
		vkgp_info[0].pColorBlendState		= &vkgfxpipe_colblend_info[0];
		vkgp_info[0].pDynamicState			= NULL;
		vkgp_info[0].layout					= vkgp_lay[0];
		vkgp_info[0].renderPass				= vkrp_loop[0];
		vkgp_info[0].subpass				= 0;
		vkgp_info[0].basePipelineHandle		= VK_NULL_HANDLE;
		vkgp_info[0].basePipelineIndex		= -1;

	VkPipeline vkgfxpipe[1];
	vr("vkCreateGraphicsPipelines", &vkres, 
		vkCreateGraphicsPipelines(vkld[0], VK_NULL_HANDLE, 1, vkgp_info, NULL, &vkgfxpipe[0]) );

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

	VkFramebufferCreateInfo vkfbuf_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
			vkfbuf_info[i].sType	= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		nf(&vkfbuf_info[i]);
			vkfbuf_info[i].renderPass		= vkrp_loop[0];;
			vkfbuf_info[i].attachmentCount	= 1;
			vkfbuf_info[i].pAttachments		= &vkimgview[i];
			vkfbuf_info[i].width			= vksurf_ables[0].currentExtent.width;
			vkfbuf_info[i].height			= vksurf_ables[0].currentExtent.height;
			vkfbuf_info[i].layers			= 1;
	}

	VkFramebuffer vkfbuf[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkCreateFramebuffer", &vkres, 
			vkCreateFramebuffer(vkld[0], &vkfbuf_info[i], NULL, &vkfbuf[i]) );
	}

	VkClearValue vkclear_col[1];
		vkclear_col[0].color	= { 1.0f, 0.0f, 1.0f, 1.0f };

	VkRenderPassBeginInfo vkrpbegin_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vkrpbegin_info[i].sType	= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegin_info[i].pNext	= NULL;
		vkrpbegin_info[i].renderPass		= vkrp_loop[0];
		vkrpbegin_info[i].framebuffer		= vkfbuf[i];
		vkrpbegin_info[i].renderArea		= vkgfxpipe_sciz[0];
		vkrpbegin_info[i].clearValueCount	= 1;
		vkrpbegin_info[i].pClearValues		= vkclear_col;
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

	VkCommandBuffer vkcombuf[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vr("vkAllocateCommandBuffers", &vkres, 
			vkAllocateCommandBuffers(vkld[0], &vkcombuf_alloc_info[0], vkcombuf) );
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
		vksmplr_info[0].pNext	= NULL;
		vksmplr_info[0].flags	= 0;
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

	VkDescriptorPoolSize vkdescpool_size[1];
		vkdescpool_size[0].type				= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		vkdescpool_size[0].descriptorCount	= 1;

	VkDescriptorPoolCreateInfo vkdescpool_info[1];
		vkdescpool_info[0].sType	= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	nf(&vkdescpool_info[0]);
		vkdescpool_info[0].maxSets			= 2;
		vkdescpool_info[0].poolSizeCount	= 1;
		vkdescpool_info[0].pPoolSizes		= vkdescpool_size;

	VkDescriptorPool vkdescpool[1];
	vr("vkCreateDescriptorPool", &vkres, 
		vkCreateDescriptorPool(vkld[0], &vkdescpool_info[0], NULL, &vkdescpool[0]) );

	VkDescriptorSetAllocateInfo vkdescset_alloc_info[1];
		vkdescset_alloc_info[0].sType	= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		vkdescset_alloc_info[0].pNext	= NULL;
		vkdescset_alloc_info[0].descriptorPool		= vkdescpool[0];
		vkdescset_alloc_info[0].descriptorSetCount	= vkdescpool_info[0].maxSets;
		vkdescset_alloc_info[0].pSetLayouts			= vkgp_laydes;

	VkDescriptorSet vkdescset[vkdescpool_info[0].maxSets];
	vr("vkAllocateDescriptorSets", &vkres, 
		vkAllocateDescriptorSets(vkld[0], &vkdescset_alloc_info[0], vkdescset) );

	VkDescriptorImageInfo vkdesc_img_info[swap_img_cnt];
	for(int i = 0; i < swap_img_cnt; i++) {
		vkdesc_img_info[i].sampler		= vksmplr[0];
		vkdesc_img_info[i].imageView	= vkimgview[i];
		vkdesc_img_info[i].imageLayout	= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}

	const int DESC_UP_CNT = 2;

	VkWriteDescriptorSet vkdescset_write[DESC_UP_CNT];
	for(int i = 0; i < DESC_UP_CNT; i++) {
		vkdescset_write[i].sType	= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vkdescset_write[i].pNext	= NULL;
		vkdescset_write[i].dstSet			= vkdescset[i];
		vkdescset_write[i].dstBinding		= 0;
		vkdescset_write[i].dstArrayElement	= 0;
		vkdescset_write[i].descriptorCount	= 1;
		vkdescset_write[i].descriptorType	= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		vkdescset_write[i].pImageInfo		= &vkdesc_img_info[(i+1)%DESC_UP_CNT];
		vkdescset_write[i].pBufferInfo		= NULL;
		vkdescset_write[i].pTexelBufferView	= NULL;
	}

	rv("vkUpdateDescriptorSets");
		vkUpdateDescriptorSets(vkld[0], DESC_UP_CNT, vkdescset_write, 0, NULL);

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "COMMAND");	/**/
	///////////////////////////////////////

	for(int i = 0; i < swap_img_cnt; i++) {

		vr("vkBeginCommandBuffer", &vkres, 
			vkBeginCommandBuffer(vkcombuf[i], &vkcombufbegin_info[i]) );

			rv("vkCmdBeginRenderPass");
				vkCmdBeginRenderPass (
					vkcombuf[i], &vkrpbegin_info[i], VK_SUBPASS_CONTENTS_INLINE );

				rv("vkCmdBindPipeline");
					vkCmdBindPipeline (
						vkcombuf[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkgfxpipe[0] );

				rv("vkCmdBindDescriptorSets");
					vkCmdBindDescriptorSets (
						vkcombuf[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkgp_lay[0],
						0, 1, &vkdescset[i], 0, NULL );

				rv("vkCmdDraw");
					vkCmdDraw (
						vkcombuf[i], 3, 1, 0, 0 );

			rv("vkCmdEndRenderPass");
				vkCmdEndRenderPass(vkcombuf[i]);

		vr("vkEndCommandBuffer", &vkres, 
			vkEndCommandBuffer(vkcombuf[i]) );

	}

	VkQueue vkq[1];
	rv("vkGetDeviceQueue");
		vkGetDeviceQueue(vkld[0], GQF_IDX, 0, &vkq[0]);

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "SYNC");		/**/
	///////////////////////////////////////

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

	for(int i = 0; i < swap_img_cnt * TEST_CYCLES; i++) {

	  ///////////////////////////////////////
	 /**/	hd("STAGE:", "DO_LOOP");	/**/
	///////////////////////////////////////

		rv("nanosleep(NS_DELAY)");
			nanosleep((const struct timespec[]){{0, NS_DELAY}}, NULL);

		vr("vkResetFences", &vkres, 
			vkResetFences(vkld[0], 1, vkfence_aqimg) );

		vr("vkCreateSemaphore", &vkres, 
			vkCreateSemaphore(vkld[0], &vksemaph_info[0], NULL, &vksemaph_image[0]) );

		vr("vkAcquireNextImageKHR", &vkres, 
			vkAcquireNextImageKHR(vkld[0], vkswap[0], UINT64_MAX, vksemaph_image[0], 
				VK_NULL_HANDLE, &aqimg_idx[0]) );

		vr("vkCreateSemaphore", &vkres, 
			vkCreateSemaphore(vkld[0], &vksemaph_info[0], NULL, &vksemaph_rendr[0]) );

		VkSubmitInfo vksub_info[1];
			vksub_info[0].sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO;
			vksub_info[0].pNext	= NULL;
			vksub_info[0].waitSemaphoreCount	= 1;
			vksub_info[0].pWaitSemaphores		= vksemaph_image;
			vksub_info[0].pWaitDstStageMask		= &qsubwait;
			vksub_info[0].commandBufferCount	= 1;
			vksub_info[0].pCommandBuffers		= &vkcombuf[aqimg_idx[0]];
			vksub_info[0].signalSemaphoreCount	= 1;
			vksub_info[0].pSignalSemaphores		= vksemaph_rendr;

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

		vr("vkQueuePresentKHR", &vkres, 
			vkQueuePresentKHR(vkq[0], &vkpresent_info[0]) );

	}

	rv("XCloseDisplay");
		XCloseDisplay(d);

	rv("return");
		return 0;
}



