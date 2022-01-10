#ifndef SLACK_ENGINE_H
#define SLACK_ENGINE_H

	#include "slack_vulkan.h"

	namespace engine {

		void init_context(
			svk::VK_Context 			*vk_context,
			const char 					*app_name = "EngineName" );

		void exit_context(
			svk::VK_Context 			*vk_context );

		void find_physical_devices(
			svk::VK_Context 			*vk_context,
			svk::VK_PhysicalDevice 		*vk_physical_devices );

		void show_physical_devices(
			svk::VK_Context 			*vk_context,
			svk::VK_PhysicalDevice  	*vk_physical_device );

		void find_queue_families(
			svk::VK_PhysicalDevice 		*vk_physical_device,
			svk::VK_QueueFamily 		*vk_queue_families );

		void show_queue_families(
			svk::VK_PhysicalDevice 		*vk_physical_device,
			svk::VK_QueueFamily 		*vk_queue_families );

		void find_queue_family_index(
			svk::VK_PhysicalDevice 		*vk_physical_device,
			svk::VK_QueueFamily 		*vk_queue_families,
			uint32_t					*qf_index,
			uint32_t					queue_flag_bits,
			uint32_t					queue_flag_nots = 0 );

		void init_logical_device(
			svk::VK_PhysicalDevice 		*vk_physical_device,
			uint32_t					dev_queue_count,
			VkDeviceQueueCreateInfo 	*vk_device_queue_info,
			svk::VK_LogicalDevice 		*vk_logical_device );

		void exit_logical_device(
			svk::VK_LogicalDevice 		*vk_logical_device );

		void init_command_pool(
			svk::VK_LogicalDevice 		*vk_logical_device,
			svk::VK_DeviceQueueInfo		*vk_device_queue_info,
			svk::VK_CommandPool 		*vk_command_pool );

		void exit_command_pool(
			svk::VK_LogicalDevice 		*vk_logical_device,
			svk::VK_CommandPool 		*vk_command_pool );

		void init_shader_module(
			svk::VK_LogicalDevice 		*vk_logical_device,
			svk::VK_Shader 				*vk_shader,
			const char 					*shader_file );

		void show_shader_module(
			svk::VK_Shader 				*vk_shader );

		void exit_shader_module(
			svk::VK_LogicalDevice 		*vk_logical_device,
			svk::VK_Shader 				*vk_shader );

	}

#endif