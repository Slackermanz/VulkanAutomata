#include "slack_engine.h"

//	Create Vulkan application context
svk::VK_Context engine::init_context( const char *app_name ) {

	svk::VK_Context result {};

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
		svk::create_instance( &vk_config, &result);

	ov( "Init Struct: svk::VK_DebugUtils" );
		result.vk_dbutl = svk::new_vk_debug_utils();

	ov( "Create Vulkan Debug Utils" );
		svk::create_debug_utils( &result );

	ov( "Vulkan Physical Device count" );
		svk::count_physical_devices( &result ); 
	
	return result; }

//	Cestroy Vulkan application context
void engine::exit_context( svk::VK_Context *vk_context ) {
	ov( "Destroy Vulkan Context" );
		svk::destroy_debug_utils	( vk_context );
		svk::destroy_instance	( vk_context ); }

//	Query physical device availability
void engine::find_physical_devices( svk::VK_Context *vk_context, svk::VK_PhysicalDevice *vk_physical_devices ) {
	ov( "Fill Struct: svk::VK_PhysicalDevice" );
		svk::enum_physical_devices( vk_physical_devices, vk_context );

	ov( "Vulkan Physical Device selection" );
		svk::find_physical_device( vk_physical_devices, vk_context ); }

//	Read out physical device availability
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

//	Query device's Queue types
void engine::find_queue_families( svk::VK_PhysicalDevice *vk_physical_device, svk::VK_QueueFamily *vk_queue_families ) {
	ov( "Fill Struct: svk::VK_QueueFamily" );
		svk::enum_queue_families( vk_queue_families, vk_physical_device ); }

//	Read out device's Queue types
void engine::show_queue_families( svk::VK_PhysicalDevice *vk_physical_device, svk::VK_QueueFamily *vk_queue_families ) {
	for(int i = 0; i < vk_physical_device->qf_count; i++) {
		ov("Queue Family " + std::to_string(i) + ":");
		ov("queueFlags", vk_queue_families[i].qf_props.queueFlags);
		ov("queueCount", vk_queue_families[i].qf_props.queueCount); } }

//	Select a device queue for use in logical device
void engine::find_queue_family_index(
	svk::VK_PhysicalDevice 	*vk_physical_device,
	svk::VK_QueueFamily 	*vk_queue_families,
	uint32_t				*qf_index,
	uint32_t				queue_flag_bits,
	uint32_t				queue_flag_nots ) {
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

//	Create logical device
svk::VK_LogicalDevice engine::init_logical_device(
	svk::VK_PhysicalDevice 		*vk_physical_device,
	uint32_t					dev_queue_count,
	VkDeviceQueueCreateInfo		*vk_device_queue_info) {

	svk::VK_LogicalDevice result {};

	VkDeviceCreateInfo ld_info;
		ld_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		ld_info.pNext = nullptr;
		ld_info.flags = 0;
		ld_info.queueCreateInfoCount = dev_queue_count;
		ld_info.pQueueCreateInfos = vk_device_queue_info;
		ld_info.enabledLayerCount = 0;
		ld_info.ppEnabledLayerNames = nullptr;
		ld_info.enabledExtensionCount = 0;
		ld_info.ppEnabledExtensionNames = nullptr;
		ld_info.pEnabledFeatures = nullptr;

	ov( "Create Vulkan Logical Device" );
		svk::create_logical_device( vk_physical_device, &ld_info, &result);

	return result; }

//	Destroy logical device
void engine::exit_logical_device( svk::VK_LogicalDevice *vk_logical_device ) {
	ov( "Destroy Vulkan Logical Device" );
		svk::destroy_logical_device	( vk_logical_device	); }

//	Create Command Pool
void engine::init_command_pools(
	svk::VK_LogicalDevice 	*vk_logical_device,
	const std::vector<svk::VK_DeviceQueueInfo>& vk_device_queue_info,
	svk::VK_CommandPool		*vk_command_pool ) {
	ov( "Create Command Pools" );
	for(int i = 0; i < vk_device_queue_info.size(); i++) {
		vk_command_pool[i] = svk::new_vk_command_pool( vk_device_queue_info[i].queueFamilyIndex );
		svk::create_command_pool( vk_logical_device, &vk_command_pool[i] ); } }

//	Destroy Command Pool
void engine::exit_command_pool(
	svk::VK_LogicalDevice 	*vk_logical_device,
	svk::VK_CommandPool 	*vk_command_pool,
	size_t					 command_pool_count ) {
	ov( "Destroy Vulkan Command Pool(s)" );
	for(int i = 0; i < command_pool_count; i++) {
		svk::destroy_command_pool( vk_logical_device, &vk_command_pool[i] ); } }

//	Create shader module
void engine::init_shader_module(
	svk::VK_LogicalDevice 		*vk_logical_device,
	svk::VK_Shader 				*vk_shader,
	const char 					*shader_file,
	VkShaderStageFlagBits		vk_shader_stage ) {
	ov( "Create Vulkan Shader Module" );
	vk_shader->shaderFilename 	= shader_file;
	vk_shader->vk_shader_stage 	= vk_shader_stage;
	svk::create_shader_module( vk_logical_device, vk_shader ); }

//	Read out shader module config
void engine::show_shader_module(
	svk::VK_Shader *vk_shader ) {
	ov( "Shader Module" );
	ov( "SPIR-V Filepath", 	vk_shader->shaderFilename 				);
	ov( "Shader Stage", 	str_p2hex(vk_shader->vk_shader_stage)	);
	ov( "Shader Bytes", 	vk_shader->shaderData.size()			);
	ov( "shaderBytesValid", vk_shader->shaderBytesValid 			);
	ov( "Shader Module", 	vk_shader->vk_shader_module 			); } 

//	Destroy Shader Module 
void engine::exit_shader_module(
	svk::VK_LogicalDevice 	*vk_logical_device,
	svk::VK_Shader 			*vk_shader ) {
	ov( "Destroy Shader Module" );
		svk::destroy_shader_module( vk_logical_device, vk_shader ); }

//	Add Descriptor Set Layout Binding
void engine::add_dslb(
	svk::VK_Shader 		*vk_shader,
	VkDescriptorType	vk_desc_type,
	uint32_t			desc_count ) {
	ov( "VkDescriptorSetLayoutBinding" );
	ov( "Type", str_p2hex(vk_desc_type), vk_shader->vk_dslb.size() );
	ov( "Count", desc_count, vk_shader->vk_dslb.size() );
	vk_shader->vk_dslb.push_back( svk::new_vk_dslb( vk_shader, vk_desc_type, desc_count ) ); }

//	Create and Allocate the descriptor set(s)
void engine::init_descriptor_set(
	svk::VK_LogicalDevice 	*vk_logical_device,
	svk::VK_Shader 			*vk_shader ) {
	ov( "Create Vulkan Descriptor Set Layout" );
	svk::create_descriptor_set_layout( vk_logical_device, vk_shader );
	ov( "Create Vulkan Descriptor Pool" );
	svk::create_descriptor_pool( vk_logical_device, vk_shader );
	ov( "Allocate Vulkan Descriptor Set" );
	svk::allocate_descriptor_set( vk_logical_device, vk_shader ); }

//	Destroy Descriptor Pool & Descriptor Set Layout
void engine::exit_descriptor_set(
	svk::VK_LogicalDevice 	*vk_logical_device,
	svk::VK_Shader 			*vk_shader ) {
	ov( "Destroy Vulkan Descriptor Pool" );
	svk::destroy_descriptor_pool( vk_logical_device, vk_shader );
	ov( "Destroy Vulkan Descriptor Set Layout" );
	svk::destroy_descriptor_set_layout( vk_logical_device, vk_shader ); }




















