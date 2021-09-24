#ifndef SLACK_ENGINE_H
#define SLACK_ENGINE_H

	#include "slack_vulkan.h"

	VK_Context 			engine_vulkan_init( const char *app_name = "EngineName" );
	void 				show_pd_info( VK_Context *vk_context, VK_PhysicalDevice *vk_physical_devices, uint32_t idx = UINT32_MAX );
	VK_PhysicalDevice 	engine_vulkan_get_physical_device( VK_Context *vk_context );
	void 				engine_vulkan_close_context( VK_Context *vk_context );

#endif
