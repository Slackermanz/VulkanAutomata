#include "src/slack_engine.h"

int main() {
	VK_Context vk_ctx;
		engine_vulkan_context_init( &vk_ctx, "SlackAutomata" );

	VK_PhysicalDevice vk_pdv[vk_ctx.pd_count];
		engine_vulkan_get_physical_device( &vk_ctx, vk_pdv );
		show_pd_info( &vk_pdv[vk_ctx.pd_index] );

	VK_QueueFamily vk_qfp[vk_pdv[vk_ctx.pd_index].qf_count];
		engine_vulkan_get_queue_families( &vk_pdv[vk_ctx.pd_index], vk_qfp );
		show_qf_info( &vk_pdv[vk_ctx.pd_index], vk_qfp );

	const uint32_t QUEUE_TYPE_COUNT = 3;

	VK_QueueList vk_queue_type_list[QUEUE_TYPE_COUNT];
		engine_vulkan_queue_list_add( &vk_pdv[vk_ctx.pd_index], vk_qfp, &vk_queue_type_list[0], 0, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT );
		engine_vulkan_queue_list_add( &vk_pdv[vk_ctx.pd_index], vk_qfp, &vk_queue_type_list[1], 0, VK_QUEUE_COMPUTE_BIT  );
		engine_vulkan_queue_list_add( &vk_pdv[vk_ctx.pd_index], vk_qfp, &vk_queue_type_list[2], 0, VK_QUEUE_GRAPHICS_BIT );

	uint32_t 	queue_list_count = 0;
	bool 		use_queue[QUEUE_TYPE_COUNT];

		engine_vulkan_queue_list_count( QUEUE_TYPE_COUNT, vk_queue_type_list, &queue_list_count, use_queue );

	VK_QueueList vk_queue_list[queue_list_count];
		engine_vulkan_queue_list_select( QUEUE_TYPE_COUNT, vk_queue_type_list, use_queue, vk_queue_list );

	VK_LogicalDevice vk_ldv;
		engine_vulkan_logical_device_init( &vk_pdv[vk_ctx.pd_index], queue_list_count, vk_queue_list, &vk_ldv );

	ov( "EXIT" );
		engine_vulkan_context_exit( &vk_ctx, &vk_ldv );

	return 0;
}
