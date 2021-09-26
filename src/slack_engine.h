#ifndef SLACK_ENGINE_H
#define SLACK_ENGINE_H

	#include "slack_vulkan.h"

	void 				show_pd_info( VK_PhysicalDevice *vk_physical_device, uint32_t idx = UINT32_MAX );
	void 				show_qf_info( VK_PhysicalDevice *vk_physical_device, VK_QueueFamily *vk_queue_families );
	void 				engine_vulkan_context_init( VK_Context *vk_context, const char *app_name = "EngineName" );
	void 				engine_vulkan_context_exit( VK_Context *vk_context );
	void 				engine_vulkan_get_physical_device( VK_Context *vk_context, VK_PhysicalDevice *vk_physical_devices );
	void 				engine_vulkan_get_queue_families( VK_PhysicalDevice *vk_physical_device, VK_QueueFamily *vk_queue_families );

#endif
