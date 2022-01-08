#include "slack_engine.h"

void engine::show_physical_devices( svk::VK_Context *vk_context, svk::VK_PhysicalDevice *vk_physical_device ) {
	for(int i = 0; i < vk_context->pd_count; i++) {
		ov("Physical Device " + std::to_string(i) + ":");
		ov("apiVersion", 		vk_physical_device[i].pd_props.apiVersion		);
		ov("driverVersion", 	vk_physical_device[i].pd_props.driverVersion	);
		ov("deviceType", 		vk_physical_device[i].pd_props.deviceType		);
		av("deviceName", 		vk_physical_device[i].pd_props.deviceName		);
		ov("memoryTypeCount", 	vk_physical_device[i].pd_memos.memoryTypeCount	);
		ov("memoryHeapCount", 	vk_physical_device[i].pd_memos.memoryHeapCount	);
		ov("QFP Count", 		vk_physical_device[i].qf_count					); } }

void engine::show_queue_families( svk::VK_PhysicalDevice *vk_physical_device, svk::VK_QueueFamily *vk_queue_families ) {
	for(int i = 0; i < vk_physical_device->qf_count; i++) {
		ov("Queue Family " + std::to_string(i) + ":");
		ov("queueFlags", vk_queue_families[i].qf_props.queueFlags);
		ov("queueCount", vk_queue_families[i].qf_props.queueCount); } }

void engine::init_context( svk::VK_Context *vk_context, const char *app_name ) {

	ov( "Define Vulkan Extensions" );
		const uint32_t 	VK_LAYER_EXTN_COUNT 						= 1;
		const char* 	VK_LAYER_EXTNS[VK_LAYER_EXTN_COUNT] 		= {	"VK_LAYER_KHRONOS_validation" };
		const uint32_t 	VK_INSTANCE_EXTN_COUNT 						= 1;
		const char* 	VK_INSTANCE_EXTNS[VK_INSTANCE_EXTN_COUNT] 	= {	VK_EXT_DEBUG_UTILS_EXTENSION_NAME };

	ov( "Init Struct: svk::VK_Config" );
		svk::VK_Config vk_config = svk::new_vk_config(
			app_name,
			VK_LAYER_EXTN_COUNT, 	VK_LAYER_EXTNS,
			VK_INSTANCE_EXTN_COUNT, VK_INSTANCE_EXTNS );

	ov( "Create Vulkan Instance" );
		svk::create_instance( &vk_config, vk_context );

	ov( "Init Struct: svk::VK_DebugUtils" );
		vk_context->vk_dbutl = svk::new_vk_debug_utils();

	ov( "Create Vulkan Debug Utils" );
		svk::create_debug_utils( vk_context );

	ov( "Vulkan Physical Device count" );
		svk::count_physical_devices( vk_context ); }

void engine::exit_context( svk::VK_Context *vk_context ) {
	ov( "Destroy Vulkan Context" );
		svk::destroy_debug_utils	( vk_context );
		svk::destroy_instance	( vk_context ); }

void engine::find_physical_devices( svk::VK_Context *vk_context, svk::VK_PhysicalDevice *vk_physical_devices ) {
	ov( "Fill Struct: svk::VK_PhysicalDevice" );
		svk::enum_physical_devices( vk_physical_devices, vk_context );

	ov( "Vulkan Physical Device selection" );
		svk::find_physical_device( vk_physical_devices, vk_context ); }

void engine::find_queue_families( svk::VK_PhysicalDevice *vk_physical_device, svk::VK_QueueFamily *vk_queue_families ) {
	ov( "Fill Struct: svk::VK_QueueFamily" );
		svk::enum_queue_families( vk_queue_families, vk_physical_device ); }

void engine::find_queue_family_index(
	svk::VK_PhysicalDevice 	*vk_physical_device,
	svk::VK_QueueFamily 		*vk_queue_families,
	uint32_t			*qf_index,
	uint32_t			queue_flag_bits,
	uint32_t			queue_flag_nots ) {
	if( *qf_index == UINT32_MAX ) {
		ov( "Find Queue Family Index" );
			ov("Include bits", queue_flag_bits);
			ov("Exclude bits", queue_flag_nots); }
	for(int i = 0; i < vk_physical_device->qf_count; i++) {
		if( *qf_index													== UINT32_MAX
		&& (vk_queue_families[i].qf_props.queueFlags & queue_flag_bits) == queue_flag_bits
		&& (vk_queue_families[i].qf_props.queueFlags & queue_flag_nots) == 0 ) {
			*qf_index = i;
			ov("Selected Index", *qf_index); } }
	if( *qf_index == UINT32_MAX ) { ov("Selected Index", "NOT FOUND"); } }

void engine::init_logical_device(
	svk::VK_PhysicalDevice 		*vk_physical_device,
	uint32_t				dev_queue_count,
	VkDeviceQueueCreateInfo	*vk_device_queue_info,
	svk::VK_LogicalDevice 		*vk_logical_device ) {

	ov( "Init Vulkan Logical Device" );
		*vk_logical_device = svk::new_vk_logical_device( dev_queue_count, vk_device_queue_info, 0, nullptr, nullptr );

	ov( "Create Vulkan Logical Device" );
		svk::create_logical_device( vk_physical_device, vk_logical_device ); }

void engine::exit_logical_device( svk::VK_LogicalDevice *vk_logical_device ) {
	ov( "Destroy Vulkan Logical Device" );
		svk::destroy_logical_device	( vk_logical_device	); }

void engine::init_command_pool(
	svk::VK_LogicalDevice 	*vk_logical_device,
	svk::VK_DeviceQueueInfo	*vk_device_queue_info,
	svk::VK_CommandPool		*vk_command_pool ) {
	ov( "Create Command Pools" );
	for(int i = 0; i < vk_logical_device->ld_info.queueCreateInfoCount; i++) {
		vk_command_pool[i] = svk::new_vk_command_pool( vk_device_queue_info[i].queueFamilyIndex );
		svk::create_command_pool( vk_logical_device, &vk_command_pool[i] ); } }

void engine::exit_command_pool(
	svk::VK_LogicalDevice 	*vk_logical_device,
	svk::VK_CommandPool 		*vk_command_pool ) {
	ov( "Destroy Vulkan Command Pool(s)" );
	for(int i = 0; i < vk_logical_device->ld_info.queueCreateInfoCount; i++) {
		svk::destroy_command_pool( vk_logical_device, &vk_command_pool[i] ); } }




