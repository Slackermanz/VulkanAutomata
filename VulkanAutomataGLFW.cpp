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
#include <thread>
#include <cstring>
#include <cmath>

// Include our modularized type definitions
#include "vkmodules/Types/AllTypes.h"
#include "vkmodules/Utils/Utils.h"
#include "vkmodules/CellularAutomata/CellularAutomata.h"
#include "vkmodules/UI/UI.h"
#include "vkmodules/Input/Input.h"
#include "vkmodules/VulkanFoundation/VulkanFoundation.h"

const 	uint32_t 	VERT_FLS 		=  1;	//	Number of Vertex Shader Files
const 	uint32_t 	FRAG_FLS 		=  1;	//	Number of Fragment Shader Files
const	int 		MAXLOG 			=  2;
		int 		valid 			=  1;
		uint32_t	verbose_loops 	= 12;	// How many loops to output full diagnostics
        uint32_t    panel_n_x_n     =  4;

//	Some shaders require dimensions to be powers of two (fast wrap-around/torus code)
//		16:9 Resolutions:
//			W:	1920 	1536 	1280	768		512		384		256
//			H:	1080 	864  	720		432		288		216		144
//		2:1 Resolutions:
//			W:	16384 	8192 	4096	2048	1024	512		256
//			H:	8192 	4096  	2048	1024	512		256		128

	const 	uint32_t 	APP_W 	= 64*4*2*4;	//	Window & Simulation Width
	const 	uint32_t 	APP_H 	= 64*4*1*4;	//	Window & Simulation Height

int main() {

//	Set the random seed
	uint32_t INIT_TIME = time(0);
	srand(INIT_TIME);

//	Result storage
	std::vector<VkResult> vkres;

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "USER CONFIG");			/**/
	///////////////////////////////////////////////////


	EngineInfo ei;
		ei.paused 				= false;				//	Pause Simulation
		ei.show_gui 			=  true;				//	Render Dear IMGUI
		ei.run_headless 		= false;				//	Use Headless Mode
		ei.imgdat_idx 			= 0;					//	Use Headless Mode
		ei.export_frequency 	= 8;					//	Export image file every n frames
		ei.export_batch_size 	= 180;					//	Pause recording after n exported frames
		ei.export_batch_left 	= ei.export_batch_size;	//	Frames remaining in batch
		ei.export_batch_last 	= ei.export_batch_size;	//	For IMGUI handling
		ei.export_enabled		= false;				//	Enables image exports to disk
		ei.tick_loop 			= 0;					//	Run the main loop n times, ignoring pause state
		ei.load_pattern 		= 18372;				//	Load this index from the pattern archive file

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

	uint32_t glfw_ext_count;
	const char** glfw_extentions; // Keep original variable name with typo for consistency
	initGLFWExtensions(&glfw_ext_count, &glfw_extentions, &vkres);

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

	VK_Obj vob;
	VK_Config vkcfg;
	
	initVulkanInstance(&vob, &vkcfg, instance_extensions, INST_EXS, 
					  layer_extensions, LAYR_EXS, &vkres);

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "DEBUG UTILS");			/**/
	///////////////////////////////////////////////////

	VK_Debug vkdbg;
	setupDebugMessenger(&vob, &vkdbg, &vkres);

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
		vk_swapchhain_info.imageUsage				= 31; // Hardcoded to old value found in logs, ignoring new extension. // vk_surface_capabilities.supportedUsageFlags;
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
		blit.img_info.usage 				= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
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
	 /**/	hd("STAGE:", "BLIT EXPORT BUFFER");		/**/
	///////////////////////////////////////////////////

	VK_Buffer_1x2D blit2buff;

		blit2buff.buff_info.sType 					= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	nf(&blit2buff.buff_info);
		blit2buff.buff_info.size 					= blit.vk_mem_reqs.size;
		blit2buff.buff_info.usage 					= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		blit2buff.buff_info.sharingMode 			= VK_SHARING_MODE_EXCLUSIVE;
		blit2buff.buff_info.queueFamilyIndexCount 	= 1;
		blit2buff.buff_info.pQueueFamilyIndices 	= &vob.VKQ_i;

		vr("vkCreateBuffer", &vkres, blit2buff.vk_buffer,
			vkCreateBuffer(vob.VKL, &blit2buff.buff_info, NULL, &blit2buff.vk_buffer) );

		VkMemoryRequirements vk_memory_requirements_blit2buff;

		rv("vkGetBufferMemoryRequirements");
			vkGetBufferMemoryRequirements(vob.VKL, blit2buff.vk_buffer, &blit2buff.vk_mem_reqs);

			ov("blit2buff size", 			blit2buff.vk_mem_reqs.size);
			ov("blit2buff alignment", 		blit2buff.vk_mem_reqs.alignment);
			ov("blit2buff memoryTypeBits", 	blit2buff.vk_mem_reqs.memoryTypeBits);

		blit2buff.MTB_index = findProperties(
			&pdev[vob.VKP_i].vk_pdev_mem_props,
			blit2buff.vk_mem_reqs.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT );

			ov("memoryTypeIndex", blit2buff.MTB_index);

		blit2buff.vk_mem_allo_info.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		blit2buff.vk_mem_allo_info.pNext				= NULL;
		blit2buff.vk_mem_allo_info.allocationSize		= blit2buff.vk_mem_reqs.size;
		blit2buff.vk_mem_allo_info.memoryTypeIndex		= blit2buff.MTB_index;

		vr("vkAllocateMemory", &vkres, blit2buff.vk_dev_mem,
			vkAllocateMemory(vob.VKL, &blit2buff.vk_mem_allo_info, NULL, &blit2buff.vk_dev_mem) );

		vr("vkBindBufferMemory", &vkres, blit2buff.vk_buffer,
			vkBindBufferMemory(vob.VKL,  blit2buff.vk_buffer, blit2buff.vk_dev_mem, 0) );

	//	Map the memory location on the GPU to export image data
		void* pvoid_blit2buff;
		vr("vkMapMemory", &vkres, pvoid_blit2buff,
			vkMapMemory(vob.VKL, blit2buff.vk_dev_mem, 0, VK_WHOLE_SIZE, 0, &pvoid_blit2buff) );

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

	VK_Command combuf_blit2buff_sing[1];
	for(int i = 0; i < 1; i++) {
		combuf_blit2buff_sing[i].pool_info.sType							= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		combuf_blit2buff_sing[i].pool_info.pNext							= NULL;
		combuf_blit2buff_sing[i].pool_info.flags							= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		combuf_blit2buff_sing[i].pool_info.queueFamilyIndex					= vob.VKQ_i;

		vr("vkCreateCommandPool", &vkres, combuf_blit2buff_sing[i].vk_command_pool,
			vkCreateCommandPool(vob.VKL, &combuf_blit2buff_sing[i].pool_info, NULL, &combuf_blit2buff_sing[i].vk_command_pool) );

		combuf_blit2buff_sing[i].comm_buff_alloc_info.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		combuf_blit2buff_sing[i].comm_buff_alloc_info.pNext					= NULL;
		combuf_blit2buff_sing[i].comm_buff_alloc_info.commandPool			= combuf_blit2buff_sing[i].vk_command_pool;
		combuf_blit2buff_sing[i].comm_buff_alloc_info.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		combuf_blit2buff_sing[i].comm_buff_alloc_info.commandBufferCount	= 1;

		vr("vkAllocateCommandBuffers", &vkres, combuf_blit2buff_sing[i].vk_command_buffer,
			vkAllocateCommandBuffers(vob.VKL, &combuf_blit2buff_sing[i].comm_buff_alloc_info, &combuf_blit2buff_sing[i].vk_command_buffer) );

		combuf_blit2buff_sing[i].comm_buff_begin_info.sType 				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	nf(&combuf_blit2buff_sing[i].comm_buff_begin_info);
		combuf_blit2buff_sing[i].comm_buff_begin_info.pInheritanceInfo		= NULL; }



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

	VK_Command combuf_work_imagedata_init[1];
	for(int i = 0; i < 1; i++) {
		combuf_work_imagedata_init[i].pool_info.sType							= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		combuf_work_imagedata_init[i].pool_info.pNext							= NULL;
		combuf_work_imagedata_init[i].pool_info.flags							= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		combuf_work_imagedata_init[i].pool_info.queueFamilyIndex				= vob.VKQ_i;
		vr("vkCreateCommandPool", &vkres, combuf_work_imagedata_init[i].vk_command_pool,
			vkCreateCommandPool(vob.VKL, &combuf_work_imagedata_init[i].pool_info, NULL, &combuf_work_imagedata_init[i].vk_command_pool) );
		combuf_work_imagedata_init[i].comm_buff_alloc_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		combuf_work_imagedata_init[i].comm_buff_alloc_info.pNext				= NULL;
		combuf_work_imagedata_init[i].comm_buff_alloc_info.commandPool			= combuf_work_imagedata_init[i].vk_command_pool;
		combuf_work_imagedata_init[i].comm_buff_alloc_info.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		combuf_work_imagedata_init[i].comm_buff_alloc_info.commandBufferCount	= 1;
		vr("vkAllocateCommandBuffers", &vkres, combuf_work_imagedata_init[i].vk_command_buffer,
			vkAllocateCommandBuffers(vob.VKL, &combuf_work_imagedata_init[i].comm_buff_alloc_info, &combuf_work_imagedata_init[i].vk_command_buffer) );
		combuf_work_imagedata_init[i].comm_buff_begin_info.sType 				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	nf(&combuf_work_imagedata_init[i].comm_buff_begin_info);
		combuf_work_imagedata_init[i].comm_buff_begin_info.pInheritanceInfo		= NULL; }

	VK_Command combuf_work_imagedata[2];
	for(int i = 0; i < 2; i++) {
		combuf_work_imagedata[i].pool_info.sType							= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		combuf_work_imagedata[i].pool_info.pNext							= NULL;
		combuf_work_imagedata[i].pool_info.flags							= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		combuf_work_imagedata[i].pool_info.queueFamilyIndex					= vob.VKQ_i;

		vr("vkCreateCommandPool", &vkres, combuf_work_imagedata[i].vk_command_pool,
			vkCreateCommandPool(vob.VKL, &combuf_work_imagedata[i].pool_info, NULL, &combuf_work_imagedata[i].vk_command_pool) );

		combuf_work_imagedata[i].comm_buff_alloc_info.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		combuf_work_imagedata[i].comm_buff_alloc_info.pNext					= NULL;
		combuf_work_imagedata[i].comm_buff_alloc_info.commandPool			= combuf_work_imagedata[i].vk_command_pool;
		combuf_work_imagedata[i].comm_buff_alloc_info.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		combuf_work_imagedata[i].comm_buff_alloc_info.commandBufferCount	= 1;

		vr("vkAllocateCommandBuffers", &vkres, combuf_work_imagedata[i].vk_command_buffer,
			vkAllocateCommandBuffers(vob.VKL, &combuf_work_imagedata[i].comm_buff_alloc_info, &combuf_work_imagedata[i].vk_command_buffer) );

		combuf_work_imagedata[i].comm_buff_begin_info.sType 				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	nf(&combuf_work_imagedata[i].comm_buff_begin_info);
		combuf_work_imagedata[i].comm_buff_begin_info.pInheritanceInfo		= NULL; }

	VK_Command combuf_blit_imgui_loop[swap_image_count];
	for(int i = 0; i < swap_image_count; i++) {
		combuf_blit_imgui_loop[i].pool_info.sType							= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		combuf_blit_imgui_loop[i].pool_info.pNext							= NULL;
		combuf_blit_imgui_loop[i].pool_info.flags							= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		combuf_blit_imgui_loop[i].pool_info.queueFamilyIndex				= vob.VKQ_i;

		vr("vkCreateCommandPool", &vkres, combuf_blit_imgui_loop[i].vk_command_pool,
			vkCreateCommandPool(vob.VKL, &combuf_blit_imgui_loop[i].pool_info, NULL, &combuf_blit_imgui_loop[i].vk_command_pool) );

		combuf_blit_imgui_loop[i].comm_buff_alloc_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		combuf_blit_imgui_loop[i].comm_buff_alloc_info.pNext				= NULL;
		combuf_blit_imgui_loop[i].comm_buff_alloc_info.commandPool			= combuf_blit_imgui_loop[i].vk_command_pool;
		combuf_blit_imgui_loop[i].comm_buff_alloc_info.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		combuf_blit_imgui_loop[i].comm_buff_alloc_info.commandBufferCount	= 1;

		vr("vkAllocateCommandBuffers", &vkres, combuf_blit_imgui_loop[i].vk_command_buffer,
			vkAllocateCommandBuffers(vob.VKL, &combuf_blit_imgui_loop[i].comm_buff_alloc_info, &combuf_blit_imgui_loop[i].vk_command_buffer) );

		combuf_blit_imgui_loop[i].comm_buff_begin_info.sType 				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	nf(&combuf_blit_imgui_loop[i].comm_buff_begin_info);
		combuf_blit_imgui_loop[i].comm_buff_begin_info.pInheritanceInfo		= NULL; }

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

	VkSemaphore vk_semaphore_swapchain_img_acq;
	vr("vkCreateSemaphore", &vkres, vk_semaphore_swapchain_img_acq,
		vkCreateSemaphore(vob.VKL, &vk_semaphore_info, NULL, &vk_semaphore_swapchain_img_acq) );

	VkSemaphore vk_semaphore_swapchain_imgui;
	vr("vkCreateSemaphore", &vkres, vk_semaphore_swapchain_imgui,
		vkCreateSemaphore(vob.VKL, &vk_semaphore_info, NULL, &vk_semaphore_swapchain_imgui) );

	VkSemaphore vk_semaphore_swapchain_pres;
	vr("vkCreateSemaphore", &vkres, vk_semaphore_swapchain_pres,
		vkCreateSemaphore(vob.VKL, &vk_semaphore_info, NULL, &vk_semaphore_swapchain_pres) );

	uint32_t swap_image_index = 0;

	VK_QueueSync swpsync;
	rv("vkGetDeviceQueue");
		vkGetDeviceQueue(vob.VKL, vob.VKQ_i, 0, &swpsync.vk_queue);
	VkPipelineStageFlags vk_pipeline_stage_flags_swpsync;
		vk_pipeline_stage_flags_swpsync = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		swpsync.sub_info.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		swpsync.sub_info.pNext					= NULL;
		swpsync.sub_info.waitSemaphoreCount		= 1;
		swpsync.sub_info.pWaitSemaphores		= &vk_semaphore_swapchain_img_acq;
		swpsync.sub_info.pWaitDstStageMask		= &vk_pipeline_stage_flags_swpsync;
		swpsync.sub_info.commandBufferCount		= 1;
//		swpsync.sub_info.pCommandBuffers		= &combuf_pres_loop[0].vk_command_buffer; // TODO i? 0, 1, 2 ? 
		swpsync.sub_info.signalSemaphoreCount	= 1;
		swpsync.sub_info.pSignalSemaphores		= &vk_semaphore_swapchain_imgui;

	VK_QueueSync swpsync_imgui;
	rv("vkGetDeviceQueue");
		vkGetDeviceQueue(vob.VKL, vob.VKQ_i, 0, &swpsync_imgui.vk_queue);
	VkPipelineStageFlags vk_pipeline_stage_flags_swpsync_imgui;
		vk_pipeline_stage_flags_swpsync_imgui 			= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		swpsync_imgui.sub_info.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		swpsync_imgui.sub_info.pNext					= NULL;
		swpsync_imgui.sub_info.waitSemaphoreCount		= 1;
		swpsync_imgui.sub_info.pWaitSemaphores			= &vk_semaphore_swapchain_imgui;
		swpsync_imgui.sub_info.pWaitDstStageMask		= &vk_pipeline_stage_flags_swpsync_imgui;
		swpsync_imgui.sub_info.commandBufferCount		= 1;
//		swpsync_imgui.sub_info.pCommandBuffers		= &combuf_imgui_loop[swap_image_index].vk_command_buffer;
		swpsync_imgui.sub_info.signalSemaphoreCount		= 1;
		swpsync_imgui.sub_info.pSignalSemaphores		= &vk_semaphore_swapchain_pres;

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

	VK_DescSetLayout3 dsl_work;

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
		dsl_work.set_bind[2].binding				= 2;
		dsl_work.set_bind[2].descriptorType			= VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		dsl_work.set_bind[2].descriptorCount		= 1;
		dsl_work.set_bind[2].stageFlags				= VK_SHADER_STAGE_FRAGMENT_BIT;
		dsl_work.set_bind[2].pImmutableSamplers		= NULL;

		dsl_work.set_info.sType 					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	nf(&dsl_work.set_info);
		dsl_work.set_info.bindingCount				= 3;
		dsl_work.set_info.pBindings					= dsl_work.set_bind;

		dsl_work.pool_size[0].type 					= dsl_work.set_bind[0].descriptorType;
		dsl_work.pool_size[0].descriptorCount 		= 2;
		dsl_work.pool_size[1].type 					= dsl_work.set_bind[1].descriptorType;
		dsl_work.pool_size[1].descriptorCount 		= 2;
		dsl_work.pool_size[2].type 					= dsl_work.set_bind[2].descriptorType;
		dsl_work.pool_size[2].descriptorCount 		= 2;

		dsl_work.pool_info.sType 					= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	nf(&dsl_work.pool_info);
		dsl_work.pool_info.maxSets 					= dsl_work.pool_size[0].descriptorCount
													+ dsl_work.pool_size[1].descriptorCount
													+ dsl_work.pool_size[2].descriptorCount;
		dsl_work.pool_info.poolSizeCount 			= 3;
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
		vkdevsize_work = sizeof(UB32_64);
	ov("UB32_64 size", vkdevsize_work);
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
	 /**/	hd("STAGE:", "SSBO");	/**/
	///////////////////////////////////////////////////

	VkDeviceSize vkdevsize_ssbo;
		vkdevsize_ssbo = sizeof(UB32_64) * 16;
	ov("UB32_64 * 16 size", vkdevsize_ssbo);
	VkBufferCreateInfo vkbuff_info_ssbo;
		vkbuff_info_ssbo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	nf(&vkbuff_info_ssbo);
		vkbuff_info_ssbo.size 						= vkdevsize_ssbo;
		vkbuff_info_ssbo.usage 						= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		vkbuff_info_ssbo.sharingMode 				= VK_SHARING_MODE_EXCLUSIVE;
		vkbuff_info_ssbo.queueFamilyIndexCount 		= 1;
		vkbuff_info_ssbo.pQueueFamilyIndices 		= &vob.VKQ_i;
	VkBuffer vkbuff_ssbo;
	vr("vkCreateBuffer", &vkres, vkbuff_ssbo,
		vkCreateBuffer(vob.VKL, &vkbuff_info_ssbo, NULL, &vkbuff_ssbo) );
	VkDescriptorBufferInfo vkDescBuff_info_ssbo;
		vkDescBuff_info_ssbo.buffer 		= vkbuff_ssbo;
		vkDescBuff_info_ssbo.offset 		= 0;
		vkDescBuff_info_ssbo.range 			= VK_WHOLE_SIZE;
	VkWriteDescriptorSet vkwritedescset_sb_work[2];
	for(int i = 0; i < 2; i++) {
		vkwritedescset_sb_work[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vkwritedescset_sb_work[i].pNext 				= NULL;
		vkwritedescset_sb_work[i].dstSet 				= dsl_work.vk_descriptor_set[i];
		vkwritedescset_sb_work[i].dstBinding 			= 2;
		vkwritedescset_sb_work[i].dstArrayElement 		= 0;
		vkwritedescset_sb_work[i].descriptorCount 		= 1;
		vkwritedescset_sb_work[i].descriptorType 		= dsl_work.set_bind[2].descriptorType;
		vkwritedescset_sb_work[i].pImageInfo 			= NULL;
		vkwritedescset_sb_work[i].pBufferInfo 			= &vkDescBuff_info_ssbo;
		vkwritedescset_sb_work[i].pTexelBufferView 		= NULL; }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "ssbo DESCRIPTOR MEMORY");	/**/
	///////////////////////////////////////////////////

	VkMemoryRequirements vkmemreqs_sb_work;
	rv("vkGetBufferMemoryRequirements");
		vkGetBufferMemoryRequirements(vob.VKL, vkbuff_ssbo, &vkmemreqs_sb_work);
	ov("memreq size", 			vkmemreqs_sb_work.size);
	ov("memreq alignment", 		vkmemreqs_sb_work.alignment);
	ov("memreq memoryTypeBits", vkmemreqs_sb_work.memoryTypeBits);
	int mem_index_sb_work = UINT32_MAX;
		mem_index_sb_work = findProperties(
			&pdev[vob.VKP_i].vk_pdev_mem_props,
			vkmemreqs_sb_work.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );
	ov("memoryTypeIndex", mem_index_sb_work);
	VkMemoryAllocateInfo vkmemallo_info_ssbo;
		vkmemallo_info_ssbo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		vkmemallo_info_ssbo.pNext			= NULL;
		vkmemallo_info_ssbo.allocationSize	= vkmemreqs_sb_work.size;
		vkmemallo_info_ssbo.memoryTypeIndex	= mem_index_sb_work;
	VkDeviceMemory vkdevmem_sb_work;
	vr("vkAllocateMemory", &vkres, vkdevmem_sb_work,
		vkAllocateMemory(vob.VKL, &vkmemallo_info_ssbo, NULL, &vkdevmem_sb_work) );
//	Assign device (GPU) memory to hold the Uniform Buffer
	vr("vkBindBufferMemory", &vkres, vkbuff_ssbo,
		vkBindBufferMemory(vob.VKL, vkbuff_ssbo, vkdevmem_sb_work, 0) );
//	Update the Sampler and Uniform Buffer Descriptors
	for(int i = 0; i < 2; i++) {
		rv("vkUpdateDescriptorSets");
			vkUpdateDescriptorSets(vob.VKL, 1, &vkwritedescset_sb_work[i], 0, NULL); }

//	Map the memory location on the GPU for memcpy() to submit the Uniform Buffer
	void *pvoid_memmap_ssbo;
	vr("vkMapMemory", &vkres, pvoid_memmap_ssbo,
		vkMapMemory(vob.VKL, vkdevmem_sb_work, vkDescBuff_info_ssbo.offset, vkDescBuff_info_ssbo.range, 0, &pvoid_memmap_ssbo) );

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
		rp_imgui.attach_desc.loadOp 					= VK_ATTACHMENT_LOAD_OP_LOAD;
		rp_imgui.attach_desc.storeOp 					= VK_ATTACHMENT_STORE_OP_STORE;
		rp_imgui.attach_desc.stencilLoadOp 				= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		rp_imgui.attach_desc.stencilStoreOp 			= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		rp_imgui.attach_desc.initialLayout 				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
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
	 /**/	hd("STAGE:", "BARRIERS");				/**/
	///////////////////////////////////////////////////

	// Are all of these used? TODO

	VkImageMemoryBarrier vk_IMB_blit_TSO_to_TDO;
		vk_IMB_blit_TSO_to_TDO.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_blit_TSO_to_TDO.pNext 					= NULL;
		vk_IMB_blit_TSO_to_TDO.srcAccessMask 			= 0;
		vk_IMB_blit_TSO_to_TDO.dstAccessMask 			= 0;
		vk_IMB_blit_TSO_to_TDO.oldLayout 				= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vk_IMB_blit_TSO_to_TDO.newLayout 				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		vk_IMB_blit_TSO_to_TDO.srcQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_blit_TSO_to_TDO.dstQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_blit_TSO_to_TDO.image 					= blit.vk_image;
		vk_IMB_blit_TSO_to_TDO.subresourceRange 		= rpass_info.img_subres_range;

	VkImageMemoryBarrier vk_IMB_blit_TDO_to_TSO;
		vk_IMB_blit_TDO_to_TSO.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_blit_TDO_to_TSO.pNext 					= NULL;
		vk_IMB_blit_TDO_to_TSO.srcAccessMask 			= 0;
		vk_IMB_blit_TDO_to_TSO.dstAccessMask 			= 0;
		vk_IMB_blit_TDO_to_TSO.oldLayout 				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		vk_IMB_blit_TDO_to_TSO.newLayout 				= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vk_IMB_blit_TDO_to_TSO.srcQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_blit_TDO_to_TSO.dstQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_blit_TDO_to_TSO.image 					= blit.vk_image;
		vk_IMB_blit_TDO_to_TSO.subresourceRange 		= rpass_info.img_subres_range;

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

	VkImageMemoryBarrier vk_IMB_work_SRO_to_TSO[2];
	for(int i = 0; i < 2; i++) {
		vk_IMB_work_SRO_to_TSO[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_work_SRO_to_TSO[i].pNext 					= NULL;
		vk_IMB_work_SRO_to_TSO[i].srcAccessMask 			= 0;
		vk_IMB_work_SRO_to_TSO[i].dstAccessMask 			= 0;
		vk_IMB_work_SRO_to_TSO[i].oldLayout 				= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vk_IMB_work_SRO_to_TSO[i].newLayout 				= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vk_IMB_work_SRO_to_TSO[i].srcQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_work_SRO_to_TSO[i].dstQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_work_SRO_to_TSO[i].image 					= work.vk_image[i];
		vk_IMB_work_SRO_to_TSO[i].subresourceRange 			= rpass_info.img_subres_range; }

	VkImageMemoryBarrier vk_IMB_work_TSO_to_SRO[2];
	for(int i = 0; i < 2; i++) {
		vk_IMB_work_TSO_to_SRO[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_work_TSO_to_SRO[i].pNext 					= NULL;
		vk_IMB_work_TSO_to_SRO[i].srcAccessMask 			= 0;
		vk_IMB_work_TSO_to_SRO[i].dstAccessMask 			= 0;
		vk_IMB_work_TSO_to_SRO[i].oldLayout 				= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vk_IMB_work_TSO_to_SRO[i].newLayout 				= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vk_IMB_work_TSO_to_SRO[i].srcQueueFamilyIndex 	= vob.VKQ_i;
		vk_IMB_work_TSO_to_SRO[i].dstQueueFamilyIndex 	= vob.VKQ_i;
		vk_IMB_work_TSO_to_SRO[i].image 					= work.vk_image[i];
		vk_IMB_work_TSO_to_SRO[i].subresourceRange 		= rpass_info.img_subres_range; }

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
		vk_IMB_pres_UND_to_PRS[i].subresourceRange 			= rpass_info.img_subres_range; }

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
		vk_IMB_pres_PRS_to_TDO[i].subresourceRange 			= rpass_info.img_subres_range; }

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
		vk_IMB_pres_TDO_to_PRS[i].subresourceRange 			= rpass_info.img_subres_range; }

	VkImageMemoryBarrier vk_IMB_swap_PRS_to_TSO[swap_image_count];
	for(int i = 0; i < swap_image_count; i++) {
		vk_IMB_swap_PRS_to_TSO[i].sType 					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_swap_PRS_to_TSO[i].pNext 					= NULL;
		vk_IMB_swap_PRS_to_TSO[i].srcAccessMask 			= 0;
		vk_IMB_swap_PRS_to_TSO[i].dstAccessMask 			= 0;
		vk_IMB_swap_PRS_to_TSO[i].oldLayout 				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		vk_IMB_swap_PRS_to_TSO[i].newLayout 				= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vk_IMB_swap_PRS_to_TSO[i].srcQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_swap_PRS_to_TSO[i].dstQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_swap_PRS_to_TSO[i].image 					= vk_image_swapimgs[i];
		vk_IMB_swap_PRS_to_TSO[i].subresourceRange 			= rpass_info.img_subres_range; }

	VkImageMemoryBarrier vk_IMB_swap_TSO_to_PRS[swap_image_count];
	for(int i = 0; i < swap_image_count; i++) {
		vk_IMB_swap_TSO_to_PRS[i].sType 					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_swap_TSO_to_PRS[i].pNext 					= NULL;
		vk_IMB_swap_TSO_to_PRS[i].srcAccessMask 			= 0;
		vk_IMB_swap_TSO_to_PRS[i].dstAccessMask 			= 0;
		vk_IMB_swap_TSO_to_PRS[i].oldLayout 				= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vk_IMB_swap_TSO_to_PRS[i].newLayout 				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		vk_IMB_swap_TSO_to_PRS[i].srcQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_swap_TSO_to_PRS[i].dstQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_swap_TSO_to_PRS[i].image 					= vk_image_swapimgs[i];
		vk_IMB_swap_TSO_to_PRS[i].subresourceRange 			= rpass_info.img_subres_range; }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RECORD IMAGEDATA INIT");	/**/
	///////////////////////////////////////////////////

	for(int i = 0; i < 1; i++) {
		vr("vkBeginCommandBuffer", &vkres, i,
			vkBeginCommandBuffer(combuf_work_imagedata_init[i].vk_command_buffer, &combuf_work_imagedata_init[i].comm_buff_begin_info) );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_work_imagedata_init[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_blit_imagedata_UND_to_TDO );

		vr("vkEndCommandBuffer", &vkres, i,
			vkEndCommandBuffer(combuf_work_imagedata_init[i].vk_command_buffer) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "SUBMIT IMAGEDATA INIT");	/**/
	///////////////////////////////////////////////////

	for(int i = 0; i < 1; i++) {
		if(valid) {
			rv("vkcombuf_work_init");
				qsync.sub_info.pCommandBuffers = &combuf_work_imagedata_init[i].vk_command_buffer;
			vr("vkQueueSubmit", &vkres, i,
				vkQueueSubmit(qsync.vk_queue, 1, &qsync.sub_info, VK_NULL_HANDLE) ); } }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RECORD IMAGEDATA OUT");	/**/
	///////////////////////////////////////////////////

	for(int i = 0; i < 2; i++) {
		vr("vkBeginCommandBuffer", &vkres, i,
			vkBeginCommandBuffer(combuf_work_imagedata[i].vk_command_buffer, &combuf_work_imagedata[i].comm_buff_begin_info) );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_work_imagedata[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_work_SRO_to_TSO[i] );

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
					1, &vk_IMB_work_TSO_to_SRO[i] );

		vr("vkEndCommandBuffer", &vkres, i,
			vkEndCommandBuffer(combuf_work_imagedata[i].vk_command_buffer) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RECORD BLIT IMGUI OUT");	/**/
	///////////////////////////////////////////////////

	for(int i = 0; i < swap_image_count; i++) {
		vr("vkBeginCommandBuffer", &vkres, i,
			vkBeginCommandBuffer(combuf_blit_imgui_loop[i].vk_command_buffer, &combuf_blit_imgui_loop[i].comm_buff_begin_info) );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_blit_imgui_loop[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_swap_PRS_to_TSO[i] );

			rv("vkCmdBlitImage");
				vkCmdBlitImage (
					combuf_blit_imgui_loop[i].vk_command_buffer, 
					vk_image_swapimgs[i], 		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					blit.vk_image, 				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1, &rpass_info.img_blit, 	VK_FILTER_NEAREST );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_blit_imgui_loop[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_swap_TSO_to_PRS[i] );

		vr("vkEndCommandBuffer", &vkres, i,
			vkEndCommandBuffer(combuf_blit_imgui_loop[i].vk_command_buffer) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RECORD BLIT2BUFF");		/**/
	///////////////////////////////////////////////////

	for(int i = 0; i < 1; i++) {
		vr("vkBeginCommandBuffer", &vkres, i,
			vkBeginCommandBuffer(combuf_blit2buff_sing[i].vk_command_buffer, &combuf_blit2buff_sing[i].comm_buff_begin_info) );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_blit2buff_sing[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_blit_TDO_to_TSO );

			rv("vkCmdCopyImageToBuffer");
				vkCmdCopyImageToBuffer (
					combuf_blit2buff_sing[i].vk_command_buffer, 
					blit.vk_image, 				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					blit2buff.vk_buffer,
					1, &rpass_info.buffer_img_cpy );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_blit2buff_sing[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_blit_TSO_to_TDO );

		vr("vkEndCommandBuffer", &vkres, i,
			vkEndCommandBuffer(combuf_blit2buff_sing[i].vk_command_buffer) ); }

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
					1, &vk_IMB_work_SRO_to_TSO[i/swap_image_count] );

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
					1, &vk_IMB_work_TSO_to_SRO[i/swap_image_count] );

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
		vk_present_info.pWaitSemaphores 		= &vk_semaphore_swapchain_pres;
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
//	uint32_t	imgdat_idx		= 0;	// Export Image Data Index

//	Timers
	NS_Timer 	ftime;
	NS_Timer 	optime;
	NS_Timer 	prstime;
	NS_Timer 	uitime;
	NS_Timer 	guitime;
	NS_Timer 	cmdtime;
	int  		current_sec		= time(0);
	int  		fps_freq 		= 1;
	int  		fps_report 		= time(0) - fps_freq;


	UB32_64 pcd 	= new_PCD_256();

//	Uniform Buffer Object ( 64 * 32 bits maximum )
	UB32_64 ub;
	SB_4096 sb;

	update_ub( &pcd, &ub );

	UI_info ui;
		ui.mx 	= 0;
		ui.my 	= 0;
		ui.mbl 	= 0;
		ui.mbr 	= 0;
		ui.cmd 	= 0;

	FT_info ft;
		ft.frame = 0;
		ft.seed	 = INIT_TIME%256;

	VW_info vw;
		vw.pmap = 0;
		vw.sdat	= 0;

	IMGUI_Config gc;
		gc.load_shader 					= false;
		gc.load_pattern 				= false;
		gc.load_pattern_confirm 		= false;
		gc.load_pattern_check_instant 	=  true;
		gc.load_pattern_check_reseed 	=  true;
		gc.load_pattern_random 			= false;
		gc.save_to_archive				= false;
		gc.mutate_menu 					= false;
		gc.mutate_full_random 			= false;
		gc.mutate_backstep				= false;
		gc.mutate_backstep_retry		= false;
		gc.mutate_flip 					= false;
		gc.throttle_menu 				= false;
		gc.throttle_enabled 			= false;
		gc.mode_planar					=  true;
		gc.mode_linear 					= false;
		gc.mode_circular 				= false;
		gc.mode_showdata 				= false;
		gc.scale_zoom_menu				= false;
		gc.scale_update					= false;
		gc.zoom_update					= false;
		gc.glfw_mod_LCTRL				= false;
		gc.glfw_mod_LSHIFT				= false;
		gc.show_notification_float		= false;
		gc.scale_has_panned				= false;
		gc.recording_config 			= false;
		gc.record_imgui					=  true;
		gc.load_A256_confirm 			=  true;

		gc.load_pattern_last_value 		= ei.load_pattern;
		gc.mutate_flip_str 				= 80;
		gc.mutate_backstep_idx 			= -1;
		gc.mutate_backstep_last_value 	= gc.mutate_backstep_idx;
		gc.throttle_target				= 32;
		gc.pmap_index					= 0;
		gc.pmap_index_last				= gc.pmap_index;
		gc.glfw_mouse_xpos_last			= ui.mx;
		gc.load_A256_index				= -1;
		gc.load_A256_index_last			= gc.load_A256_index;
		gc.load_A256_count				= get_PCD256_count("sav/PCD256_archive.vkpat");

		gc.notification_float_value		= 0.0f;

		memcpy(&gc.scale_value, &pcd.u32[62], sizeof(uint32_t));
		gc.scale_last_value				= gc.scale_value;

		memcpy(&gc.zoom_value, &pcd.u32[61], sizeof(uint32_t));
		gc.zoom_last_value				= gc.zoom_value;

	NS_Timer nottime;
		gc.notification_timer = nottime;
		send_notif(0, &gc);

	fspec256 fs;
	fsmag256 fsm = new_fsmag256();

	bool do_ub_update = true;

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "MAIN LOOP");				/**/
	///////////////////////////////////////////////////

//	Main Loop code
	do {
	//	Record loop start time
    	ftime = start_timer(ftime);

	//	Reset shader commands
		if(!ei.paused && ei.tick_loop == 0) { ui.cmd = 0; }
		clear_glfw_key(&glfw_key);
		clear_glfw_mouse(&glfw_mouse);

	//	Timing (seconds) setup
		current_sec = time(0);
		if( current_sec - fps_report >= fps_freq + 1) { fps_report = current_sec; }

	//	'Render'
		if(valid) {

		//	Lower the workload / FPS if paused
			if(ei.paused) { framesleep( 1000/60 ); } else {
			if(gc.throttle_enabled) { framesleep( gc.throttle_target ); } }

			if(!ei.run_headless) {
			//	Poll for GLFW window events
				glfwPollEvents();

			//	Does IMGUI want to capture input?
				ImGuiIO& io = ImGui::GetIO();

				kc.has_keyboard = io.WantCaptureKeyboard;
				kc.has_mouse 	= io.WantCaptureMouse;

				glfwSetKeyCallback( glfw_W, glfw_keyboard_event	);

				if(!ei.show_gui) { kc.has_mouse = false; kc.has_keyboard = false; }

				if( !kc.has_mouse ) {

					glfwSetCursorPosCallback 	( glfw_W, glfw_mousemove_event 	 );
					glfwSetMouseButtonCallback	( glfw_W, glfw_mouseclick_event	 );
					glfwSetScrollCallback		( glfw_W, glfw_mousescroll_event );
					ui.mx = glfw_mouse.xpos;
					ui.my = glfw_mouse.ypos;

				//	MBR: Scale Panning
					if( gc.glfw_mouse_xpos_last != glfw_mouse.xpos
					&& 	glfw_mouse.button 		== 1
					&&	glfw_mouse.action 		!= 0
					&& 	gc.glfw_mod_LSHIFT ) {
								gc.scale_has_panned		= true;
						int 	mx_offset 				= glfw_mouse.xpos - gc.glfw_mouse_xpos_last;
						float 	xscale	 				= float((APP_W / 2.0) - float(mx_offset)) / float(APP_W);
								gc.scale_value 			= gc.scale_value + gc.scale_value * ((xscale - 0.5) * 2.0) * (1.0 / (1.0 + gc.zoom_value));
								gc.scale_update			= true;
						send_notif_float(15, gc.scale_value, &gc); }

					gc.glfw_mouse_xpos_last	= glfw_mouse.xpos;

				//	Left Click
					if( glfw_mouse.button == 0 ) { ui.mbl = glfw_mouse.action; }

				//	Right Click
					if( glfw_mouse.button == 1 ) { ui.mbr = glfw_mouse.action; }
					if( glfw_mouse.button == 1
					&&  gc.glfw_mod_LSHIFT ) 	 { ui.mbr = 0; }

				//	Middle Mouse / ScrollWheel Click
					if( glfw_mouse.button == 2
					&&	glfw_mouse.action == 1 ) {
						glfw_mouse.action  				= 0;
						gc.mutate_backstep_last_value 	= gc.mutate_backstep_idx;
						do_action(  9, &ui, &ei, &gc );
						if( gc.glfw_mod_LSHIFT ) { 
							gc.save_to_archive = true;
							gc.load_A256_index = -1; } }

				//	Mouse Wheel Up
					if( glfw_mouse.yoffset == -1 ) {
						glfw_mouse.yoffset  =  0;
						do_action( 36, &ui, &ei, &gc ); }

				//	Mouse Wheel Down
					if( glfw_mouse.yoffset ==  1 ) {
						glfw_mouse.yoffset  =  0;
						do_action( 35, &ui, &ei, &gc ); }

				//	Mouse Back
					if(glfw_mouse.button == 3 && glfw_mouse.action == 1) {
						glfw_mouse.action = 0;
						do_action( 38, &ui, &ei, &gc ); }

				//	Mouse Forward
					if(glfw_mouse.button == 4 && glfw_mouse.action == 1) {
						glfw_mouse.action = 0;
						do_action( 37, &ui, &ei, &gc ); } }

				else { ui.mbl = 0; ui.mbr = 0; }

				if( !kc.has_keyboard ) {

				//	Release 	L-Shift 	Reseed
					if( glfw_key.key 	== GLFW_KEY_LEFT_SHIFT 
					&&	glfw_key.action	== 0
					&& !gc.scale_has_panned ) { do_action( 11, &ui, &ei, &gc ); }

				//	Press 		L-Shift 	Sticky Modifier
					if( glfw_key.key 	== GLFW_KEY_LEFT_SHIFT
					&&	glfw_key.action	== 1 ) { gc.glfw_mod_LSHIFT = true; }

				//	Release 	L-Shift 	UnSticky Modifier
					if( glfw_key.key 	== GLFW_KEY_LEFT_SHIFT 
					&&	glfw_key.action	== 0 ) { gc.glfw_mod_LSHIFT = false; gc.scale_has_panned = false; }

				//	Press 		X 			Clear
					if( glfw_key.key 	== GLFW_KEY_X
		 			&& 	glfw_key.action >= 1 ) { do_action( 12, &ui, &ei, &gc ); }

				//	Press 		Z 			SymSeed
					if( glfw_key.key 	== GLFW_KEY_Z
		 			&& 	glfw_key.action >= 1 ) { do_action( 13, &ui, &ei, &gc ); }

				//	Press 		C 			BlendSeed
					if( glfw_key.key 	== GLFW_KEY_C
		 			&& 	glfw_key.action >= 1 ) { do_action( 33, &ui, &ei, &gc ); }

				//	Press 		TAB			Show Random Archive Pattern
					if( glfw_key.key 	== GLFW_KEY_TAB
		 			&& 	glfw_key.action == 1 ) { do_action( 39, &ui, &ei, &gc ); }

				//	Press 		RIGHT 		Show Prev Archive Pattern
					if( glfw_key.key 	== GLFW_KEY_RIGHT
		 			&& 	glfw_key.action >= 1 ) { do_action( 37, &ui, &ei, &gc ); }

				//	Press 		LEFT		Show Next Archive Pattern
					if( glfw_key.key 	== GLFW_KEY_LEFT
		 			&& 	glfw_key.action >= 1 ) { do_action( 38, &ui, &ei, &gc ); }

				//	Press 		CTRL-S		Save Archive Pattern
					if( glfw_key.key 	== GLFW_KEY_S
		 			&& (glfw_key.mods & GLFW_MOD_CONTROL)
		 			&& 	glfw_key.action == 1 ) { do_action(  7, &ui, &ei, &gc ); }

				//	Press 		T 			Toggle Throttle
					if( glfw_key.key 	== GLFW_KEY_T
		 			&& 	glfw_key.action >= 1 ) { do_action( 18, &ui, &ei, &gc ); }

				//	Press 		R 			Full Randomization
					if( glfw_key.key 	== GLFW_KEY_R
		 			&& 	glfw_key.action == 1 ) { do_action(  8, &ui, &ei, &gc ); }

				//	Press 		V 			Mutate Target
					if( glfw_key.key 	== GLFW_KEY_V
		 			&& 	glfw_key.action == 1 ) { do_action( 10, &ui, &ei, &gc ); }

				//	Press 		Q 			Reload Target
					if( glfw_key.key 	== GLFW_KEY_Q
		 			&& 	glfw_key.action == 1 ) { do_action( 24, &ui, &ei, &gc ); }

				//	Press 		KP_ENTER 	Toggle Recording
					if( glfw_key.key 	== GLFW_KEY_KP_ENTER
		 			&& 	glfw_key.action == 1 ) { do_action( 20, &ui, &ei, &gc ); }

				//	Press 		KP_ADD 		Increase Export Freq
					if( glfw_key.key 	== GLFW_KEY_KP_ADD
		 			&& 	glfw_key.action >= 1 ) { do_action( 26, &ui, &ei, &gc ); }

				//	Press 		KPSUBTRACT	Decrease Export Freq
					if( glfw_key.key 	== GLFW_KEY_KP_SUBTRACT
		 			&& 	glfw_key.action >= 1 ) { do_action( 27, &ui, &ei, &gc ); }

				//	Press 		1 			Planar Mapping
					if( glfw_key.key 	== GLFW_KEY_1
		 			&& 	glfw_key.action == 1 ) { do_action( 30, &ui, &ei, &gc ); do_action( 29, &ui, &ei, &gc ); }

				//	Press 		2 			Linear Mapping
					if( glfw_key.key 	== GLFW_KEY_2
		 			&& 	glfw_key.action == 1 ) { do_action( 31, &ui, &ei, &gc ); do_action( 29, &ui, &ei, &gc ); }

				//	Press 		3 			Circular Mapping
					if( glfw_key.key 	== GLFW_KEY_3
		 			&& 	glfw_key.action == 1 ) { do_action( 32, &ui, &ei, &gc ); do_action( 29, &ui, &ei, &gc ); }

				//	Press 		S 			Step[1]
					if( glfw_key.key 	== GLFW_KEY_S
		 			&& !(glfw_key.mods & GLFW_MOD_CONTROL)
		 			&& 	glfw_key.action >= 1 ) { do_action(  2, &ui, &ei, &gc ); }

				//	Press 		SPACE 		Toggle Pause
					if( glfw_key.key 	== GLFW_KEY_SPACE
		 			&& 	glfw_key.action == 1 ) { do_action(  1, &ui, &ei, &gc ); }

				//	Press 		ESCAPE 		Toggle IMGUI
					if( glfw_key.key 	== GLFW_KEY_ESCAPE
		 			&& 	glfw_key.action == 1 ) { do_action( 0, &ui, &ei, &gc ); } }

			//	Notifications
				if( gc.show_notification ) {
					uint32_t uint_notif_age =
						std::chrono::duration_cast<std::chrono::nanoseconds> (
							std::chrono::high_resolution_clock::now()
						- 	gc.notification_timer.st ).count();
					if( uint_notif_age < 1200000000 ) 	{ gc.notification_age = 1.0f - float(uint_notif_age) / float(1800000000); }
					if( uint_notif_age > 2400000000 ) 	{ tog(&gc.show_notification); } }

				if( gc.show_notification_float ) {
					uint32_t uint_notif_age =
						std::chrono::duration_cast<std::chrono::nanoseconds> (
							std::chrono::high_resolution_clock::now()
						- 	gc.notification_float_timer.st ).count();
					if( uint_notif_age < 1200000000 ) 	{ gc.notification_float_age = 1.0f - float(uint_notif_age) / float(1800000000); }
					if( uint_notif_age > 2400000000 ) 	{ tog(&gc.show_notification_float); } }


				if( gc.load_A256_confirm ) {
					gc.load_A256_confirm = false;
					ui.cmd = 1;
					ei.tick_loop = 1;
					pcd = load_PCD256("sav/PCD256_archive.vkpat", gc.load_A256_index);
					do_ub_update = true;
					memcpy(&gc.scale_value, &pcd.u32[62], sizeof(uint32_t));
					memcpy(&gc.zoom_value, &pcd.u32[61], sizeof(uint32_t));
					gc.scale_update = true;
					gc.zoom_update = true;
					for(int j = 0; j < 16; j++) {
						for(int i = 0; i < 48; i++) { sb.ub[j].u32[i] = pcd.u32[i]; }
					} }

			//	Load random Archive pattern
				if( gc.load_pattern_confirm ) {
					gc.load_pattern_confirm = false;
					if( gc.load_pattern_random ) {
						gc.load_pattern_random = false;
					//	Depreicated patterns under 17000
						ei.load_pattern = (rand()%(ei.PCD_count - 18080)) + 18080;	}
					loadPattern_PCD408_to_256( &ei, &pcd );
					do_ub_update = true;
					memcpy(&gc.scale_value, &pcd.u32[62], sizeof(uint32_t));
					memcpy(&gc.zoom_value, &pcd.u32[61], sizeof(uint32_t));
					if(gc.load_pattern_check_reseed) { ui.cmd = 1; ei.tick_loop = 1; } }

//
				if( gc.mutate_set_target ) {
					loglevel = MAXLOG;
					uint32_t panel_index = (uint32_t((float(ui.my)/float(APP_H))*float(panel_n_x_n))*panel_n_x_n) +(uint32_t((float(ui.mx)/float(APP_W))*float(panel_n_x_n)));
					ov("INDEX",panel_index);
					loglevel = -1;
					gc.mutate_set_target = false;
					for(int i = 0; i < 48; i++) { pcd.u32[i] = sb.ub[panel_index].u32[i]; }
					save_PCD256("sav/PCD256_global_all.vkpat", &pcd); }

				if( gc.mutate_backstep ) {
					gc.mutate_backstep = false;
					ui.cmd = 1;
					ei.tick_loop = 1;
					pcd = load_PCD256("sav/PCD256_global_all.vkpat", gc.mutate_backstep_idx);
					do_ub_update = true;
					memcpy(&gc.scale_value, &pcd.u32[62], sizeof(uint32_t));
					memcpy(&gc.zoom_value, &pcd.u32[61], sizeof(uint32_t));
					gc.scale_update = true;
					gc.zoom_update = true;
					for(int j = 0; j < 16; j++) {
						for(int i = 0; i < 48; i++) { sb.ub[j].u32[i] = pcd.u32[i]; }
					} }

				if( gc.mutate_full_random ) {
					gc.mutate_full_random = false;
					ui.cmd = 1;
					ei.tick_loop = 1;
					for(int i = 0; i < 48; i++) { pcd.u32[i] = mut_rnd(); }
					for(int j = 1; j < 16; j++) {
						for(int i = 0; i < 48; i++) { sb.ub[j].u32[i] = mut_rnd(); }
					}
					do_ub_update = true;
					save_PCD256("sav/PCD256_global_all.vkpat", &pcd); }

				if( gc.mutate_flip ) {
					gc.mutate_flip = false;
					ui.cmd = 1;
					ei.tick_loop = 1;

					for(int j = 1; j < 16; j++) {
						for(int i = 0; i < 48; i++) { sb.ub[j].u32[i] = bit_flp( pcd.u32[i], (rand()%(gc.mutate_flip_str*2))+(gc.mutate_flip_str/2)+1 ); }
					}

					for(int i = 0; i < 48; i++) { pcd.u32[i] = bit_flp( pcd.u32[i], (rand()%(gc.mutate_flip_str*2))+(gc.mutate_flip_str/2)+1 ); }

					for(int i = 0; i < 48; i++) {
						sb.ub[0].u32[i] = pcd.u32[i]; }

					do_ub_update = true;
					save_PCD256("sav/PCD256_global_all.vkpat", &pcd); }

				if( gc.mutate_backstep_retry ) {
					gc.mutate_backstep_retry = false;
					gc.mutate_backstep_idx--;
					gc.mutate_backstep_last_value = gc.mutate_backstep_idx; }

			//	Update special floats
				//	[62]	'Scale' value
				if( gc.scale_update ) {
					gc.scale_update = false;

					float fsc = gc.scale_value;
					memcpy(&pcd.u32[62], &fsc, sizeof(uint32_t));
					do_ub_update = true; }
				//	[61]	'Zoom' value
				if( gc.zoom_update ) {
					gc.zoom_update = false;
					float fzm = gc.zoom_value;// + gc.zoom_value * (float(sound) / float(INT16_MAX)) * 1.0f;
					memcpy(&pcd.u32[61], &fzm, sizeof(uint32_t));
					do_ub_update = true;	}

			//	Save current target to PCD256 Archive
				if( gc.save_to_archive ) {
					gc.save_to_archive = false;
					save_PCD256("sav/PCD256_archive.vkpat", &pcd);
					gc.load_A256_count = get_PCD256_count("sav/PCD256_archive.vkpat");
					send_notif(1, &gc); }

				if(ei.show_gui || ei.paused) {
					guitime = start_timer(guitime);
						ImGui_ImplVulkan_NewFrame();
						ImGui_ImplGlfw_NewFrame();
						ImGui::NewFrame();
						imgui_menu( glfw_W, &ui, &ei, &gc );
					//	ImGui::ShowDemoWindow();	// TODO TODO
						ImGui::Render();
					end_timer(guitime, "IMGUI Build Time");

					cmdtime = start_timer(cmdtime);
						for(int i = 0; i < swap_image_count; i++) {
							vr("vkBeginCommandBuffer", &vkres, i,
								vkBeginCommandBuffer(combuf_imgui_loop[i].vk_command_buffer, &combuf_imgui_loop[i].comm_buff_begin_info) );

								rv("vkCmdBeginRenderPass");
									vkCmdBeginRenderPass (
										combuf_imgui_loop[i].vk_command_buffer,
										&vkrpbegininfo_imgui[i%swap_image_count],
										VK_SUBPASS_CONTENTS_INLINE );

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

			if( do_ub_update ) { update_ub( &pcd, &ub ); update_ub( &ub, &sb.ub[0] ); }
		//	Update Uniform Buffer values
		//		[62]	'Scale' value
		//	if( gc.scale_update ) {
		//		gc.scale_update = false;
		//		memcpy(&pcd.u32[62], &gc.scale_value, sizeof(uint32_t));
		//		update_ub( &pcd, &ub );	}
		//		[61]	'Zoom' value
		//	if( gc.zoom_update ) {
		//		gc.zoom_update = false;
		//		memcpy(&pcd.u32[61], &gc.zoom_value, sizeof(uint32_t));
		//		update_ub( &pcd, &ub );	}
		//		[60]	Mouse info & Command IDs
			ub.u32[60] 		= pack_ui_info(ui);
		//		[59]	Views/Modes
			vw.pmap		= gc.pmap_index;
			vw.sdat		= (gc.mode_showdata) ? 1u : 0u;
			ub.u32[59] 	= pack_vw_info(vw);
			pcd.u32[59] = ub.u32[59];
		//		[63]	Frame Index, Time-Seed
			ft.frame 	= frame_index;
			ub.u32[63]	= pack_ft_info(ft);

		//	Send UB values to GPU
			rv("memcpy");
				memcpy(pvoid_memmap_work, &ub, sizeof(ub));


		//	Send SSBO values to GPU
//			sb.ub[0] = ub;
			rv("memcpy");
				memcpy(pvoid_memmap_ssbo, &sb, sizeof(sb));

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
						vkAcquireNextImageKHR(vob.VKL, vk_swapchain, UINT64_MAX, vk_semaphore_swapchain_img_acq, VK_NULL_HANDLE, &swap_image_index) );
					ov("swap_image_index", swap_image_index); }

				if(valid) {
				//	Copy the "Work" Image to the "Swap" Image
					rv("vkcombuf_pres");
						swpsync.sub_info.pCommandBuffers = &combuf_pres_loop[swap_image_index+((frame_index%2)*swap_image_count)].vk_command_buffer;
					VkFence f = (ei.show_gui || ei.paused) ? VK_NULL_HANDLE : qsync.vk_fence;
					swpsync.sub_info.pSignalSemaphores = (ei.show_gui || ei.paused) ? &vk_semaphore_swapchain_imgui : &vk_semaphore_swapchain_pres;
					vr("vkQueueSubmit", &vkres, swpsync.sub_info.pCommandBuffers,
						vkQueueSubmit(swpsync.vk_queue, 1, &swpsync.sub_info, f) ); }

				if(valid && (ei.show_gui || ei.paused)) {
				//	Add IMGUI interface
					rv("vkcombuf_IMGUI");
						swpsync_imgui.sub_info.pCommandBuffers = &combuf_imgui_loop[swap_image_index].vk_command_buffer;
					VkFence f = qsync.vk_fence;
					vr("vkQueueSubmit", &vkres, swpsync_imgui.sub_info.pCommandBuffers,
						vkQueueSubmit(swpsync_imgui.vk_queue, 1, &swpsync_imgui.sub_info, f) ); } }

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

			if((!ei.paused || ei.tick_loop)
			&& 	valid
			&&  ei.export_enabled
			&&	ei.export_frequency > 0
			&&	frame_index % ei.export_frequency == 0
			&& 	frame_index > 0) {
			//	Submit 'imagedata' commands to the GPU graphics queue
				bool get_gui = false;
				if(ei.run_headless || !gc.record_imgui || !ei.show_gui) {
					rv("combuf_work_imagedata");
						qsync.sub_info.pCommandBuffers = &combuf_work_imagedata[(frame_index+0)%2].vk_command_buffer; }
				else {
					get_gui = true;
					rv("combuf_work_imagedata");
						qsync.sub_info.pCommandBuffers = &combuf_blit_imgui_loop[swap_image_index].vk_command_buffer; }
				vr("vkQueueSubmit", &vkres, qsync.sub_info.pCommandBuffers,
					vkQueueSubmit(qsync.vk_queue, 1, &qsync.sub_info, VK_NULL_HANDLE) );
				vr("vkDeviceWaitIdle", &vkres, "IDLE",
					vkDeviceWaitIdle(vob.VKL) );

			//	TODO use own sync object?
				rv("combuf_work_imagedata");
					qsync.sub_info.pCommandBuffers = &combuf_blit2buff_sing[0].vk_command_buffer;
				vr("vkQueueSubmit", &vkres, qsync.sub_info.pCommandBuffers,
					vkQueueSubmit(qsync.vk_queue, 1, &qsync.sub_info, VK_NULL_HANDLE) );
				vr("vkDeviceWaitIdle", &vkres, "IDLE",
					vkDeviceWaitIdle(vob.VKL) );
				

				optime = start_timer(optime);
				if(!verbose_loops) { loglevel = MAXLOG; }
				save_image(pvoid_blit2buff, "IMG"+std::to_string(ei.imgdat_idx), APP_W, APP_H, glfw_mouse, get_gui);
				end_timer(optime, "Save ImageData");
				if(!verbose_loops) { loglevel = -1; }
				ei.imgdat_idx++;
				dft1d(ei.imgdat_idx*735, 512, &fs, &fsm);
				if( ei.export_batch_size  > 0
				&&	ei.export_batch_left  > 0 ) { ei.export_batch_left--; }
				if( !ei.run_headless
				&&	ei.export_batch_size  > 0
				&&	ei.export_batch_left == 0 ) {
					ei.paused = true;
					ei.export_batch_left =  ei.export_batch_size; } }

		}

	//	End of loop
		if(fps_report == current_sec) {
			fps_report--;
			if(!verbose_loops) { loglevel = MAXLOG; }
			end_timer(ftime, "Full Loop Time");
			if(!verbose_loops) { loglevel = -1; } }

		if(verbose_loops) { end_timer(ftime, "Full Loop Time"); }

		hd("STAGE:", "LOOP");

		if(ei.tick_loop > 0) { ei.tick_loop--; }
		if(verbose_loops > 0) { verbose_loops--; } else { loglevel = -1; }

		do_ub_update = false;

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
