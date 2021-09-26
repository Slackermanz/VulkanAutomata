#ifndef SLACK_VULKAN_H
#define SLACK_VULKAN_H

	#include <vulkan/vulkan.h>
	#include "slack_debug.h"

	  /////////////////////////////
	 //	STRUCTS
	/////////////////////////////

	struct VK_DebugUtils {
		VkDebugUtilsMessengerCreateInfoEXT	vk_debug_utils_info;
		VkDebugUtilsMessengerEXT			vk_debug_utils_messenger; };

	struct VK_Config {
		VkApplicationInfo 		app_info;
		VkInstanceCreateInfo	inst_info; };

	struct VK_Context {
		VkInstance 			vi;
		VK_DebugUtils		vk_dbutl;
		uint32_t			pd_count;
		uint32_t			pd_index; };

	struct VK_PhysicalDevice {
		uint32_t 							 pd_index;
		VkPhysicalDevice 					 pd;
		VkPhysicalDeviceProperties			 pd_props;
		VkPhysicalDeviceFeatures			 pd_feats;
		VkPhysicalDeviceMemoryProperties	 pd_memos;
		uint32_t 							 qf_count; };

	struct VK_QueueFamily {
		uint32_t 							 qf_index;
		VkQueueFamilyProperties				 qf_props; };

	  /////////////////////////////
	 //	INITS
	/////////////////////////////

	VK_Config new_vk_config(
		const char* 		appname 	= "ApplicationName",
		uint32_t			lay_cnt		= 0,
		const char* const* 	lay_ext 	= {},
		uint32_t			ins_cnt		= 0,
		const char* const* 	ins_ext 	= {} );

	VK_DebugUtils new_vk_debug_utils();

	  /////////////////////////////
	 //	FUNCTIONS
	/////////////////////////////

	void svk_create_instance		( VK_Config *vk_config, VK_Context *vk_context );
	void svk_destroy_instance		( VK_Context *vk_context );
	void svk_create_debug_utils		( VK_Context *vk_context );
	void svk_destroy_debug_utils	( VK_Context *vk_context );
	void svk_count_physical_devices	( VK_Context *vk_context );
	void svk_enum_physical_devices	( VK_PhysicalDevice *vk_physical_device, VK_Context *vk_context );
	void svk_find_physical_device	( VK_PhysicalDevice *vk_physical_device, VK_Context *vk_context );
	void svk_enum_queue_families	( VK_QueueFamily *vk_queue_family, VK_PhysicalDevice *vk_physical_device );

#endif
