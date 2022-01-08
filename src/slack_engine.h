#ifndef SLACK_ENGINE_H
#define SLACK_ENGINE_H

	#include "slack_vulkan.h"

	void engine_vulkan_context_init(
		VK_Context 				*vk_context,
		const char 				*app_name = "EngineName" );

	void engine_vulkan_context_exit(
		VK_Context 				*vk_context );

	void engine_vulkan_get_physical_device(
		VK_Context 				*vk_context,
		VK_PhysicalDevice 		*vk_physical_devices );

	void show_pdv_info(
		VK_Context 				*vk_context,
		VK_PhysicalDevice  		*vk_physical_device );

	void engine_vulkan_get_queue_families(
		VK_PhysicalDevice 		*vk_physical_device,
		VK_QueueFamily 			*vk_queue_families );

	void show_qfp_info(
		VK_PhysicalDevice 		*vk_physical_device,
		VK_QueueFamily 			*vk_queue_families );

	void engine_vulkan_get_queue_family_index(
		VK_PhysicalDevice 		*vk_physical_device,
		VK_QueueFamily 			*vk_queue_families,
		uint32_t				*qf_index,
		uint32_t				queue_flag_bits,
		uint32_t				queue_flag_nots = 0 );

	void engine_vulkan_logical_device_init(
		VK_PhysicalDevice 		*vk_physical_device,
		uint32_t				dev_queue_count,
		VkDeviceQueueCreateInfo *vk_device_queue_info,
		VK_LogicalDevice 		*vk_logical_device );

	void engine_vulkan_logical_device_exit(
		VK_LogicalDevice 		*vk_logical_device );

	void engine_vulkan_command_pool_init(
		VK_LogicalDevice 		*vk_logical_device,
		VK_DeviceQueueInfo		*vk_device_queue_info,
		VK_CommandPool 			*vk_command_pool );

	void engine_vulkan_command_pool_exit(
		VK_LogicalDevice 		*vk_logical_device,
		VK_CommandPool 			*vk_command_pool );

#endif
