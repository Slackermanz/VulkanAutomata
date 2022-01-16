#include "slack_vulkan.h"
#include <vector>

static VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_utils_callback(
			VkDebugUtilsMessageSeverityFlagBitsEXT	messageSeverity, 
			VkDebugUtilsMessageTypeFlagsEXT			messageType, 
	const 	VkDebugUtilsMessengerCallbackDataEXT* 	pCallbackData, 
	void* 	pUserData ) {
	std::string msg = pCallbackData->pMessage;
		msg = str_inplace( msg, ": [", 	":\n[" 	);
		msg = str_inplace( msg, " ] ", 	" ]\n" 	);
		msg = str_inplace( msg, " | ", 	"\n  " 	);
		msg = str_inplace( msg, "; ", 	";\n" 	);
		msg = str_inplace( msg, ": ", 	":\n" 	);
		msg = str_inplace( msg, ". ", 	".\n" 	);
	ov( msg );
	return VK_FALSE; }

	  /////////////////////////////
	 //	INITS
	/////////////////////////////

svk::VK_DebugUtils svk::new_vk_debug_utils() {
	VK_DebugUtils vk_debug_utils;
		vk_debug_utils.vk_debug_utils_info.sType 			= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		vk_debug_utils.vk_debug_utils_info.pNext 			= nullptr;
		vk_debug_utils.vk_debug_utils_info.flags 			= 0;
		vk_debug_utils.vk_debug_utils_info.messageSeverity 	= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
															| VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT 
															| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT 
															| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		vk_debug_utils.vk_debug_utils_info.messageType 		= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
															| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		vk_debug_utils.vk_debug_utils_info.pfnUserCallback 	= vk_debug_utils_callback;
		vk_debug_utils.vk_debug_utils_info.pUserData 		= nullptr;
	return vk_debug_utils; }

svk::VK_Config svk::new_vk_config(
	const char* 		appname,
	uint32_t			lay_cnt,
	const char* const* 	lay_ext,
	uint32_t			ins_cnt,
	const char* const* 	ins_ext	) {
	VK_Config vk_config;
		vk_config.app_info.sType						= VK_STRUCTURE_TYPE_APPLICATION_INFO;
		vk_config.app_info.pNext						= nullptr;
		vk_config.app_info.pApplicationName				= appname;
		vk_config.app_info.applicationVersion			= 0;
		vk_config.app_info.pEngineName					= nullptr;
		vk_config.app_info.engineVersion				= 0;
		vk_config.app_info.apiVersion					= VK_API_VERSION_1_2;
		vk_config.inst_info.sType 						= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		vk_config.inst_info.pNext 						= nullptr;
		vk_config.inst_info.flags 						= 0;
		vk_config.inst_info.pApplicationInfo 			= &vk_config.app_info;
		vk_config.inst_info.enabledLayerCount 			= lay_cnt;
		vk_config.inst_info.ppEnabledLayerNames 		= lay_ext;
		vk_config.inst_info.enabledExtensionCount 		= ins_cnt;
		vk_config.inst_info.ppEnabledExtensionNames		= ins_ext;
	return vk_config; }

svk::VK_LogicalDevice svk::new_vk_logical_device(
	uint32_t						queueCreateInfoCount,
	const VkDeviceQueueCreateInfo*	pQueueCreateInfos,
	uint32_t						enabledExtensionCount,
	const char* const*				ppEnabledExtensionNames,
	const VkPhysicalDeviceFeatures* pEnabledFeatures ) {
	VK_LogicalDevice vk_logical_device;
		vk_logical_device.ld_info.sType 					= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		vk_logical_device.ld_info.pNext 					= nullptr;
		vk_logical_device.ld_info.flags 					= 0;
		vk_logical_device.ld_info.queueCreateInfoCount 		= queueCreateInfoCount;
		vk_logical_device.ld_info.pQueueCreateInfos 		= pQueueCreateInfos;
		vk_logical_device.ld_info.enabledLayerCount 		= 0;
		vk_logical_device.ld_info.ppEnabledLayerNames 		= nullptr;
		vk_logical_device.ld_info.enabledExtensionCount 	= enabledExtensionCount;
		vk_logical_device.ld_info.ppEnabledExtensionNames 	= ppEnabledExtensionNames;
		vk_logical_device.ld_info.pEnabledFeatures 			= pEnabledFeatures;
	return vk_logical_device; }

svk::VK_CommandPool svk::new_vk_command_pool(
	uint32_t					qf_index,
	VkCommandPoolCreateFlags	flags ) {
	VK_CommandPool vk_command_pool;
		vk_command_pool.cp_info.sType 				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		vk_command_pool.cp_info.pNext 				= nullptr;
		vk_command_pool.cp_info.flags 				= flags;
		vk_command_pool.cp_info.queueFamilyIndex 	= qf_index;
	return vk_command_pool; }

	  /////////////////////////////
	 //	FUNCTIONS
	/////////////////////////////

void svk::create_instance( VK_Config *vk_config, VK_Context *vk_context ) {
	ov("vkCreateInstance", vk_context->vi,
		vkCreateInstance( &vk_config->inst_info, nullptr, &vk_context->vi ) ); }

void svk::destroy_instance( VK_Context *vk_context ) {
	rv("vkDestroyInstance");
		vkDestroyInstance( vk_context->vi, nullptr ); }

void svk::create_debug_utils( VK_Context *vk_context ) {
	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT_pfn
	=	reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>( 
			vkGetInstanceProcAddr( vk_context->vi, "vkCreateDebugUtilsMessengerEXT" ) );
	ov("vkCreateDebugUtilsMessengerEXT_pfn", vk_context->vk_dbutl.vk_debug_utils_messenger,
		vkCreateDebugUtilsMessengerEXT_pfn(
			vk_context->vi, &vk_context->vk_dbutl.vk_debug_utils_info,
			nullptr,
			&vk_context->vk_dbutl.vk_debug_utils_messenger ) ); }

void svk::destroy_debug_utils( VK_Context *vk_context ) {
	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT_pfn
	=	reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>( 
			vkGetInstanceProcAddr( vk_context->vi, "vkDestroyDebugUtilsMessengerEXT" ) );
	rv("vkDestroyDebugUtilsMessengerEXT_pfn");
		vkDestroyDebugUtilsMessengerEXT_pfn( vk_context->vi, vk_context->vk_dbutl.vk_debug_utils_messenger, nullptr ); }

void svk::count_physical_devices( VK_Context *vk_context ) {
	ov("vkEnumeratePhysicalDevices", vk_context->pd_count,
		vkEnumeratePhysicalDevices( vk_context->vi, &vk_context->pd_count, nullptr ) ); }

void svk::enum_physical_devices( VK_PhysicalDevice *vk_physical_device, VK_Context *vk_context ) {
	std::vector<VkPhysicalDevice> vk_pd_list(vk_context->pd_count);
	ov("vkEnumeratePhysicalDevices", &vk_pd_list,
		vkEnumeratePhysicalDevices( vk_context->vi, &vk_context->pd_count, vk_pd_list.data() ) );
	for(int i = 0; i < vk_context->pd_count; i++) {
		ov("Physical Device", i);
		vk_physical_device[i].pd_index 	= i;
		vk_physical_device[i].pd 		= vk_pd_list[i];
		rv("vkGetPhysicalDeviceProperties", i);
			vkGetPhysicalDeviceProperties				( vk_physical_device[i].pd, &vk_physical_device[i].pd_props );
		rv("vkGetPhysicalDeviceFeatures", i);
			vkGetPhysicalDeviceFeatures					( vk_physical_device[i].pd, &vk_physical_device[i].pd_feats );
		rv("vkGetPhysicalDeviceMemoryProperties", i);
			vkGetPhysicalDeviceMemoryProperties			( vk_physical_device[i].pd, &vk_physical_device[i].pd_memos );
		rv("vkGetPhysicalDeviceQueueFamilyProperties", i);
			vkGetPhysicalDeviceQueueFamilyProperties	( vk_physical_device[i].pd, &vk_physical_device[i].qf_count, nullptr ); } }

void svk::find_physical_device( VK_PhysicalDevice *vk_physical_device, VK_Context *vk_context ) {
				vk_context->pd_index 	= UINT32_MAX;
	uint32_t 	pd_type_list[5] 		= { 2, 1, 3, 4, 0 };
	for(int i = 0; i < 5; i++) {
		for(int j = 0; j < vk_context->pd_count; j++) {
			if( vk_context->pd_index 						== UINT32_MAX
			&&	vk_physical_device[j].pd_props.deviceType 	== pd_type_list[i] ) { vk_context->pd_index = j; } }
		if( vk_context->pd_index == UINT32_MAX ) {
			ov("No devices found of type", pd_type_list[i]); } }
	if( vk_context->pd_index != UINT32_MAX ) {
		ov("Type", 	vk_physical_device[vk_context->pd_index].pd_props.deviceType );
		ov("Index", vk_context->pd_index 										 ); } }

void svk::enum_queue_families( VK_QueueFamily *vk_queue_family, VK_PhysicalDevice *vk_physical_device ) {
	std::vector<VkQueueFamilyProperties> vk_qf_list(vk_physical_device->qf_count);
	rv("vkGetPhysicalDeviceQueueFamilyProperties");
		vkGetPhysicalDeviceQueueFamilyProperties( vk_physical_device->pd, &vk_physical_device->qf_count, vk_qf_list.data() );
	for(int i = 0; i < vk_physical_device->qf_count; i++) {
		vk_queue_family[i].qf_index = i;
		vk_queue_family[i].qf_props	= vk_qf_list[i]; } }

void svk::create_logical_device( VK_PhysicalDevice *vk_physical_device, VK_LogicalDevice *vk_logical_device ) {
	ov("vkCreateDevice", vk_logical_device->ld,
		vkCreateDevice( vk_physical_device->pd, &vk_logical_device->ld_info, nullptr, &vk_logical_device->ld ) ); }

void svk::destroy_logical_device( VK_LogicalDevice *vk_logical_device ) {
	rv("vkDestroyDevice");
		vkDestroyDevice( vk_logical_device->ld, nullptr ); }

void svk::create_command_pool( VK_LogicalDevice *vk_logical_device, VK_CommandPool *vk_command_pool ) {
	ov("vkCreateCommandPool", vk_command_pool->cp,
		vkCreateCommandPool( vk_logical_device->ld, &vk_command_pool->cp_info, nullptr, &vk_command_pool->cp ) ); }

void svk::destroy_command_pool( VK_LogicalDevice *vk_logical_device, VK_CommandPool *vk_command_pool ) {
	rv("vkDestroyCommandPool");
		vkDestroyCommandPool( vk_logical_device->ld, vk_command_pool->cp, nullptr ); }

void getShaderCodeInfo( svk::VK_Shader *vk_shader ) {
	std::ifstream 		file		(vk_shader->shaderFilename, std::ios::ate | std::ios::binary);
	size_t 				fileSize = 	(size_t) file.tellg();
	vk_shader->shaderData.resize(fileSize);
	file.seekg(0);
	file.read(vk_shader->shaderData.data(), fileSize);
	file.close();

	vk_shader->shaderFilename		= vk_shader->shaderFilename;
	vk_shader->shaderBytesValid		= vk_shader->shaderData.size() % 4 == 0; }

void svk::create_shader_module( VK_LogicalDevice *vk_logical_device, VK_Shader *vk_shader ) {
	getShaderCodeInfo( vk_shader );

	VkShaderModuleCreateInfo module_info{};

	module_info.sType		= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	module_info.pNext		= nullptr;
	module_info.flags		= 0;
	module_info.codeSize		= vk_shader->shaderData.size();
	module_info.pCode		= reinterpret_cast<const uint32_t*>(vk_shader->shaderData.data());
	ov("vkCreateShaderModule", vk_shader->vk_shader_module,
		vkCreateShaderModule( vk_logical_device->ld, &module_info, nullptr, &vk_shader->vk_shader_module ) ); }

void svk::destroy_shader_module( VK_LogicalDevice *vk_logical_device, VK_Shader *vk_shader ) {
	rv("vkDestroyShaderModule");
		vkDestroyShaderModule( vk_logical_device->ld, vk_shader->vk_shader_module, nullptr ); }

VkDescriptorSetLayoutBinding svk::new_vk_dslb(
	VK_Shader			*vk_shader,
	VkDescriptorType	descriptorType,
	uint32_t			descriptorCount ) {
	VkDescriptorSetLayoutBinding vk_dslb;
		vk_dslb.binding 			= vk_shader->vk_dslb.size();
		vk_dslb.descriptorType 		= descriptorType;
		vk_dslb.descriptorCount 	= descriptorCount;
		vk_dslb.stageFlags 			= vk_shader->vk_shader_stage;
		vk_dslb.pImmutableSamplers 	= nullptr;
	return vk_dslb; }

void svk::create_descriptor_set_layout( VK_LogicalDevice *vk_logical_device, VK_Shader *vk_shader ) {
	VkDescriptorSetLayoutCreateInfo vk_dsl_info;
		vk_dsl_info.sType 			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		vk_dsl_info.pNext 			= nullptr;
		vk_dsl_info.flags 			= 0;
		vk_dsl_info.bindingCount 	= vk_shader->vk_dslb.size();
		vk_dsl_info.pBindings 		= vk_shader->vk_dslb.data();
	ov("vkCreateDescriptorSetLayout", vk_shader->vk_dsl,
		vkCreateDescriptorSetLayout( vk_logical_device->ld, &vk_dsl_info, nullptr, &vk_shader->vk_dsl ) ); }

void svk::destroy_descriptor_set_layout( VK_LogicalDevice *vk_logical_device, VK_Shader *vk_shader ) {
	rv("vkDestroyDescriptorSetLayout");
		vkDestroyDescriptorSetLayout( vk_logical_device->ld, vk_shader->vk_dsl, nullptr ); }

void svk::create_descriptor_pool( VK_LogicalDevice *vk_logical_device, VK_Shader *vk_shader ) {
	for( int i = 0; i < vk_shader->vk_dslb.size(); i++ ) {
		VkDescriptorPoolSize vk_dps;
			vk_dps.type 			= vk_shader->vk_dslb.at(i).descriptorType;
			vk_dps.descriptorCount 	= vk_shader->vk_dslb.at(i).descriptorCount;
		vk_shader->vk_dps.push_back( vk_dps ); }
	VkDescriptorPoolCreateInfo vk_dsp_info;
		vk_dsp_info.sType 			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		vk_dsp_info.pNext 			= nullptr;
		vk_dsp_info.flags 			= 0;
		vk_dsp_info.maxSets 		= 1; //TODO Might need to be 2 for ping-pong shaders
		vk_dsp_info.poolSizeCount	= vk_shader->vk_dps.size();
		vk_dsp_info.pPoolSizes		= vk_shader->vk_dps.data();
	ov("vkCreateDescriptorPool", vk_shader->vk_dsp,
		vkCreateDescriptorPool( vk_logical_device->ld, &vk_dsp_info, nullptr, &vk_shader->vk_dsp ) ); }

void svk::destroy_descriptor_pool( VK_LogicalDevice *vk_logical_device, VK_Shader *vk_shader ) {
	rv("vkDestroyDescriptorPool");
		vkDestroyDescriptorPool( vk_logical_device->ld, vk_shader->vk_dsp, nullptr ); }

void svk::allocate_descriptor_set( VK_LogicalDevice *vk_logical_device, VK_Shader *vk_shader ) {
	VkDescriptorSetAllocateInfo vk_ds_info;
		vk_ds_info.sType 				= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		vk_ds_info.pNext 				= nullptr;
		vk_ds_info.descriptorPool 		= vk_shader->vk_dsp;
		vk_ds_info.descriptorSetCount 	= 1;
		vk_ds_info.pSetLayouts 			= &vk_shader->vk_dsl;
	ov("vkAllocateDescriptorSets", vk_shader->vk_descriptor_set,
		vkAllocateDescriptorSets( vk_logical_device->ld, &vk_ds_info, &vk_shader->vk_descriptor_set ) ); }







