#include "src/slack_engine.h"
#include <vector>

int main() {

//	Create Application Context
	svk::VK_Context vk_ctx;
		engine::init_context( &vk_ctx, "SlackAutomata" );

//	Find and select Physical Device
	std::vector<svk::VK_PhysicalDevice> vk_pdv(vk_ctx.pd_count);
		engine::find_physical_devices( &vk_ctx, vk_pdv.data() );
		engine::show_physical_devices( &vk_ctx, vk_pdv.data() );

//	Identify available queue families
	std::vector<svk::VK_QueueFamily> vk_qfp(vk_pdv[vk_ctx.pd_index].qf_count);
		engine::find_queue_families( &vk_pdv[vk_ctx.pd_index], vk_qfp.data() );
		engine::show_queue_families( &vk_pdv[vk_ctx.pd_index], vk_qfp.data() );

//	Identify useful Queue Family Index(s)
	uint32_t QFI_graphic = UINT32_MAX;
		engine::find_queue_family_index( &vk_pdv[vk_ctx.pd_index], vk_qfp.data(), &QFI_graphic, VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT );
		engine::find_queue_family_index( &vk_pdv[vk_ctx.pd_index], vk_qfp.data(), &QFI_graphic, VK_QUEUE_GRAPHICS_BIT );

	uint32_t QFI_compute = UINT32_MAX;
		engine::find_queue_family_index( &vk_pdv[vk_ctx.pd_index], vk_qfp.data(), &QFI_compute, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT );
		engine::find_queue_family_index( &vk_pdv[vk_ctx.pd_index], vk_qfp.data(), &QFI_compute, VK_QUEUE_COMPUTE_BIT );

	uint32_t QFI_combine = UINT32_MAX;
		engine::find_queue_family_index( &vk_pdv[vk_ctx.pd_index], vk_qfp.data(), &QFI_combine, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT );

//	Number of queues to create on the Logical Device
	constexpr uint32_t DEV_QUEUE_COUNT = 1;

//	Device Queue configuration(s)
	svk::VK_DeviceQueueInfo vk_device_queue_info[DEV_QUEUE_COUNT];
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
	svk::VK_LogicalDevice vk_ldv;
		engine::init_logical_device( &vk_pdv[vk_ctx.pd_index], DEV_QUEUE_COUNT, vk_device_queue_create_info, &vk_ldv );

	ov( "CONTEXT ESTABLISHED" );

//	Create Command Pool(s) for the Device Queues
	svk::VK_CommandPool vk_cpl[DEV_QUEUE_COUNT];
		engine::init_command_pool( &vk_ldv, vk_device_queue_info, vk_cpl );

//	Add a Shader Module
	svk::VK_Shader test_shader;
#ifdef _WIN32 //todo: abstract away such differences in a filesystem abstraction (maybe std::filesystem)
	engine::init_shader_module(&vk_ldv, &test_shader, "../noop.spv");
#else
	engine::init_shader_module(&vk_ldv, &test_shader, "./noop.spv");
#endif
		engine::show_shader_module( &test_shader );

//	Cleanup and exit
	ov( "EXIT" );
		engine::exit_shader_module( &vk_ldv, &test_shader );
		engine::exit_command_pool( &vk_ldv, vk_cpl );
		engine::exit_logical_device( &vk_ldv );
		engine::exit_context( &vk_ctx );

	return 0;
}

