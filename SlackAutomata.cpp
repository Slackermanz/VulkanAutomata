#include "src/slack_engine.h"

int main() {

//	Create Application Context
	VK_Context vk_ctx;
		engine_vulkan_context_init( &vk_ctx, "SlackAutomata" );

//	Find and select Physical Device
	VK_PhysicalDevice vk_pdv[vk_ctx.pd_count];
		engine_vulkan_get_physical_device( &vk_ctx, vk_pdv );
		show_pdv_info( &vk_ctx, vk_pdv );

//	Identify available queue families
	VK_QueueFamily vk_qfp[vk_pdv[vk_ctx.pd_index].qf_count];
		engine_vulkan_get_queue_families( &vk_pdv[vk_ctx.pd_index], vk_qfp );
		show_qfp_info( &vk_pdv[vk_ctx.pd_index], vk_qfp );

//	Identify useful Queue Family Index(s)
	uint32_t QFI_graphic = UINT32_MAX;
		engine_vulkan_get_queue_family_index( &vk_pdv[vk_ctx.pd_index], vk_qfp, &QFI_graphic, VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT );
		engine_vulkan_get_queue_family_index( &vk_pdv[vk_ctx.pd_index], vk_qfp, &QFI_graphic, VK_QUEUE_GRAPHICS_BIT );

	uint32_t QFI_compute = UINT32_MAX;
		engine_vulkan_get_queue_family_index( &vk_pdv[vk_ctx.pd_index], vk_qfp, &QFI_compute, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT );
		engine_vulkan_get_queue_family_index( &vk_pdv[vk_ctx.pd_index], vk_qfp, &QFI_compute, VK_QUEUE_COMPUTE_BIT );

	uint32_t QFI_combine = UINT32_MAX;
		engine_vulkan_get_queue_family_index( &vk_pdv[vk_ctx.pd_index], vk_qfp, &QFI_combine, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT );

//	Number of queues to create on the Logical Device
	uint32_t DEV_QUEUE_COUNT = 1;

//	Device Queue configuration(s)
	VK_DeviceQueueInfo vk_device_queue_info[DEV_QUEUE_COUNT];
		vk_device_queue_info[0].queueFamilyIndex 	= QFI_combine;
		vk_device_queue_info[0].queueCount 			= vk_qfp[vk_device_queue_info[0].queueFamilyIndex].qf_props.queueCount;

//		vk_device_queue_info[1].queueFamilyIndex 	= QFI_compute;
//		vk_device_queue_info[1].queueCount 			= vk_qfp[vk_device_queue_info[1].queueFamilyIndex].qf_props.queueCount;

//	Array data for pQueuePriorities (to the maximum of 16 queue entries)
	float qp[16] = { 0.0f };

//	Device Queue creation info(s)
	VkDeviceQueueCreateInfo vk_device_queue_create_info[DEV_QUEUE_COUNT];
	for( int i = 0; i < DEV_QUEUE_COUNT; i++ ) {
		vk_device_queue_create_info[i].sType				= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		vk_device_queue_create_info[i].pNext				= nullptr;
		vk_device_queue_create_info[i].flags				= 0;
		vk_device_queue_create_info[i].queueFamilyIndex		= vk_device_queue_info[i].queueFamilyIndex;
		vk_device_queue_create_info[i].queueCount			= vk_device_queue_info[i].queueCount;
		vk_device_queue_create_info[i].pQueuePriorities		= qp; }

//	Create the Logical Device
	VK_LogicalDevice vk_ldv;
		engine_vulkan_logical_device_init( &vk_pdv[vk_ctx.pd_index], DEV_QUEUE_COUNT, vk_device_queue_create_info, &vk_ldv );

//	Create Command Pool(s) for the Device Queues
	VK_CommandPool vk_cpl[DEV_QUEUE_COUNT];
		engine_vulkan_command_pool_init( &vk_ldv, vk_device_queue_info, vk_cpl );

//	Do something
//	...

//	Cleanup and exit
	ov( "EXIT" );
		engine_vulkan_command_pool_exit( &vk_ldv, vk_cpl );
		engine_vulkan_logical_device_exit( &vk_ldv );
		engine_vulkan_context_exit( &vk_ctx );

	return 0;
}

