#include "slack_engine.h"

VK_Context engine_vulkan_init( const char *app_name ) {
	ov( "Define Vulkan Extensions" );
		const uint32_t 	VK_LAYER_EXTN_COUNT 					= 1;
		const char* 	VK_LAYER_EXTNS[VK_LAYER_EXTN_COUNT] 	= {	"VK_LAYER_KHRONOS_validation" };
		const uint32_t 	VK_INSTANCE_EXTN_COUNT 					= 1;
		const char* 	VK_INSTANCE_EXTNS[VK_LAYER_EXTN_COUNT] 	= {	VK_EXT_DEBUG_UTILS_EXTENSION_NAME };

	ov( "Init Struct: VK_Config" );
		VK_Config vk_config = new_vk_config(
			app_name,
			VK_LAYER_EXTN_COUNT, 	VK_LAYER_EXTNS,
			VK_INSTANCE_EXTN_COUNT, VK_INSTANCE_EXTNS );

	ov( "Init Struct: VK_Context" );
		VK_Context vk_context = new_vk_context();

	ov( "Create Vulkan Instance" );
		svk_create_instance( &vk_config, &vk_context );

	ov( "Init Struct: VK_DebugUtils" );
		vk_context.vk_dbutl = new_vk_debug_utils();

	ov( "Create Vulkan Debug Utils" );
		svk_create_debug_utils( &vk_context );

	return vk_context; }

void show_pd_info( VK_Context *vk_context, VK_PhysicalDevice *vk_physical_device, uint32_t idx ) {
	idx = (idx == UINT32_MAX) ? vk_physical_device->pd_index : idx;
	ov("Physical Device " + std::to_string(idx) + ":");
	ov("apiVersion", 		vk_physical_device->pd_props.apiVersion			);
	ov("driverVersion", 	vk_physical_device->pd_props.driverVersion		);
	ov("deviceType", 		vk_physical_device->pd_props.deviceType			);
	av("deviceName", 		vk_physical_device->pd_props.deviceName			);
	ov("memoryTypeCount", 	vk_physical_device->pd_memos.memoryTypeCount	);
	ov("memoryHeapCount", 	vk_physical_device->pd_memos.memoryHeapCount	); }

VK_PhysicalDevice engine_vulkan_get_physical_device( VK_Context *vk_context ) {
	ov( "Vulkan Physical Device count" );
		svk_count_physical_devices( vk_context );

	ov( "Init Struct: VK_PhysicalDevice" );
		VK_PhysicalDevice vk_physical_devices[vk_context->pd_count];

	ov( "Fill VK_PhysicalDevice Struct(s)" );
		svk_enum_physical_devices( vk_physical_devices, vk_context );

	for(int i = 0; i < vk_context->pd_count; i++) { show_pd_info( vk_context, &vk_physical_devices[i], i ); }

	ov( "Vulkan Physical Device selection" );
		svk_find_physical_device( vk_physical_devices, vk_context );

	return vk_physical_devices[vk_context->pd_index]; }

void engine_vulkan_close_context( VK_Context *vk_context ) {
	ov( "Destroy Vulkan Context" );
		svk_destroy_debug_utils	( vk_context );
		svk_destroy_instance	( vk_context ); }
