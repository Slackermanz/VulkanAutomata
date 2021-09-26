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

	engine_vulkan_context_exit( &vk_ctx );

	ov( "EXIT" );
	return 0;
}
