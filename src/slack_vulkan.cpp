#include "slack_vulkan.h"

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

VK_DebugUtils new_vk_debug_utils() {
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

VK_Context new_vk_context() {
	VK_Context vk_context;
		vk_context.vi 		= nullptr;
		vk_context.vk_dbutl	= new_vk_debug_utils();
		vk_context.pd_count = 0;
		vk_context.pd_index = UINT32_MAX;
		vk_context.pd 		= nullptr;
	return vk_context; }

VK_Config new_vk_config(
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

void svk_create_instance( VK_Config *vk_config, VK_Context *vk_context ) {
	ov("vkCreateInstance", vk_context->vi,
		vkCreateInstance( &vk_config->inst_info, nullptr, &vk_context->vi ) ); }

void svk_destroy_instance( VK_Context *vk_context ) {
	rv("vkDestroyInstance");
		vkDestroyInstance( vk_context->vi, nullptr ); }

void svk_create_debug_utils( VK_Context *vk_context ) {
	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT_pfn
	=	reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>( 
			vkGetInstanceProcAddr( vk_context->vi, "vkCreateDebugUtilsMessengerEXT" ) );
	ov("vkCreateDebugUtilsMessengerEXT_pfn", vk_context->vk_dbutl.vk_debug_utils_messenger,
		vkCreateDebugUtilsMessengerEXT_pfn(
			vk_context->vi, &vk_context->vk_dbutl.vk_debug_utils_info,
			nullptr,
			&vk_context->vk_dbutl.vk_debug_utils_messenger ) ); }

void svk_destroy_debug_utils( VK_Context *vk_context ) {
	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT_pfn
	=	reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>( 
			vkGetInstanceProcAddr( vk_context->vi, "vkDestroyDebugUtilsMessengerEXT" ) );
	rv("vkDestroyDebugUtilsMessengerEXT_pfn");
		vkDestroyDebugUtilsMessengerEXT_pfn( vk_context->vi, vk_context->vk_dbutl.vk_debug_utils_messenger, nullptr ); }

void svk_count_physical_devices( VK_Context *vk_context ) {
	ov("vkEnumeratePhysicalDevices", vk_context->pd_count,
		vkEnumeratePhysicalDevices( vk_context->vi, &vk_context->pd_count, nullptr ) ); }

void svk_enum_physical_devices( VK_PhysicalDevice *vk_physical_device, VK_Context *vk_context ) {
	VkPhysicalDevice vk_pd_list[vk_context->pd_count];
	ov("vkEnumeratePhysicalDevices", &vk_pd_list,
		vkEnumeratePhysicalDevices( vk_context->vi, &vk_context->pd_count, vk_pd_list ) );
	for(int i = 0; i < vk_context->pd_count; i++) {
		vk_physical_device[i].pd_index 	= i;
		vk_physical_device[i].pd 		= vk_pd_list[i];
		rv("vkGetPhysicalDeviceProperties", i);
			vkGetPhysicalDeviceProperties		( vk_physical_device[i].pd, &vk_physical_device[i].pd_props );
		rv("vkGetPhysicalDeviceFeatures", i);
			vkGetPhysicalDeviceFeatures			( vk_physical_device[i].pd, &vk_physical_device[i].pd_feats );
		rv("vkGetPhysicalDeviceMemoryProperties", i);
			vkGetPhysicalDeviceMemoryProperties	( vk_physical_device[i].pd, &vk_physical_device[i].pd_memos ); } }

void svk_find_physical_device( VK_PhysicalDevice *vk_physical_device, VK_Context *vk_context ) {
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


