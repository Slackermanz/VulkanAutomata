#include "slack_engine.h"

void show_pdv_info( VK_Context *vk_context, VK_PhysicalDevice *vk_physical_device ) {
	for(int i = 0; i < vk_context->pd_count; i++) {
		ov("Physical Device " + std::to_string(i) + ":");
		ov("apiVersion", 		vk_physical_device[i].pd_props.apiVersion		);
		ov("driverVersion", 	vk_physical_device[i].pd_props.driverVersion	);
		ov("deviceType", 		vk_physical_device[i].pd_props.deviceType		);
		av("deviceName", 		vk_physical_device[i].pd_props.deviceName		);
		ov("memoryTypeCount", 	vk_physical_device[i].pd_memos.memoryTypeCount	);
		ov("memoryHeapCount", 	vk_physical_device[i].pd_memos.memoryHeapCount	);
		ov("QFP Count", 		vk_physical_device[i].qf_count					); } }

void show_qfp_info( VK_PhysicalDevice *vk_physical_device, VK_QueueFamily *vk_queue_families ) {
	for(int i = 0; i < vk_physical_device->qf_count; i++) {
		ov("Queue Family " + std::to_string(i) + ":");
		ov("queueFlags", vk_queue_families[i].qf_props.queueFlags);
		ov("queueCount", vk_queue_families[i].qf_props.queueCount); } }

void engine_vulkan_context_init( VK_Context *vk_context, const char *app_name ) {

	ov( "Define Vulkan Extensions" );
		const uint32_t 	VK_LAYER_EXTN_COUNT 						= 1;
		const char* 	VK_LAYER_EXTNS[VK_LAYER_EXTN_COUNT] 		= {	"VK_LAYER_KHRONOS_validation" };
		const uint32_t 	VK_INSTANCE_EXTN_COUNT 						= 1;
		const char* 	VK_INSTANCE_EXTNS[VK_INSTANCE_EXTN_COUNT] 	= {	VK_EXT_DEBUG_UTILS_EXTENSION_NAME };

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

	ov( "Vulkan Physical Device selection" );
		svk_find_physical_device( vk_physical_devices, vk_context ); }

void engine_vulkan_get_queue_families( VK_PhysicalDevice *vk_physical_device, VK_QueueFamily *vk_queue_families ) {
	ov( "Fill Struct: VK_QueueFamily" );
		svk_enum_queue_families( vk_queue_families, vk_physical_device ); }

void engine_vulkan_get_queue_family_index(
	VK_PhysicalDevice 	*vk_physical_device,
	VK_QueueFamily 		*vk_queue_families,
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

void engine_vulkan_logical_device_init(
	VK_PhysicalDevice 		*vk_physical_device,
	uint32_t				dev_queue_count,
	VkDeviceQueueCreateInfo	*vk_device_queue_info,
	VK_LogicalDevice 		*vk_logical_device ) {

	ov( "Init Vulkan Logical Device" );
		*vk_logical_device = new_vk_logical_device( dev_queue_count, vk_device_queue_info, 0, nullptr, nullptr );

	ov( "Create Vulkan Logical Device" );
		svk_create_logical_device( vk_physical_device, vk_logical_device ); }

void engine_vulkan_logical_device_exit( VK_LogicalDevice *vk_logical_device ) {
	ov( "Destroy Vulkan Logical Device" );
		svk_destroy_logical_device	( vk_logical_device	); }

void engine_vulkan_command_pool_init(
	VK_LogicalDevice 	*vk_logical_device,
	VK_DeviceQueueInfo	*vk_device_queue_info,
	VK_CommandPool		*vk_command_pool ) {
	ov( "Create Command Pools" );
	for(int i = 0; i < vk_logical_device->ld_info.queueCreateInfoCount; i++) {
		vk_command_pool[i] = new_vk_command_pool( vk_device_queue_info[i].queueFamilyIndex );
		svk_create_command_pool( vk_logical_device, &vk_command_pool[i] ); } }

void engine_vulkan_command_pool_exit(
	VK_LogicalDevice 	*vk_logical_device,
	VK_CommandPool 		*vk_command_pool ) {
	ov( "Destroy Vulkan Command Pool(s)" );
	for(int i = 0; i < vk_logical_device->ld_info.queueCreateInfoCount; i++) {
		svk_destroy_command_pool( vk_logical_device, &vk_command_pool[i] ); } }

/*void engine_vulkan_add_queue(
	VK_QueueFamily 			*vk_queue_families,
	VkDeviceQueueCreateInfo *vk_device_queue_info,
	uint32_t				qf_index,
	uint32_t				queue_count ) {
	float qp[vk_queue_families[qf_index].qf_props.queueCount] = { 0.0f };
	VkDeviceQueueCreateInfo vk_device_queue_create_info;
		vk_device_queue_info->sType				= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		vk_device_queue_info->pNext				= nullptr;
		vk_device_queue_info->flags				= 0;
		vk_device_queue_info->queueFamilyIndex	= qf_index;
		vk_device_queue_info->queueCount		= (queue_count != 0) ? queue_count : vk_queue_families[qf_index].qf_props.queueCount;
		vk_device_queue_info->pQueuePriorities	= qp; }*/
/*
void engine_vulkan_logical_device_init(
	VK_PhysicalDevice 		*vk_physical_device,
//	VK_DeviceQueueInfo 		*vk_device_queue_info,
	VK_LogicalDevice 		*vk_logical_device ) {

	ov( "Init Vulkan Logical Device" );
		*vk_logical_device = new_vk_logical_device( , , 0, nullptr, nullptr );

	ov( "Create Vulkan Logical Device" );
		svk_create_logical_device( vk_physical_device, vk_logical_device ); }*/








/*void engine_vulkan_context_exit(
	VK_Context 			*vk_context,
	VK_LogicalDevice 	*vk_logical_device,
	VK_CommandPool		 *vk_command_pool ) {
	ov( "Destroy Vulkan Context" );
	for(int i = 0; i < vk_logical_device->ld_info.queueCreateInfoCount; i++) {
		svk_destroy_command_pool	( vk_logical_device, &vk_command_pool[i] ); }
		svk_destroy_logical_device	( vk_logical_device	);
		svk_destroy_debug_utils		( vk_context 		);
		svk_destroy_instance		( vk_context 		); }*/
/*
void engine_vulkan_get_queue_family_index(
	VK_PhysicalDevice 	*vk_physical_device,
	VK_QueueFamily 		*vk_queue_families,
	uint32_t			*qf_index,
	uint32_t			queue_flag_bits,
	uint32_t			queue_flag_nots ) {
	ov( "Check QF Index" );
		ov("bits", queue_flag_bits);
		ov("nots", queue_flag_nots);
	for(int i = 0; i < vk_physical_device->qf_count; i++) {
		if( *qf_index													== UINT32_MAX
		&& (vk_queue_families[i].qf_props.queueFlags & queue_flag_bits) == queue_flag_bits
		&& (vk_queue_families[i].qf_props.queueFlags & queue_flag_nots) == 0 ) {
			*qf_index = i; } }
	ov("Selected Index", *qf_index); }

/*void engine_vulkan_queue_list_add(
	VK_PhysicalDevice 	*vk_physical_device,
	VK_QueueFamily 		*vk_queue_families,
	VK_QueueList		*vk_queue_list,
	uint32_t			queue_max_threads,
	uint32_t			queue_flag_bits,
	uint32_t			queue_flag_nots ) {

	vk_queue_list->qf_index = UINT32_MAX;
	engine_vulkan_get_queue_family_index(
		vk_physical_device,
		vk_queue_families,
		&vk_queue_list->qf_index,
		queue_flag_bits,
		queue_flag_nots );

	if( vk_queue_list->qf_index == UINT32_MAX ) { queue_max_threads = 0; }
	else 										{
		queue_max_threads 	= (queue_max_threads == 0) ? UINT32_MAX : queue_max_threads;
		queue_max_threads 	= (queue_max_threads > vk_queue_families[vk_queue_list->qf_index].qf_props.queueCount)
							? vk_queue_families[vk_queue_list->qf_index].qf_props.queueCount
							: queue_max_threads; }

	ov("Queue Threads", queue_max_threads);

	vk_queue_list->queue_max_threads = queue_max_threads; }*/


/*void engine_vulkan_queue_list_count(
	const uint32_t 	 QTC,
	VK_QueueList	*vk_queue_list,
	uint32_t 		*queue_list_count,
	bool 			*use_queue ) {

	uint32_t qlc = *queue_list_count;

	ov( "Queue List Indexes" );
	for(int i = 0; i < QTC; i++) { ov("qf_index", vk_queue_list[i].qf_index, i); }

	ov( "Check Queue Viability" );
	for(int i = 0; i < QTC; i++) { use_queue[i] = true; }
	for(int i = 0; i < QTC; i++) {
		if( vk_queue_list[i].qf_index != UINT32_MAX ) 	{ qlc++; }
		else 											{ use_queue[i] = false; ov("Reject Queue", i); }
		for(int j = i; j < QTC; j++) {
			if( j != i
			&& 	vk_queue_list[j].qf_index != UINT32_MAX
			&& 	vk_queue_list[j].qf_index == vk_queue_list[i].qf_index ) {
				qlc--;
				use_queue[j] = false; ov("Reject Queue", j);
				break; } } }

	*queue_list_count = qlc;

	ov("queue_list_count", *queue_list_count); }*/

/*void engine_vulkan_queue_list_select(
	const uint32_t 	 QTC,
	VK_QueueList	*vk_queue_type_list,
	bool 			*use_queue,
	VK_QueueList	*vk_queue_list ) {
		uint32_t queue_list_index = 0;
		for(int i = 0; i < QTC; i++) {
			if( use_queue[i] ) {
				ov("use_queue", i);
				vk_queue_list[queue_list_index] = vk_queue_type_list[i];
				queue_list_index++; } } }*/










/*void engine_vulkan_logical_device_init(
	VK_PhysicalDevice 		*vk_physical_device,
	uint32_t				 vk_queue_list_count,
	VK_QueueList			*vk_queue_list,
	VK_LogicalDevice 		*vk_logical_device ) {

	ov( "Init VkDeviceQueueCreateInfo" );
	float qp = 0.0f;
	VkDeviceQueueCreateInfo vk_device_queue_info[vk_queue_list_count];
	for(int i = 0; i < vk_queue_list_count; i++) {
		vk_device_queue_info[i].sType 				= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		vk_device_queue_info[i].pNext 				= nullptr;
		vk_device_queue_info[i].flags 				= 0;
		vk_device_queue_info[i].queueFamilyIndex 	= vk_queue_list[i].qf_index;
		vk_device_queue_info[i].queueCount 			= vk_queue_list[i].queue_max_threads;
		vk_device_queue_info[i].pQueuePriorities	= &qp; }

	ov( "Init Vulkan Logical Device" );
		*vk_logical_device = new_vk_logical_device( vk_queue_list_count, vk_device_queue_info, 0, nullptr, nullptr );

	ov( "Create Vulkan Logical Device" );
		svk_create_logical_device( vk_physical_device, vk_logical_device ); }*/




/*void engine_vulkan_command_pool_init(
	VK_QueueList		*vk_queue_list,
	VK_LogicalDevice 	*vk_logical_device,
	VK_CommandPool		*vk_command_pool ) {
	ov( "Create Command Pools" );
	for(int i = 0; i < vk_logical_device->ld_info.queueCreateInfoCount; i++) {
		vk_command_pool[i] = new_vk_command_pool( vk_queue_list[i].qf_index );
		svk_create_command_pool( vk_logical_device, &vk_command_pool[i] ); } }*/




