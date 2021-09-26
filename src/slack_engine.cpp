#include "slack_engine.h"

void show_pd_info( VK_PhysicalDevice *vk_physical_device, uint32_t idx ) {
	idx = (idx == UINT32_MAX) ? vk_physical_device->pd_index : idx;
	ov("Physical Device " + std::to_string(idx) + ":");
	ov("apiVersion", 		vk_physical_device->pd_props.apiVersion			);
	ov("driverVersion", 	vk_physical_device->pd_props.driverVersion		);
	ov("deviceType", 		vk_physical_device->pd_props.deviceType			);
	av("deviceName", 		vk_physical_device->pd_props.deviceName			);
	ov("memoryTypeCount", 	vk_physical_device->pd_memos.memoryTypeCount	);
	ov("memoryHeapCount", 	vk_physical_device->pd_memos.memoryHeapCount	);
	ov("QFP Count", 		vk_physical_device->qf_count					); }

void show_qf_info( VK_PhysicalDevice *vk_physical_device, VK_QueueFamily *vk_queue_families ) {
	for(int i = 0; i < vk_physical_device->qf_count; i++) {
		ov("Queue Family " + std::to_string(i) + ":");
		ov("queueFlags", vk_queue_families[i].qf_props.queueFlags);
		ov("queueCount", vk_queue_families[i].qf_props.queueCount); } }

void engine_vulkan_context_init( VK_Context *vk_context, const char *app_name ) {
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

	ov( "Create Vulkan Instance" );
		svk_create_instance( &vk_config, vk_context );

	ov( "Init Struct: VK_DebugUtils" );
		vk_context->vk_dbutl = new_vk_debug_utils();

	ov( "Create Vulkan Debug Utils" );
		svk_create_debug_utils( vk_context );

	ov( "Vulkan Physical Device count" );
		svk_count_physical_devices( vk_context ); }

void engine_vulkan_context_exit( VK_Context *vk_context ) {
	ov( "Destroy Vulkan Context" );
		svk_destroy_debug_utils	( vk_context );
		svk_destroy_instance	( vk_context ); }

void engine_vulkan_get_physical_device( VK_Context *vk_context, VK_PhysicalDevice *vk_physical_devices ) {
	ov( "Fill Struct: VK_PhysicalDevice" );
		svk_enum_physical_devices( vk_physical_devices, vk_context );

	for(int i = 0; i < vk_context->pd_count; i++) { show_pd_info( &vk_physical_devices[i], i ); }

	ov( "Vulkan Physical Device selection" );
		svk_find_physical_device( vk_physical_devices, vk_context ); }

void engine_vulkan_get_queue_families( VK_PhysicalDevice *vk_physical_device, VK_QueueFamily *vk_queue_families ) {
	ov( "Fill Struct: VK_QueueFamily" );
		svk_enum_queue_families( vk_queue_families, vk_physical_device ); }


