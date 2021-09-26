#ifndef SLACK_ENGINE_H
#define SLACK_ENGINE_H

	#include "slack_vulkan.h"

	void show_pd_info(
		VK_PhysicalDevice *vk_physical_device,
		uint32_t idx = UINT32_MAX );

	void show_qf_info(
		VK_PhysicalDevice *vk_physical_device,
		VK_QueueFamily *vk_queue_families );

	void engine_vulkan_context_init(
		VK_Context *vk_context,
		const char *app_name = "EngineName" );

	void engine_vulkan_context_exit(
		VK_Context *vk_context,
		VK_LogicalDevice *vk_logical_device );

	void engine_vulkan_get_physical_device(
		VK_Context *vk_context,
		VK_PhysicalDevice *vk_physical_devices );

	void engine_vulkan_get_queue_families(
		VK_PhysicalDevice *vk_physical_device,
		VK_QueueFamily *vk_queue_families );

	void engine_vulkan_queue_list_add(
		VK_PhysicalDevice 	*vk_physical_device,
		VK_QueueFamily 		*vk_queue_families,
		VK_QueueList		*vk_queue_list,
		uint32_t			queue_max_threads,
		uint32_t			queue_flag_bits,
		uint32_t			queue_flag_nots = 0 );

	void engine_vulkan_queue_list_count(
		const uint32_t 	 QTC,
		VK_QueueList	*vk_queue_list,
		uint32_t 		*queue_list_count,
		bool 			*use_queue );

	void engine_vulkan_queue_list_select(
		const uint32_t 	 QTC,
		VK_QueueList	*vk_queue_type_list,
		bool 			*use_queue,
		VK_QueueList	*vk_queue_list );

	void engine_vulkan_logical_device_init(
		VK_PhysicalDevice 		*vk_physical_device,
		uint32_t				 vk_queue_list_count,
		VK_QueueList			*vk_queue_list,
		VK_LogicalDevice 		*vk_logical_device );

#endif
