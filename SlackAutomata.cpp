#include "src/slack_engine.h"

int main() {

	VK_Context 			vk_ctx 	= engine_vulkan_init( "SlackAutomata" );
	VK_PhysicalDevice 	vk_pdv 	= engine_vulkan_get_physical_device( &vk_ctx );

	show_pd_info( &vk_ctx, &vk_pdv );

	engine_vulkan_close_context( &vk_ctx );

	ov( "EXIT" );

	return 0;
}
