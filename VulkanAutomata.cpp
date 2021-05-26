//#define VK_USE_PLATFORM_XLIB_KHR
#include <string>
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>
#include <fstream>
#include <chrono>
#include <cstring>
//#include <X11/Xlib.h>

const 	uint32_t 	VERT_FLS 		= 1;	//	Number of Vertex Shader Files
const 	uint32_t 	FRAG_FLS 		= 1;	//	Number of Fragment Shader Files

		int loglevel 	= 3;
		int valid 		= 1;

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
	if(loglevel >= 0) {
		std::cout << "\n\n" << bar << "\n " << msg_fmt << "\n" 	<< bar << "\n\n"; }
	valid = 0;
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
	if(loglevel >= 1) {
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
	reslist->push_back(res);
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
	if(loglevel >= 1) {
		std::cout << "  " << idx_string << ":\t" << (res==0?" ":res_string) << " \t" << id << pad << " [" << v << "]\n"; } }

void rv(const std::string& id) {
//	Return void output message
	if(loglevel >= 1) {
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

// TODO [2] = implementation specific ?
struct VK_DescSetLayout {
	VkDescriptorSetLayoutBinding		set_bind[2];
	VkDescriptorSetLayoutCreateInfo		set_info;
	VkDescriptorPoolSize				pool_size[2];
	VkDescriptorPoolCreateInfo			pool_info[2];
	VkDescriptorSetLayout				vk_desc_set_layout;
	VkDescriptorPool					vk_desc_pool[2];
	VkDescriptorSetAllocateInfo			allo_info[2];
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

struct WSize {
	uint32_t 	app_w;
	uint32_t 	app_h;
	uint32_t 	divs;
	uint32_t 	mode; };
uint32_t wsize_pack(WSize ws) {
	uint32_t packed_ui32 = 
		( (uint32_t)ws.app_w		  )
	+ 	( (uint32_t)ws.app_h 	<< 12 )
	+ 	( (uint32_t)ws.divs 	<< 24 )
	+ 	( (uint32_t)ws.mode 	<< 28 );
	return packed_ui32; }

struct MInfo {
	uint32_t 	mouse_x;
	uint32_t 	mouse_y;
	uint32_t 	mouse_c;
	uint32_t 	run_cmd; };
uint32_t minfo_pack(MInfo mi) {
	uint32_t packed_ui32 = 
		( (uint32_t)mi.mouse_c	   	  )
	+ 	( (uint32_t)mi.mouse_x 	<< 4  )
	+ 	( (uint32_t)mi.mouse_y 	<< 16 )
	+ 	( (uint32_t)mi.run_cmd  << 28 );
	return packed_ui32; }

int main(void) {

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "APPLICATION CONFIG");		/**/
	///////////////////////////////////////////////////

	const	uint32_t 	APP_W 			= 768;		//	1920 1536 1280	768	512	384	256
	const	uint32_t 	APP_H 			= 432;		//	1080 864  720	432	288	216	144
	const	long 		FPS 			= 0;
	const	long 		NS_DELAY 		= (FPS==0) ? 1 : 1000000000 / FPS; 				//	Nanosecond Delay

	const	float 		TRIQUAD_SCALE 	= 1.0;											//	Vertex Shader Triangle Scale
	const	float 		VP_SCALE 		= TRIQUAD_SCALE + (1.0-TRIQUAD_SCALE) * 0.5;	//	Vertex Shader Viewport Scale

	const 	uint32_t 	INST_EXS 		= 3;	//	Number of Vulkan Instance Extensions
	const 	uint32_t 	LDEV_EXS 		= 1;	//	Number of Vulkan Logical Device Extensions
	const 	uint32_t 	VLID_LRS 		= 1;	//	Number of Vulkan Validation Layers

//	Paths to shader files and extension names
	const 	char* 	filepath_vert		[VERT_FLS]
	=	{	"./app/vert_TriQuad.spv" 			};
	const 	char* 	filepath_frag		[FRAG_FLS]
	=	{	"./app/frag_automata0000.spv"		};
	const 	char* 	instance_extensions	[INST_EXS]
	=	{	"VK_KHR_surface", 
			"VK_KHR_xlib_surface", 
			"VK_EXT_debug_utils"				};
	const 	char* 	validation_layers	[VLID_LRS]
	=	{	"VK_LAYER_KHRONOS_validation" 		};
	const 	char* 	device_extensions	[LDEV_EXS]
	=	{	"VK_KHR_swapchain"					};

//	Config Notification Messages
	ov("Application Width", 	APP_W	);
	ov("Application Height", 	APP_H	);
	ov("FPS Target", 			FPS		);
	for(int i = 0; i < VERT_FLS; i++) {	iv("Vertex Shaders", 			filepath_vert[i], 		i ); }
	for(int i = 0; i < FRAG_FLS; i++) {	iv("Fragment Shaders", 			filepath_frag[i], 		i ); }
	for(int i = 0; i < INST_EXS; i++) {	iv("Instance Extensions", 		instance_extensions[i], i ); }
	for(int i = 0; i < LDEV_EXS; i++) {	iv("Logical Device Extensions", device_extensions[i], 	i ); }
	for(int i = 0; i < VLID_LRS; i++) {	iv("Validation Layers", 		validation_layers[i], 	i ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "VULKAN INIT");			/**/
	///////////////////////////////////////////////////

//	VkResult storage
	std::vector<VkResult> vkres;
	vr("init", &vkres, "NULL", VK_ERROR_UNKNOWN);

	VK_Obj vob;

	VK_Config vkcfg;

		vkcfg.app_info.sType						= VK_STRUCTURE_TYPE_APPLICATION_INFO;
		vkcfg.app_info.pNext						= NULL;
		vkcfg.app_info.pApplicationName				= "VulkanAutomata";
		vkcfg.app_info.applicationVersion			= 0;
		vkcfg.app_info.pEngineName					= NULL;
		vkcfg.app_info.engineVersion				= 0;
		vkcfg.app_info.apiVersion					= VK_API_VERSION_1_2;

		vkcfg.inst_info.sType 						= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	nf(&vkcfg.inst_info);
		vkcfg.inst_info.pApplicationInfo 			= &vkcfg.app_info;
		vkcfg.inst_info.enabledLayerCount 			= VLID_LRS;
		vkcfg.inst_info.ppEnabledLayerNames 		= validation_layers;
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
	 /**/	hd("STAGE:", "LOGICAL DEVICE");			/**/
	///////////////////////////////////////////////////

	VK_LogDev ldev;

		ldev.ldev_info.sType 						= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	nf(&ldev.ldev_info);
		ldev.ldev_info.queueCreateInfoCount 		= 1;
		ldev.ldev_info.pQueueCreateInfos 			= &pdq.pdq_info;
		ldev.ldev_info.enabledLayerCount 			= 0;
		ldev.ldev_info.ppEnabledLayerNames 			= NULL;
		ldev.ldev_info.enabledExtensionCount 		= 1;
		ldev.ldev_info.ppEnabledExtensionNames 		= device_extensions;
		ldev.ldev_info.pEnabledFeatures 			= &pdev[vob.VKP_i].vk_pdev_feats;

	vr("vkCreateDevice", &vkres, vob.VKL,
		vkCreateDevice(vob.VKP, &ldev.ldev_info, NULL, &vob.VKL) );
	ov("VkDevice", vob.VKL);

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
												| VK_IMAGE_USAGE_TRANSFER_SRC_BIT
												| VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
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
		rpass_info.vk_viewport.width					= APP_W * VP_SCALE;
		rpass_info.vk_viewport.height					= APP_H * VP_SCALE;
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
		pipe_info.p_cbat_info.srcColorBlendFactor		= VK_BLEND_FACTOR_ZERO;
		pipe_info.p_cbat_info.dstColorBlendFactor		= VK_BLEND_FACTOR_ZERO;
		pipe_info.p_cbat_info.colorBlendOp				= VK_BLEND_OP_ADD;
		pipe_info.p_cbat_info.srcAlphaBlendFactor		= VK_BLEND_FACTOR_ZERO;
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
		dsl_work.pool_size[0].descriptorCount 		= 3;	// ??? TODO
		dsl_work.pool_size[1].type 					= dsl_work.set_bind[1].descriptorType;
		dsl_work.pool_size[1].descriptorCount 		= 7;	// ??? TODO

	for(int i = 0; i < 2; i++) {
		dsl_work.pool_info[i].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	nf(&dsl_work.pool_info[i]);
		dsl_work.pool_info[i].maxSets 			= 1; // 2 ? // TODO
		dsl_work.pool_info[i].poolSizeCount 	= 1;
		dsl_work.pool_info[i].pPoolSizes 		= &dsl_work.pool_size[i]; }

	vr("vkCreateDescriptorSetLayout", &vkres, dsl_work.vk_desc_set_layout,
		vkCreateDescriptorSetLayout(vob.VKL, &dsl_work.set_info, NULL, &dsl_work.vk_desc_set_layout) );

	for(int i = 0; i < 2; i++) {
	vr("vkCreateDescriptorPool", &vkres, dsl_work.vk_desc_pool[i],
		vkCreateDescriptorPool(vob.VKL, &dsl_work.pool_info[i], NULL, &dsl_work.vk_desc_pool[i]) );

		dsl_work.allo_info[i].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		dsl_work.allo_info[i].pNext 				= NULL;
		dsl_work.allo_info[i].descriptorPool 		= dsl_work.vk_desc_pool[i];
		dsl_work.allo_info[i].descriptorSetCount 	= 1;
		dsl_work.allo_info[i].pSetLayouts 			= &dsl_work.vk_desc_set_layout;

		vr("vkAllocateDescriptorSets", &vkres, dsl_work.vk_descriptor_set[i],
			vkAllocateDescriptorSets(vob.VKL, &dsl_work.allo_info[i], &dsl_work.vk_descriptor_set[i]) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "WORK SAMPLER");			/**/
	///////////////////////////////////////////////////

	VkDescriptorImageInfo vkdescimg_info[2];
		vkdescimg_info[0].sampler			= rpass_info.vk_sampler;
		vkdescimg_info[0].imageView			= work_init[1].vk_image_view; // Reference the other image
		vkdescimg_info[0].imageLayout		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vkdescimg_info[1].sampler			= rpass_info.vk_sampler;
		vkdescimg_info[1].imageView			= work_init[0].vk_image_view; // Reference the other image
		vkdescimg_info[1].imageLayout		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkWriteDescriptorSet vkwritedescset[2];
	for(int i = 0; i < 2; i++) {
		vkwritedescset[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vkwritedescset[i].pNext 				= NULL;
		vkwritedescset[i].dstSet 				= dsl_work.vk_descriptor_set[i];
		vkwritedescset[i].dstBinding 			= 1;
		vkwritedescset[i].dstArrayElement 		= 0;
		vkwritedescset[i].descriptorCount 		= 1;
		vkwritedescset[i].descriptorType 		= dsl_work.set_bind[0].descriptorType;
		vkwritedescset[i].pImageInfo 			= &vkdescimg_info[i];
		vkwritedescset[i].pBufferInfo 			= NULL;
		vkwritedescset[i].pTexelBufferView 		= NULL; }

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
			vkUpdateDescriptorSets(vob.VKL, 1, &vkwritedescset[i], 0, NULL);
		rv("vkUpdateDescriptorSets");
			vkUpdateDescriptorSets(vob.VKL, 1, &vkwritedescset_ub_work[i], 0, NULL); }

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
	 /**/	hd("STAGE:", "MAIN LOOP INIT");			/**/
	///////////////////////////////////////////////////

	uint32_t 	frame_index 	= 0;

//	FPS tracking init
    auto 		start_frame 	= std::chrono::high_resolution_clock::now();
	auto 		finish_frame 	= start_frame;
	int  		current_sec		= time(0);
	int  		fps_freq 		= 1;
	int  		fps_report 		= time(0) - fps_freq;

//	Unpacked uint32_t Uniform Buffer Object
	WSize 	window_size;
			window_size.app_w	= APP_W;
			window_size.app_h	= APP_H;
			window_size.divs	= 1;
			window_size.mode  	= 0;

//	Unpacked uint32_t Uniform Buffer Object
	MInfo 	mouse_info;
			mouse_info.mouse_x 	= 0; 	//	Mouse X Position
			mouse_info.mouse_y 	= 0; 	//	Mouse Y Position
			mouse_info.mouse_c 	= 0; 	//	Mouse Button / UI State info
			mouse_info.run_cmd 	= 0; 	//	Command Codes

//	Uniform Buffer Object ( 64 * 32 bits maximum )
	UniBuf ub;
		ub.v0  = 0; ub.v1  = 0; ub.v2  = 0; ub.v3  = 0;	ub.v4  = 0; ub.v5  = 0; ub.v6  = 0; ub.v7  = 0;
		ub.v8  = 0; ub.v9  = 0; ub.v10 = 0; ub.v11 = 0;	ub.v12 = 0; ub.v13 = 0; ub.v14 = 0; ub.v15 = 0;
		ub.v16 = 0; ub.v17 = 0; ub.v18 = 0; ub.v19 = 0;	ub.v20 = 0; ub.v21 = 0; ub.v22 = 0; ub.v23 = 0;
		ub.v24 = 0; ub.v25 = 0; ub.v26 = 0; ub.v27 = 0;	ub.v28 = 0; ub.v29 = 0; ub.v30 = 0; ub.v31 = 0;
		ub.v32 = 0; ub.v33 = 0; ub.v34 = 0; ub.v35 = 0;	ub.v36 = 0; ub.v37 = 0; ub.v38 = 0; ub.v39 = 0;
		ub.v40 = 0; ub.v41 = 0; ub.v42 = 0; ub.v43 = 0;	ub.v44 = 0; ub.v45 = 0; ub.v46 = 0; ub.v47 = 0;
		ub.v48 = 0; ub.v49 = 0; ub.v50 = 0; ub.v51 = 0;	ub.v52 = 0; ub.v53 = 0; ub.v54 = 0; ub.v55 = 0;
		ub.v56 = 0; ub.v57 = 0; ub.v58 = 0; ub.v59 = 0;	ub.v60 = 0; ub.v61 = 0; ub.v62 = 0; ub.v63 = 0;

	//	Pack and assign custom uint32_t objects to ub.v(xx) slots
		ub.v62 = minfo_pack( mouse_info  );
		ub.v63 = wsize_pack( window_size );

//	Map the memory location on the GPU for memcpy() to submit the Uniform Buffer
	void *pvoid_memmap_work;
	vr("vkMapMemory", &vkres, pvoid_memmap_work,
		vkMapMemory(vob.VKL, vkdevmem_ub_work, vkDescBuff_info_work.offset, vkDescBuff_info_work.range, 0, &pvoid_memmap_work) );

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "MAIN LOOP");				/**/
	///////////////////////////////////////////////////

//	Main Loop code
	do {
	//	FPS timing setup
		current_sec = time(0);
		if( current_sec - fps_report >= fps_freq + 1) { fps_report = current_sec; }

	//	'Render'
		if(valid) {
		//	Record loop start time
    		start_frame = std::chrono::high_resolution_clock::now();

		//	Report current frame index
			ov("frame_index", frame_index);

		//	Update Uniform Buffer values
			ub.v61 = frame_index;

		//	Send UB values to GPU
			rv("memcpy");
				memcpy(pvoid_memmap_work, &ub, sizeof(ub));

			if(valid) {
			//	Submit 'work' commands to the GPU graphics queue
				rv("vkcombuf_work");
					qsync.sub_info.pCommandBuffers = &combuf_work_loop[frame_index%2].vk_command_buffer;
				if(valid) {
					vr("vkQueueSubmit", &vkres, qsync.sub_info.pCommandBuffers,
						vkQueueSubmit(qsync.vk_queue, 1, &qsync.sub_info, qsync.vk_fence) ); } }

				if(valid) {
				//	Wait for the queued commands to finish execution
					do {
						vr("vkWaitForFences <100ms>", &vkres, qsync.vk_fence,
							vkWaitForFences(vob.VKL, 1, &qsync.vk_fence, VK_TRUE, 100000000) );
					} while (vkres[vkres.size()-1] == VK_TIMEOUT); }

				if(valid) {
				//	Reset the fence for reuse in the next iteration
					vr("vkResetFences", &vkres, qsync.vk_fence,
						vkResetFences(vob.VKL, 1, &qsync.vk_fence) ); } }

		if(fps_report == current_sec || frame_index < 8) {
			loglevel = 3;
			auto finish_frame	= std::chrono::high_resolution_clock::now();
			std::string ftime 	= std::to_string(
				std::chrono::duration_cast<std::chrono::nanoseconds>(finish_frame-start_frame).count()) + " ns, "
								+ std::to_string(
				int(1000000000.0 / std::chrono::duration_cast<std::chrono::nanoseconds>(finish_frame-start_frame).count())) + " FPS, i"
				+  std::to_string(frame_index);
			iv("Frame", ftime, frame_index);
			if(frame_index >= 8) { loglevel = -1; } } else { if(frame_index >= 8) { loglevel = -1; } }
		if(fps_report == current_sec) { fps_report--; }

		frame_index++;

	} while (valid);

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "EXIT APPLICATION");		/**/
	///////////////////////////////////////////////////

	if(!valid) { hd("STAGE:", "ABORTED"); }

	rv("return");
	return 0;
}
