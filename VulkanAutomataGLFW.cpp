#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "lib/imgui.h"
#include "lib/imgui_impl_vulkan.h"
#include "lib/imgui_impl_glfw.h"
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <thread>
#include <cstring>
#include <cmath>

// Include our modularized type definitions
#include "vkmodules/Types/AllTypes.h"
#include "vkmodules/Utils/Utils.h"
#include "vkmodules/CellularAutomata/CellularAutomata.h"
#include "vkmodules/UI/UI.h"
#include "vkmodules/Input/Input.h"
#include "vkmodules/VulkanFoundation/VulkanFoundation.h"
#include "vkmodules/CommandBuffer/CommandBuffer.h"
#include "vkmodules/Resources/Resources.h"
#include "vkmodules/Rendering/Rendering.h"
#include "vkmodules/Platform/Platform.h"

const 	uint32_t 	VERT_FLS 		=  1;	//	Number of Vertex Shader Files
const 	uint32_t 	FRAG_FLS 		=  1;	//	Number of Fragment Shader Files
const	int 		MAXLOG 			=  2;
		int 		valid 			=  1;
		uint32_t	verbose_loops 	= 12;	// How many loops to output full diagnostics
        uint32_t    panel_n_x_n     =  4;

//	Some shaders require dimensions to be powers of two (fast wrap-around/torus code)
//		16:9 Resolutions:
//			W:	1920 	1536 	1280	768		512		384		256
//			H:	1080 	864  	720		432		288		216		144
//		2:1 Resolutions:
//			W:	16384 	8192 	4096	2048	1024	512		256
//			H:	8192 	4096  	2048	1024	512		256		128

	const 	uint32_t 	APP_W 	= 64*4*2*4;	//	Window & Simulation Width
	const 	uint32_t 	APP_H 	= 64*4*1*4;	//	Window & Simulation Height

int main() {

//	Set the random seed
	uint32_t INIT_TIME = time(0);
	srand(INIT_TIME);

//	Result storage
	std::vector<VkResult> vkres;

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "USER CONFIG");			/**/
	///////////////////////////////////////////////////


	EngineInfo ei;
		ei.paused 				= false;				//	Pause Simulation
		ei.show_gui 			=  true;				//	Render Dear IMGUI
		ei.run_headless 		= false;				//	Use Headless Mode
		ei.imgdat_idx 			= 0;					//	Use Headless Mode
		ei.export_frequency 	= 8;					//	Export image file every n frames
		ei.export_batch_size 	= 180;					//	Pause recording after n exported frames
		ei.export_batch_left 	= ei.export_batch_size;	//	Frames remaining in batch
		ei.export_batch_last 	= ei.export_batch_size;	//	For IMGUI handling
		ei.export_enabled		= false;				//	Enables image exports to disk
		ei.tick_loop 			= 0;					//	Run the main loop n times, ignoring pause state
		ei.load_pattern 		= 18372;				//	Load this index from the pattern archive file

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "APPLICATION INIT");		/**/
	///////////////////////////////////////////////////

//	Paths to shader files
	const 	char* 	filepath_vert		[VERT_FLS]
	=	{	"./app/vert_TriQuad.spv" 			};
	const 	char* 	filepath_frag		[FRAG_FLS]
	=	{	"./app/frag_automata0000.spv"		};

//	Config Notification Messages
	ov("Application Width", 	APP_W			);
	ov("Application Height", 	APP_H			);
	ov("ei.load_pattern", 		ei.load_pattern	);
	for(int i = 0; i < VERT_FLS; i++) {	iv("Vertex Shaders", 			filepath_vert[i], 		i ); }
	for(int i = 0; i < FRAG_FLS; i++) {	iv("Fragment Shaders", 			filepath_frag[i], 		i ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "GLFW EXTENSIONS");		/**/
	///////////////////////////////////////////////////

	uint32_t glfw_ext_count;
	const char** glfw_extentions; // Keep original variable name with typo for consistency
	initGLFWExtensions(&glfw_ext_count, &glfw_extentions, &vkres);

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "VULKAN EXTENSIONS");		/**/
	///////////////////////////////////////////////////

	uint32_t 	INST_EXS 		= 1 + glfw_ext_count;	//	Number of Vulkan Instance Extensions
	uint32_t 	LAYR_EXS 		= 1;					//	Number of Vulkan Layers
	uint32_t 	LDEV_EXS 		= 1;					//	Number of Vulkan Logical Device Extensions

//	Paths to shader files and extension names
	const 	char* 	instance_extensions	[INST_EXS]
	=	{	"VK_EXT_debug_utils", 
			glfw_extentions[0], 
			glfw_extentions[1]					};	// TODO Number of entries might not be the same for all systems!
	const 	char* 	layer_extensions	[LAYR_EXS]
	=	{	"VK_LAYER_KHRONOS_validation" 		};
	const 	char* 	device_extensions	[LDEV_EXS]
	=	{	VK_KHR_SWAPCHAIN_EXTENSION_NAME		};

//	Extension Notification Messages
	for(int i = 0; i < INST_EXS; i++) {	iv("Instance Extensions", 		instance_extensions[i], i ); }
	for(int i = 0; i < LAYR_EXS; i++) {	iv("Layer Extensions", 			layer_extensions[i], 	i ); }
	for(int i = 0; i < LDEV_EXS; i++) {	iv("Logical Device Extensions", device_extensions[i], 	i ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "VULKAN INIT");			/**/
	///////////////////////////////////////////////////

	VK_Obj vob;
	VK_Config vkcfg;
	
	initVulkanInstance(&vob, &vkcfg, instance_extensions, INST_EXS, 
					  layer_extensions, LAYR_EXS, &vkres);

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "DEBUG UTILS");			/**/
	///////////////////////////////////////////////////

	VK_Debug vkdbg;
	setupDebugMessenger(&vob, &vkdbg, &vkres);

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "PHYSICAL DEVICE");		/**/
	///////////////////////////////////////////////////

	VK_PhysDev selectedPdevInfo; // To store properties/features of the selected device
	selectPhysicalDevice(
		vob.VKI,            // Vulkan instance handle
		&vob,               // Output: Stores VKP handle and index
		&selectedPdevInfo,  // Output: Stores selected device properties/features
		&vkres);            // Result vector

	// --- Queue Selection ---
	hd("STAGE:", "QUEUES"); // Keep stage header

	uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
	uint32_t graphicsQueueCount = 0;
	findGraphicsQueueFamily(
		vob.VKP,                // Selected physical device handle
		&graphicsQueueFamilyIndex, // Output: Queue family index
		&graphicsQueueCount,       // Output: Queue count in the family
		&vkres);                // Result vector

	// Store the found queue index in vob
	vob.VKQ_i = graphicsQueueFamilyIndex;

	// Setup queue priorities array (using the found queueCount)
	std::vector<float> gfxQueuePriorities(graphicsQueueCount, 0.0f); // Use std::vector

	VK_PDQueues pdq; // Keep declaration for queue create info struct
	setupDeviceQueueCreateInfo(
		vob.VKQ_i,                  // The selected graphics queue family index
		graphicsQueueCount,         // The number of queues in the family
		gfxQueuePriorities.data(),  // Pointer to the priorities array
		&pdq);                      // Output struct

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "GLFW VULKAN SURFACE");	/**/
	///////////////////////////////////////////////////
	
	VkSurfaceKHR 				glfw_surface = VK_NULL_HANDLE; // Initialize
	VkSurfaceCapabilitiesKHR 	vk_surface_capabilities = {}; // Initialize
	GLFWwindow* 				glfw_W = nullptr; // Initialize

	createGLFWWindowAndSurface(
		APP_W,                      // Width
		APP_H,                      // Height
		vkcfg.app_info.pApplicationName, // Title from app info
		&vob,                       // Core Vulkan objects
		&ei,                        // Engine info
		&glfw_W,                    // Output: Window handle
		&glfw_surface,              // Output: Surface handle
		&vk_surface_capabilities,   // Output: Surface capabilities
		&vkres                      // Result vector
	);

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "LOGICAL DEVICE");			/**/
	///////////////////////////////////////////////////

	createLogicalDevice(
		vob.VKP,                    // Physical device handle
		&pdq,                       // Struct containing queue create info
		device_extensions,          // Device extensions array
		LDEV_EXS,                   // Device extension count
		&selectedPdevInfo.vk_pdev_feats, // Enabled features from selected physical device
		&vob,                       // Output: stores logical device handle (VKL)
		&vkres);                    // Result vector

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "SWAPCHAIN");				/**/
	///////////////////////////////////////////////////

	VkSwapchainKHR vk_swapchain = VK_NULL_HANDLE; // Initialize to NULL
	uint32_t swap_image_count = 0;
	std::vector<VkImage> vk_image_swapimgs_vec; // Use std::vector

	if(!ei.run_headless) {
        createSwapChain(
            &vob,                       // Core Vulkan objects
            glfw_surface,               // Window surface
            &vk_surface_capabilities,   // Surface capabilities
            vob.VKQ_i,                  // Graphics queue family index (using the one from vob)
            &vk_swapchain,              // Pass current swapchain (will be VK_NULL_HANDLE initially)
            &vk_swapchain,              // Output: Swapchain handle
            &swap_image_count,          // Output: Image count
            &vk_image_swapimgs_vec,     // Output: Image handles vector
            &vkres);                    // Result vector
	} else { rv("Headless Mode Enabled!"); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "WORK LAYER IMAGES");		/**/
	///////////////////////////////////////////////////

	VK_Layer_2x2D work; // Keep the declaration of the struct array

	for(int i = 0; i < 2; i++) {
		// Use a temporary VK_Layer_1x2D to pass to the function
		VK_Layer_1x2D tempImageData;

		createImage(
			vob.VKL,                    // Logical device
			vob.VKP,                    // Physical device
			APP_W,                      // Width
			APP_H,                      // Height
			VK_FORMAT_R16G16B16A16_UNORM, // Format
			VK_IMAGE_TILING_OPTIMAL,    // Tiling
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, // Usage flags
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, // Memory properties
			VK_SHARING_MODE_EXCLUSIVE,  // Sharing mode
			NULL,                       // pQueueFamilyIndices (optional for exclusive)
			0,                          // queueFamilyIndexCount
			&tempImageData,             // Output struct (single image data)
			&vkres);                    // Result vector

		// Copy results from temp struct to the correct index in the work array
		work.ext3D[i] = tempImageData.ext3D;
		work.img_info[i] = tempImageData.img_info;
		work.vk_image[i] = tempImageData.vk_image;
		work.MTB_index[i] = tempImageData.MTB_index;
		work.vk_mem_reqs[i] = tempImageData.vk_mem_reqs;
		work.vk_mem_allo_info[i] = tempImageData.vk_mem_allo_info;
		work.vk_dev_mem[i] = tempImageData.vk_dev_mem;
	}
	
	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "BLIT EXPORT IMAGE");		/**/
	///////////////////////////////////////////////////

	VK_Layer_1x2D blit; // Keep the declaration

	createImage(
		vob.VKL,                    // Logical device
		vob.VKP,                    // Physical device
		APP_W,                      // Width
		APP_H,                      // Height
		VK_FORMAT_R8G8B8A8_UNORM,   // Format
		VK_IMAGE_TILING_OPTIMAL,    // Tiling
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, // Usage flags
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, // Memory properties
		VK_SHARING_MODE_EXCLUSIVE,  // Sharing mode
		NULL,                       // pQueueFamilyIndices
		0,                          // queueFamilyIndexCount
		&blit,                      // Output struct (directly use 'blit' here)
		&vkres);                    // Result vector

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "BLIT EXPORT BUFFER");		/**/
	///////////////////////////////////////////////////

	VK_Buffer_Data blit2buff_data; // Declare the new struct
	VkDeviceSize blit_buffer_size = blit.vk_mem_reqs.size; // Get size from the blit image mem reqs
	createBuffer(
		vob.VKL,
		vob.VKP,
		blit_buffer_size, // Use the size from the blit image
		VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		&vob.VKQ_i, // Pass address of the queue index
		1,          // Queue family index count
		&blit2buff_data, // Pass the address of our data struct
		&vkres);

	//	Map the memory location on the GPU to export image data
		void* pvoid_blit2buff;
		vr("vkMapMemory", &vkres, pvoid_blit2buff,
			vkMapMemory(vob.VKL, blit2buff_data.vk_dev_mem, 0, VK_WHOLE_SIZE, 0, &pvoid_blit2buff) );

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "SHADER DATA");			/**/
	///////////////////////////////////////////////////

	ShaderData shade_data[VERT_FLS + FRAG_FLS]; // Keep declaration

	// Load Vertex Shaders
	for (int i = 0; i < VERT_FLS; i++) {
		loadAndCreateShaderModule(
			vob.VKL,
			filepath_vert[i],
			&shade_data[i], // Output struct for this shader
			&vkres);
		shade_data[i].stage_bits = VK_SHADER_STAGE_VERTEX_BIT; // Set stage after loading
	}

	// Load Fragment Shaders
	for (int i = VERT_FLS; i < VERT_FLS + FRAG_FLS; i++) {
		loadAndCreateShaderModule(
			vob.VKL,
			filepath_frag[i - VERT_FLS], // Adjust index for frag file array
			&shade_data[i], // Output struct for this shader
			&vkres);
		shade_data[i].stage_bits = VK_SHADER_STAGE_FRAGMENT_BIT; // Set stage after loading
	}

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RENDERPASS INFO");		/**/
	///////////////////////////////////////////////////

	VK_RPConfig rpass_info;

		rpass_info.rect2D.offset.x		= 0;
		rpass_info.rect2D.offset.y		= 0;
		rpass_info.rect2D.extent.width	= APP_W;
		rpass_info.rect2D.extent.height	= APP_H;

		rpass_info.clear_val.color = { 1.0f, 0.0f, 0.0f, 1.0f };

		rpass_info.img_subres_range.aspectMask 			= VK_IMAGE_ASPECT_COLOR_BIT;
		rpass_info.img_subres_range.baseMipLevel 		= 0;
		rpass_info.img_subres_range.levelCount 			= 1;
		rpass_info.img_subres_range.baseArrayLayer 		= 0;
		rpass_info.img_subres_range.layerCount 			= 1;

		rpass_info.img_subres_layer.aspectMask 			= VK_IMAGE_ASPECT_COLOR_BIT;
		rpass_info.img_subres_layer.mipLevel 			= 0;
		rpass_info.img_subres_layer.baseArrayLayer 		= 0;
		rpass_info.img_subres_layer.layerCount 			= 1;

		rpass_info.img_blit.srcSubresource 				= rpass_info.img_subres_layer;
		rpass_info.img_blit.srcOffsets[0].x 			= 0;
		rpass_info.img_blit.srcOffsets[0].y 			= 0;
		rpass_info.img_blit.srcOffsets[0].z 			= 0;
		rpass_info.img_blit.srcOffsets[1].x 			= APP_W;
		rpass_info.img_blit.srcOffsets[1].y 			= APP_H;
		rpass_info.img_blit.srcOffsets[1].z 			= 1;
		rpass_info.img_blit.dstSubresource 				= rpass_info.img_subres_layer;
		rpass_info.img_blit.dstOffsets[0].x 			= 0;
		rpass_info.img_blit.dstOffsets[0].y 			= 0;
		rpass_info.img_blit.dstOffsets[0].z 			= 0;
		rpass_info.img_blit.dstOffsets[1].x 			= APP_W;
		rpass_info.img_blit.dstOffsets[1].y 			= APP_H;
		rpass_info.img_blit.dstOffsets[1].z 			= 1;

		rpass_info.buffer_img_cpy.bufferOffset 			= 0;
		rpass_info.buffer_img_cpy.bufferRowLength 		= APP_W;
		rpass_info.buffer_img_cpy.bufferImageHeight 	= APP_H;
		rpass_info.buffer_img_cpy.imageSubresource 		= rpass_info.img_subres_layer;
		rpass_info.buffer_img_cpy.imageOffset.x 		= 0;
		rpass_info.buffer_img_cpy.imageOffset.y 		= 0;
		rpass_info.buffer_img_cpy.imageOffset.z 		= 0;
		rpass_info.buffer_img_cpy.imageExtent.width 	= APP_W;
		rpass_info.buffer_img_cpy.imageExtent.height 	= APP_H;
		rpass_info.buffer_img_cpy.imageExtent.depth		= 1;

		rpass_info.vk_viewport.x						= 0;
		rpass_info.vk_viewport.y						= 0;
		rpass_info.vk_viewport.width					= APP_W;
		rpass_info.vk_viewport.height					= APP_H;
		rpass_info.vk_viewport.minDepth					= 0.0f;
		rpass_info.vk_viewport.maxDepth					= 1.0f;

	createDefaultSampler(
		vob.VKL,                // Logical device
		&rpass_info.vk_sampler, // Output sampler handle (stored in VK_RPConfig struct)
		&vkres);                // Result vector

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "PIPELINE INFO"); 			/**/
	///////////////////////////////////////////////////

	VK_PipeInfo pipe_info;

		pipe_info.p_rast_info.sType						= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	nf(&pipe_info.p_rast_info);
		pipe_info.p_rast_info.depthClampEnable			= VK_FALSE;
		pipe_info.p_rast_info.rasterizerDiscardEnable	= VK_FALSE;
		pipe_info.p_rast_info.polygonMode				= VK_POLYGON_MODE_FILL;
		pipe_info.p_rast_info.cullMode					= VK_CULL_MODE_NONE;
		pipe_info.p_rast_info.frontFace					= VK_FRONT_FACE_CLOCKWISE;
		pipe_info.p_rast_info.depthBiasEnable			= VK_FALSE;
		pipe_info.p_rast_info.depthBiasConstantFactor	= 0.0f;
		pipe_info.p_rast_info.depthBiasClamp			= 0.0f;
		pipe_info.p_rast_info.depthBiasSlopeFactor		= 0.0f;
		pipe_info.p_rast_info.lineWidth					= 1.0f;

		pipe_info.p_vprt_info.sType 					= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	nf(&pipe_info.p_vprt_info);
		pipe_info.p_vprt_info.viewportCount				= 1;
		pipe_info.p_vprt_info.pViewports				= &rpass_info.vk_viewport;
		pipe_info.p_vprt_info.scissorCount				= 1;
		pipe_info.p_vprt_info.pScissors					= &rpass_info.rect2D;

		pipe_info.p_msam_info.sType						= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	nf(&pipe_info.p_msam_info);
		pipe_info.p_msam_info.rasterizationSamples		= VK_SAMPLE_COUNT_1_BIT;
		pipe_info.p_msam_info.sampleShadingEnable		= VK_FALSE;
		pipe_info.p_msam_info.minSampleShading			= 0.0f;
		pipe_info.p_msam_info.pSampleMask				= NULL;
		pipe_info.p_msam_info.alphaToCoverageEnable		= VK_FALSE;
		pipe_info.p_msam_info.alphaToOneEnable			= VK_FALSE;

		pipe_info.p_vtin_info.sType						= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	nf(&pipe_info.p_vtin_info);
		pipe_info.p_vtin_info.vertexBindingDescriptionCount		= 0;
		pipe_info.p_vtin_info.pVertexBindingDescriptions		= NULL;
		pipe_info.p_vtin_info.vertexAttributeDescriptionCount	= 0;
		pipe_info.p_vtin_info.pVertexAttributeDescriptions		= NULL;

		pipe_info.p_inas_info.sType						= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	nf(&pipe_info.p_inas_info);
		pipe_info.p_inas_info.topology					= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipe_info.p_inas_info.primitiveRestartEnable	= VK_FALSE;

		pipe_info.p_cbat_info.blendEnable				= VK_FALSE;
		pipe_info.p_cbat_info.srcColorBlendFactor		= VK_BLEND_FACTOR_SRC_ALPHA;
		pipe_info.p_cbat_info.dstColorBlendFactor		= VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		pipe_info.p_cbat_info.colorBlendOp				= VK_BLEND_OP_ADD;
		pipe_info.p_cbat_info.srcAlphaBlendFactor		= VK_BLEND_FACTOR_ONE;
		pipe_info.p_cbat_info.dstAlphaBlendFactor		= VK_BLEND_FACTOR_ZERO;
		pipe_info.p_cbat_info.alphaBlendOp				= VK_BLEND_OP_ADD;
		pipe_info.p_cbat_info.colorWriteMask			= 15;

		pipe_info.p_cbst_info.sType						= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	nf(&pipe_info.p_cbst_info);
		pipe_info.p_cbst_info.logicOpEnable				= VK_FALSE;
		pipe_info.p_cbst_info.logicOp					= VK_LOGIC_OP_NO_OP;
		pipe_info.p_cbst_info.attachmentCount			= 1;
		pipe_info.p_cbst_info.pAttachments				= &pipe_info.p_cbat_info;
		pipe_info.p_cbst_info.blendConstants[0]			= 1.0f;
		pipe_info.p_cbst_info.blendConstants[1]			= 1.0f;
		pipe_info.p_cbst_info.blendConstants[2]			= 1.0f;
		pipe_info.p_cbst_info.blendConstants[3]			= 1.0f;

		for(int i = 0; i < VERT_FLS+FRAG_FLS; i++) {
			pipe_info.p_shad_info[i].sType					= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		nf(&pipe_info.p_shad_info[i]);
			pipe_info.p_shad_info[i].stage					= shade_data[i].stage_bits;
			pipe_info.p_shad_info[i].module					= shade_data[i].vk_shader_module;
			pipe_info.p_shad_info[i].pName					= "main";
			pipe_info.p_shad_info[i].pSpecializationInfo	= NULL; }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "COMMAND BUFFERS");		/**/
	///////////////////////////////////////////////////

	// Are all of these used? TODO

	VK_Command combuf_blit2buff_sing[1];
	createCommandBuffers(vob.VKL, vob.VKQ_i, 1, combuf_blit2buff_sing, &vkres);

	VK_Command combuf_imgui_loop[swap_image_count];
	createCommandBuffers(vob.VKL, vob.VKQ_i, swap_image_count, combuf_imgui_loop, &vkres);

	VK_Command combuf_pres_init[swap_image_count];
	createCommandBuffers(vob.VKL, vob.VKQ_i, swap_image_count, combuf_pres_init, &vkres);

	VK_Command combuf_pres_loop[swap_image_count*2];
    // Note the count is swap_image_count * 2
    createCommandBuffers(vob.VKL, vob.VKQ_i, swap_image_count * 2, combuf_pres_loop, &vkres);

	VK_Command combuf_work_init[2];
	createCommandBuffers(vob.VKL, vob.VKQ_i, 2, combuf_work_init, &vkres);

	VK_Command combuf_work_loop[2];
	createCommandBuffers(vob.VKL, vob.VKQ_i, 2, combuf_work_loop, &vkres);

	VK_Command combuf_work_imagedata_init[1];
	createCommandBuffers(vob.VKL, vob.VKQ_i, 1, combuf_work_imagedata_init, &vkres);

	VK_Command combuf_work_imagedata[2];
	createCommandBuffers(vob.VKL, vob.VKQ_i, 2, combuf_work_imagedata, &vkres);

	VK_Command combuf_blit_imgui_loop[swap_image_count];
	createCommandBuffers(vob.VKL, vob.VKQ_i, swap_image_count, combuf_blit_imgui_loop, &vkres);

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "QUEUE SYNC");				/**/
	///////////////////////////////////////////////////

	VK_QueueSync qsync;

	rv("vkGetDeviceQueue");
		vkGetDeviceQueue(vob.VKL, vob.VKQ_i, 0, &qsync.vk_queue);

		qsync.sub_info.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		qsync.sub_info.pNext					= NULL;
		qsync.sub_info.waitSemaphoreCount		= 0;
		qsync.sub_info.pWaitSemaphores			= NULL;
		qsync.sub_info.pWaitDstStageMask		= NULL;
		qsync.sub_info.commandBufferCount		= 1;
		qsync.sub_info.pCommandBuffers			= NULL;
		qsync.sub_info.signalSemaphoreCount		= 0;
		qsync.sub_info.pSignalSemaphores		= NULL;

	createFence(
		vob.VKL,        // Logical device
		0,              // Flags (0 for unsignaled)
		&qsync.vk_fence,// Output fence handle
		&vkres);        // Result vector

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "SWAPCHAIN SYNC");			/**/
	///////////////////////////////////////////////////

	VkSemaphore vk_semaphore_swapchain_img_acq; // Keep declaration
	createSemaphore(
		vob.VKL,
		&vk_semaphore_swapchain_img_acq,
		&vkres);

	VkSemaphore vk_semaphore_swapchain_imgui; // Keep declaration
	createSemaphore(
		vob.VKL,
		&vk_semaphore_swapchain_imgui,
		&vkres);

	VkSemaphore vk_semaphore_swapchain_pres; // Keep declaration
	createSemaphore(
		vob.VKL,
		&vk_semaphore_swapchain_pres,
		&vkres);

	uint32_t swap_image_index = 0;

	VK_QueueSync swpsync;
	rv("vkGetDeviceQueue");
		vkGetDeviceQueue(vob.VKL, vob.VKQ_i, 0, &swpsync.vk_queue);
	VkPipelineStageFlags vk_pipeline_stage_flags_swpsync;
		vk_pipeline_stage_flags_swpsync = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		swpsync.sub_info.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		swpsync.sub_info.pNext					= NULL;
		swpsync.sub_info.waitSemaphoreCount		= 1;
		swpsync.sub_info.pWaitSemaphores		= &vk_semaphore_swapchain_img_acq;
		swpsync.sub_info.pWaitDstStageMask		= &vk_pipeline_stage_flags_swpsync;
		swpsync.sub_info.commandBufferCount		= 1;
//		swpsync.sub_info.pCommandBuffers		= &combuf_pres_loop[0].vk_command_buffer; // TODO i? 0, 1, 2 ? 
		swpsync.sub_info.signalSemaphoreCount	= 1;
		swpsync.sub_info.pSignalSemaphores		= &vk_semaphore_swapchain_imgui;

	VK_QueueSync swpsync_imgui;
	rv("vkGetDeviceQueue");
		vkGetDeviceQueue(vob.VKL, vob.VKQ_i, 0, &swpsync_imgui.vk_queue);
	VkPipelineStageFlags vk_pipeline_stage_flags_swpsync_imgui;
		vk_pipeline_stage_flags_swpsync_imgui 			= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		swpsync_imgui.sub_info.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		swpsync_imgui.sub_info.pNext					= NULL;
		swpsync_imgui.sub_info.waitSemaphoreCount		= 1;
		swpsync_imgui.sub_info.pWaitSemaphores			= &vk_semaphore_swapchain_imgui;
		swpsync_imgui.sub_info.pWaitDstStageMask		= &vk_pipeline_stage_flags_swpsync_imgui;
		swpsync_imgui.sub_info.commandBufferCount		= 1;
//		swpsync_imgui.sub_info.pCommandBuffers		= &combuf_imgui_loop[swap_image_index].vk_command_buffer;
		swpsync_imgui.sub_info.signalSemaphoreCount		= 1;
		swpsync_imgui.sub_info.pSignalSemaphores		= &vk_semaphore_swapchain_pres;

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "WORK IMAGE VIEWS");		/**/
	///////////////////////////////////////////////////

	VK_ImageView work_init[2]; // Keep declaration

	for(int i = 0; i < 2; i++) {
		createImageView(
			vob.VKL,                    // Logical device
			work.vk_image[i],           // Source image handle from work array
			work.img_info[i].format,    // Format from work image info
			VK_IMAGE_ASPECT_COLOR_BIT,  // Aspect flags
			&work_init[i],              // Output struct for this view
			&vkres);                    // Result vector

		// --- Keep the Image Memory Barrier setup that follows ---
		work_init[i].img_mem_barr.sType                 = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		work_init[i].img_mem_barr.pNext                 = NULL;
		work_init[i].img_mem_barr.srcAccessMask         = 0;
		work_init[i].img_mem_barr.dstAccessMask         = 0;
		work_init[i].img_mem_barr.oldLayout             = VK_IMAGE_LAYOUT_UNDEFINED;
		work_init[i].img_mem_barr.newLayout             = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		work_init[i].img_mem_barr.srcQueueFamilyIndex   = vob.VKQ_i;
		work_init[i].img_mem_barr.dstQueueFamilyIndex   = vob.VKQ_i;
		work_init[i].img_mem_barr.image                 = work.vk_image[i];
		work_init[i].img_mem_barr.subresourceRange      = rpass_info.img_subres_range;
	}

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RECORD WORK_INIT");		/**/
	///////////////////////////////////////////////////

	for(int i = 0; i < 2; i++) {
		vr("vkBeginCommandBuffer", &vkres, i,
			vkBeginCommandBuffer(combuf_work_init[i].vk_command_buffer, &combuf_work_init[i].comm_buff_begin_info) );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_work_init[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &work_init[i].img_mem_barr );

		vr("vkEndCommandBuffer", &vkres, i,
			vkEndCommandBuffer(combuf_work_init[i].vk_command_buffer) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "SUBMIT WORK_INIT");		/**/
	///////////////////////////////////////////////////

	for(int i = 0; i < 2; i++) {
		if(valid) {
			rv("vkcombuf_work_init");
				qsync.sub_info.pCommandBuffers = &combuf_work_init[i].vk_command_buffer;
			vr("vkQueueSubmit", &vkres, i,
				vkQueueSubmit(qsync.vk_queue, 1, &qsync.sub_info, VK_NULL_HANDLE) ); } }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "WORK DESCRIPTOR SETS");	/**/
	///////////////////////////////////////////////////

	VK_DescSetLayout3 dsl_work;

	createWorkDescriptorSet(
		vob.VKL,        // Logical device
		&dsl_work,      // Input/Output struct for layout, pool, sets
		2,              // Number of sets to allocate (for ping-pong)
		&vkres);        // Result vector


	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "WORK UNIFORM BUFFER");	/**/
	///////////////////////////////////////////////////

	VkDeviceSize vkdevsize_work;
	vkdevsize_work = sizeof(UB32_64);
	ov("UB32_64 size", vkdevsize_work);

	VK_Buffer_Data work_ub_data; // Struct to hold results
	createBuffer(
		vob.VKL,                    // Logical device
		vob.VKP,                    // Physical device
		vkdevsize_work,             // Size of the buffer
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, // Usage flags
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // Memory properties
		VK_SHARING_MODE_EXCLUSIVE,  // Sharing mode
		&vob.VKQ_i,                 // Queue family indices
		1,                          // Queue family index count
		&work_ub_data,              // Output struct
		&vkres);                    // Result vector

	// --- Keep the original descriptor info setup ---
	VkBuffer vkbuff_work = work_ub_data.vk_buffer; // Get the buffer handle
	VkDeviceMemory vkdevmem_ub_work = work_ub_data.vk_dev_mem; // Get the memory handle

    VkDescriptorBufferInfo vkDescBuff_info_work;
        vkDescBuff_info_work.buffer         = vkbuff_work;
        vkDescBuff_info_work.offset         = 0;
        vkDescBuff_info_work.range          = VK_WHOLE_SIZE;

	// --- Keep the original vkMapMemory call ---
	void *pvoid_memmap_work;
	vr("vkMapMemory", &vkres, pvoid_memmap_work,
		vkMapMemory(vob.VKL, vkdevmem_ub_work, vkDescBuff_info_work.offset, vkDescBuff_info_work.range, 0, &pvoid_memmap_work) );

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "SSBO");	/**/
	///////////////////////////////////////////////////


	VkDeviceSize vkdevsize_ssbo;
	vkdevsize_ssbo = sizeof(UB32_64) * 16;
	ov("UB32_64 * 16 size", vkdevsize_ssbo);

	VK_Buffer_Data work_ssbo_data; // Struct to hold results
	createBuffer(
		vob.VKL,                    // Logical device
		vob.VKP,                    // Physical device
		vkdevsize_ssbo,             // Size of the buffer
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, // Usage flags
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // Memory properties
		VK_SHARING_MODE_EXCLUSIVE,  // Sharing mode
		&vob.VKQ_i,                 // Queue family indices
		1,                          // Queue family index count
		&work_ssbo_data,            // Output struct
		&vkres);                    // Result vector

	// --- Keep the original descriptor info setup ---
	VkBuffer vkbuff_ssbo = work_ssbo_data.vk_buffer; // Get the buffer handle
	VkDeviceMemory vkdevmem_sb_work = work_ssbo_data.vk_dev_mem; // Get the memory handle

    VkDescriptorBufferInfo vkDescBuff_info_ssbo;
        vkDescBuff_info_ssbo.buffer         = vkbuff_ssbo;
        vkDescBuff_info_ssbo.offset         = 0;
        vkDescBuff_info_ssbo.range          = VK_WHOLE_SIZE;

	// --- Keep the original vkMapMemory call ---
	void *pvoid_memmap_ssbo;
	vr("vkMapMemory", &vkres, pvoid_memmap_ssbo,
		vkMapMemory(vob.VKL, vkdevmem_sb_work, vkDescBuff_info_ssbo.offset, vkDescBuff_info_ssbo.range, 0, &pvoid_memmap_ssbo) );

	// Perform bulk update of descriptor sets after all resources are ready
	updateWorkDescriptorSets(
		vob.VKL,                        // Logical device
		&dsl_work,                      // Struct containing the allocated descriptor sets
		rpass_info.vk_sampler,          // Sampler handle
		work_init,                      // Array of work image views (work_init[0], work_init[1])
		vkbuff_work,                    // Uniform buffer handle
		vkbuff_ssbo,                    // Storage buffer handle
		2);                             // Number of sets to update

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "WORK RENDER PASS");		/**/
	///////////////////////////////////////////////////

	VK_RenderPass rp_work; // Keep declaration

	createSimpleColorRenderPass(
		vob.VKL,                             // Logical device
		work.img_info[0].format,             // Format from work image
		VK_ATTACHMENT_LOAD_OP_LOAD,          // LoadOp
		VK_ATTACHMENT_STORE_OP_STORE,        // StoreOp
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, // InitialLayout
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, // FinalLayout
		&rp_work,                            // Output struct
		&vkres);                             // Result vector

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "WORK FRAMEBUFFER");		/**/
	///////////////////////////////////////////////////

	VK_FrameBuff fb_work[2]; // Keep declaration

	for(int i = 0; i < 2; i++) {
		createFramebuffer(
			vob.VKL,                    // Logical device
			rp_work.vk_render_pass,     // Render pass for work
			work_init[i].vk_image_view, // Corresponding work image view
			APP_W,                      // Width
			APP_H,                      // Height
			&fb_work[i],                // Output struct for this framebuffer
			&vkres);                    // Result vector
	}

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "WORK PIPELINE");			/**/
	///////////////////////////////////////////////////

	VK_Pipe pipe_work; // Keep declaration

	// Create Pipeline Layout
	createPipelineLayout(
		vob.VKL,                    // Logical device
		dsl_work.vk_desc_set_layout,// Descriptor set layout for work pipeline
		&pipe_work,                 // Output struct (stores layout handle)
		&vkres);                    // Result vector

	// Create Graphics Pipeline
	createWorkGraphicsPipeline(
		vob.VKL,                    // Logical device
		&pipe_work,                 // Input: layout handle. Output: pipeline handle.
		&pipe_info,                 // Struct containing pipeline state config
		rp_work.vk_render_pass,     // Render pass for work pipeline
		&vkres);                    // Result vector

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RECORD WORK LOOP");		/**/
	///////////////////////////////////////////////////

	VkRenderPassBeginInfo vkrpbegininfo_work[2];
	for(int i = 0; i < 2; i++) {
		vkrpbegininfo_work[i].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegininfo_work[i].pNext 				= NULL;
		vkrpbegininfo_work[i].renderPass 			= rp_work.vk_render_pass;
		vkrpbegininfo_work[i].framebuffer 			= fb_work[i].vk_framebuffer;
		vkrpbegininfo_work[i].renderArea 			= rpass_info.rect2D;
		vkrpbegininfo_work[i].clearValueCount 		= 1;
		vkrpbegininfo_work[i].pClearValues 			= &rpass_info.clear_val; }

	for(int i = 0; i < 2; i++) {
		vr("vkBeginCommandBuffer", &vkres, i,
			vkBeginCommandBuffer(combuf_work_loop[i].vk_command_buffer, &combuf_work_loop[i].comm_buff_begin_info) );

			rv("vkCmdBeginRenderPass");
				vkCmdBeginRenderPass (
					combuf_work_loop[i].vk_command_buffer, &vkrpbegininfo_work[i], VK_SUBPASS_CONTENTS_INLINE );

				rv("vkCmdBindPipeline");
					vkCmdBindPipeline (
						combuf_work_loop[i].vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_work.vk_pipeline );

				rv("vkCmdBindDescriptorSets");
					vkCmdBindDescriptorSets (
						combuf_work_loop[i].vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_work.vk_pipeline_layout,
						0, 1, &dsl_work.vk_descriptor_set[i], 0, NULL );

				rv("vkCmdDraw");
					vkCmdDraw (
						combuf_work_loop[i].vk_command_buffer, 3, 1, 0, 0 );

			rv("vkCmdEndRenderPass");
				vkCmdEndRenderPass(combuf_work_loop[i].vk_command_buffer);

		vr("vkEndCommandBuffer", &vkres, i,
			vkEndCommandBuffer(combuf_work_loop[i].vk_command_buffer) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "IMGUI RENDER PASS");		/**/
	///////////////////////////////////////////////////

	VK_RenderPass rp_imgui; // Keep declaration

	createSimpleColorRenderPass(
		vob.VKL,                               // Logical device
		VK_FORMAT_B8G8R8A8_UNORM,              // Format for swapchain/ImGui
		VK_ATTACHMENT_LOAD_OP_LOAD,            // LoadOp
		VK_ATTACHMENT_STORE_OP_STORE,          // StoreOp
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,       // InitialLayout (from presentation)
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // FinalLayout (ready for ImGui draw)
		&rp_imgui,                             // Output struct
		&vkres);                               // Result vector

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "IMGUI FRAMEBUFFER");		/**/
	///////////////////////////////////////////////////

	VK_ImageView vk_imgview_imgui[swap_image_count]; // Keep declaration

	for(int i = 0; i < swap_image_count; i++) {
		createImageView(
			vob.VKL,                        // Logical device
			vk_image_swapimgs_vec[i],           // Source image handle from swapchain images array
			VK_FORMAT_B8G8R8A8_UNORM,       // Format (matches ImGui render pass)
			VK_IMAGE_ASPECT_COLOR_BIT,      // Aspect flags
			&vk_imgview_imgui[i],           // Output struct for this view
			&vkres);                        // Result vector
	}

	VK_FrameBuff fb_imgui[swap_image_count]; // Keep declaration

	for(int i = 0; i < swap_image_count; i++) {
		createFramebuffer(
			vob.VKL,                        // Logical device
			rp_imgui.vk_render_pass,        // Render pass for ImGui
			vk_imgview_imgui[i].vk_image_view, // Corresponding ImGui image view
			APP_W,                          // Width
			APP_H,                          // Height
			&fb_imgui[i],                   // Output struct for this framebuffer
			&vkres);                        // Result vector
	}

	VkRenderPassBeginInfo vkrpbegininfo_imgui[swap_image_count];
	for(int i = 0; i < swap_image_count; i++) {
		vkrpbegininfo_imgui[i].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkrpbegininfo_imgui[i].pNext 				= NULL;
		vkrpbegininfo_imgui[i].renderPass 			= rp_imgui.vk_render_pass;
		vkrpbegininfo_imgui[i].framebuffer 			= fb_imgui[i].vk_framebuffer;
		vkrpbegininfo_imgui[i].renderArea 			= rpass_info.rect2D;
		vkrpbegininfo_imgui[i].clearValueCount 		= 1;
		vkrpbegininfo_imgui[i].pClearValues 		= &rpass_info.clear_val; }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "BARRIERS");				/**/
	///////////////////////////////////////////////////

	// Are all of these used? TODO

	VkImageMemoryBarrier vk_IMB_blit_TSO_to_TDO;
		vk_IMB_blit_TSO_to_TDO.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_blit_TSO_to_TDO.pNext 					= NULL;
		vk_IMB_blit_TSO_to_TDO.srcAccessMask 			= 0;
		vk_IMB_blit_TSO_to_TDO.dstAccessMask 			= 0;
		vk_IMB_blit_TSO_to_TDO.oldLayout 				= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vk_IMB_blit_TSO_to_TDO.newLayout 				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		vk_IMB_blit_TSO_to_TDO.srcQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_blit_TSO_to_TDO.dstQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_blit_TSO_to_TDO.image 					= blit.vk_image;
		vk_IMB_blit_TSO_to_TDO.subresourceRange 		= rpass_info.img_subres_range;

	VkImageMemoryBarrier vk_IMB_blit_TDO_to_TSO;
		vk_IMB_blit_TDO_to_TSO.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_blit_TDO_to_TSO.pNext 					= NULL;
		vk_IMB_blit_TDO_to_TSO.srcAccessMask 			= 0;
		vk_IMB_blit_TDO_to_TSO.dstAccessMask 			= 0;
		vk_IMB_blit_TDO_to_TSO.oldLayout 				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		vk_IMB_blit_TDO_to_TSO.newLayout 				= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vk_IMB_blit_TDO_to_TSO.srcQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_blit_TDO_to_TSO.dstQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_blit_TDO_to_TSO.image 					= blit.vk_image;
		vk_IMB_blit_TDO_to_TSO.subresourceRange 		= rpass_info.img_subres_range;

	VkImageMemoryBarrier vk_IMB_blit_imagedata_UND_to_TDO;
		vk_IMB_blit_imagedata_UND_to_TDO.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_blit_imagedata_UND_to_TDO.pNext 					= NULL;
		vk_IMB_blit_imagedata_UND_to_TDO.srcAccessMask 			= 0;
		vk_IMB_blit_imagedata_UND_to_TDO.dstAccessMask 			= 0;
		vk_IMB_blit_imagedata_UND_to_TDO.oldLayout 				= VK_IMAGE_LAYOUT_UNDEFINED;
		vk_IMB_blit_imagedata_UND_to_TDO.newLayout 				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		vk_IMB_blit_imagedata_UND_to_TDO.srcQueueFamilyIndex 	= vob.VKQ_i;
		vk_IMB_blit_imagedata_UND_to_TDO.dstQueueFamilyIndex 	= vob.VKQ_i;
		vk_IMB_blit_imagedata_UND_to_TDO.image 					= blit.vk_image;
		vk_IMB_blit_imagedata_UND_to_TDO.subresourceRange 		= rpass_info.img_subres_range;

	VkImageMemoryBarrier vk_IMB_pres_CAO_to_PRS[swap_image_count];
	for(int i = 0; i < swap_image_count; i++) {
		vk_IMB_pres_CAO_to_PRS[i].sType 					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_pres_CAO_to_PRS[i].pNext 					= NULL;
		vk_IMB_pres_CAO_to_PRS[i].srcAccessMask 			= 0;
		vk_IMB_pres_CAO_to_PRS[i].dstAccessMask 			= 0;
		vk_IMB_pres_CAO_to_PRS[i].oldLayout 				= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		vk_IMB_pres_CAO_to_PRS[i].newLayout 				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		vk_IMB_pres_CAO_to_PRS[i].srcQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_pres_CAO_to_PRS[i].dstQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_pres_CAO_to_PRS[i].image 					= vk_image_swapimgs_vec[i];
		vk_IMB_pres_CAO_to_PRS[i].subresourceRange 			= rpass_info.img_subres_range; }

	VkImageMemoryBarrier vk_IMB_work_SRO_to_TSO[2];
	for(int i = 0; i < 2; i++) {
		vk_IMB_work_SRO_to_TSO[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_work_SRO_to_TSO[i].pNext 					= NULL;
		vk_IMB_work_SRO_to_TSO[i].srcAccessMask 			= 0;
		vk_IMB_work_SRO_to_TSO[i].dstAccessMask 			= 0;
		vk_IMB_work_SRO_to_TSO[i].oldLayout 				= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vk_IMB_work_SRO_to_TSO[i].newLayout 				= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vk_IMB_work_SRO_to_TSO[i].srcQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_work_SRO_to_TSO[i].dstQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_work_SRO_to_TSO[i].image 					= work.vk_image[i];
		vk_IMB_work_SRO_to_TSO[i].subresourceRange 			= rpass_info.img_subres_range; }

	VkImageMemoryBarrier vk_IMB_work_TSO_to_SRO[2];
	for(int i = 0; i < 2; i++) {
		vk_IMB_work_TSO_to_SRO[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_work_TSO_to_SRO[i].pNext 					= NULL;
		vk_IMB_work_TSO_to_SRO[i].srcAccessMask 			= 0;
		vk_IMB_work_TSO_to_SRO[i].dstAccessMask 			= 0;
		vk_IMB_work_TSO_to_SRO[i].oldLayout 				= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vk_IMB_work_TSO_to_SRO[i].newLayout 				= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vk_IMB_work_TSO_to_SRO[i].srcQueueFamilyIndex 	= vob.VKQ_i;
		vk_IMB_work_TSO_to_SRO[i].dstQueueFamilyIndex 	= vob.VKQ_i;
		vk_IMB_work_TSO_to_SRO[i].image 					= work.vk_image[i];
		vk_IMB_work_TSO_to_SRO[i].subresourceRange 		= rpass_info.img_subres_range; }

	VkImageMemoryBarrier vk_IMB_blit_imagedata_TDO_to_TSO;
		vk_IMB_blit_imagedata_TDO_to_TSO.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_blit_imagedata_TDO_to_TSO.pNext 					= NULL;
		vk_IMB_blit_imagedata_TDO_to_TSO.srcAccessMask 			= 0;
		vk_IMB_blit_imagedata_TDO_to_TSO.dstAccessMask 			= 0;
		vk_IMB_blit_imagedata_TDO_to_TSO.oldLayout 				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		vk_IMB_blit_imagedata_TDO_to_TSO.newLayout 				= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vk_IMB_blit_imagedata_TDO_to_TSO.srcQueueFamilyIndex 	= vob.VKQ_i;
		vk_IMB_blit_imagedata_TDO_to_TSO.dstQueueFamilyIndex 	= vob.VKQ_i;
		vk_IMB_blit_imagedata_TDO_to_TSO.image 					= blit.vk_image;
		vk_IMB_blit_imagedata_TDO_to_TSO.subresourceRange 		= rpass_info.img_subres_range;

	VkImageMemoryBarrier vk_IMB_blit_imagedata_TSO_to_TDO;
		vk_IMB_blit_imagedata_TSO_to_TDO.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_blit_imagedata_TSO_to_TDO.pNext 					= NULL;
		vk_IMB_blit_imagedata_TSO_to_TDO.srcAccessMask 			= 0;
		vk_IMB_blit_imagedata_TSO_to_TDO.dstAccessMask 			= 0;
		vk_IMB_blit_imagedata_TSO_to_TDO.oldLayout 				= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vk_IMB_blit_imagedata_TSO_to_TDO.newLayout 				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		vk_IMB_blit_imagedata_TSO_to_TDO.srcQueueFamilyIndex 	= vob.VKQ_i;
		vk_IMB_blit_imagedata_TSO_to_TDO.dstQueueFamilyIndex 	= vob.VKQ_i;
		vk_IMB_blit_imagedata_TSO_to_TDO.image 					= blit.vk_image;
		vk_IMB_blit_imagedata_TSO_to_TDO.subresourceRange 		= rpass_info.img_subres_range;

	VkImageMemoryBarrier vk_IMB_pres_UND_to_PRS[swap_image_count];
	for(int i = 0; i < swap_image_count; i++) {
		vk_IMB_pres_UND_to_PRS[i].sType 					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_pres_UND_to_PRS[i].pNext 					= NULL;
		vk_IMB_pres_UND_to_PRS[i].srcAccessMask 			= 0;
		vk_IMB_pres_UND_to_PRS[i].dstAccessMask 			= 0;
		vk_IMB_pres_UND_to_PRS[i].oldLayout 				= VK_IMAGE_LAYOUT_UNDEFINED;
		vk_IMB_pres_UND_to_PRS[i].newLayout 				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		vk_IMB_pres_UND_to_PRS[i].srcQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_pres_UND_to_PRS[i].dstQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_pres_UND_to_PRS[i].image 					= vk_image_swapimgs_vec[i];
		vk_IMB_pres_UND_to_PRS[i].subresourceRange 			= rpass_info.img_subres_range; }

	VkImageMemoryBarrier vk_IMB_pres_PRS_to_TDO[swap_image_count];
	for(int i = 0; i < swap_image_count; i++) {
		vk_IMB_pres_PRS_to_TDO[i].sType 					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_pres_PRS_to_TDO[i].pNext 					= NULL;
		vk_IMB_pres_PRS_to_TDO[i].srcAccessMask 			= 0;
		vk_IMB_pres_PRS_to_TDO[i].dstAccessMask 			= 0;
		vk_IMB_pres_PRS_to_TDO[i].oldLayout 				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		vk_IMB_pres_PRS_to_TDO[i].newLayout 				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		vk_IMB_pres_PRS_to_TDO[i].srcQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_pres_PRS_to_TDO[i].dstQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_pres_PRS_to_TDO[i].image 					= vk_image_swapimgs_vec[i];
		vk_IMB_pres_PRS_to_TDO[i].subresourceRange 			= rpass_info.img_subres_range; }

	VkImageMemoryBarrier vk_IMB_pres_TDO_to_PRS[swap_image_count];
	for(int i = 0; i < swap_image_count; i++) {
		vk_IMB_pres_TDO_to_PRS[i].sType 					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_pres_TDO_to_PRS[i].pNext 					= NULL;
		vk_IMB_pres_TDO_to_PRS[i].srcAccessMask 			= 0;
		vk_IMB_pres_TDO_to_PRS[i].dstAccessMask 			= 0;
		vk_IMB_pres_TDO_to_PRS[i].oldLayout 				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		vk_IMB_pres_TDO_to_PRS[i].newLayout 				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		vk_IMB_pres_TDO_to_PRS[i].srcQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_pres_TDO_to_PRS[i].dstQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_pres_TDO_to_PRS[i].image 					= vk_image_swapimgs_vec[i];
		vk_IMB_pres_TDO_to_PRS[i].subresourceRange 			= rpass_info.img_subres_range; }

	VkImageMemoryBarrier vk_IMB_swap_PRS_to_TSO[swap_image_count];
	for(int i = 0; i < swap_image_count; i++) {
		vk_IMB_swap_PRS_to_TSO[i].sType 					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_swap_PRS_to_TSO[i].pNext 					= NULL;
		vk_IMB_swap_PRS_to_TSO[i].srcAccessMask 			= 0;
		vk_IMB_swap_PRS_to_TSO[i].dstAccessMask 			= 0;
		vk_IMB_swap_PRS_to_TSO[i].oldLayout 				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		vk_IMB_swap_PRS_to_TSO[i].newLayout 				= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vk_IMB_swap_PRS_to_TSO[i].srcQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_swap_PRS_to_TSO[i].dstQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_swap_PRS_to_TSO[i].image 					= vk_image_swapimgs_vec[i];
		vk_IMB_swap_PRS_to_TSO[i].subresourceRange 			= rpass_info.img_subres_range; }

	VkImageMemoryBarrier vk_IMB_swap_TSO_to_PRS[swap_image_count];
	for(int i = 0; i < swap_image_count; i++) {
		vk_IMB_swap_TSO_to_PRS[i].sType 					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vk_IMB_swap_TSO_to_PRS[i].pNext 					= NULL;
		vk_IMB_swap_TSO_to_PRS[i].srcAccessMask 			= 0;
		vk_IMB_swap_TSO_to_PRS[i].dstAccessMask 			= 0;
		vk_IMB_swap_TSO_to_PRS[i].oldLayout 				= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vk_IMB_swap_TSO_to_PRS[i].newLayout 				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		vk_IMB_swap_TSO_to_PRS[i].srcQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_swap_TSO_to_PRS[i].dstQueueFamilyIndex 		= vob.VKQ_i;
		vk_IMB_swap_TSO_to_PRS[i].image 					= vk_image_swapimgs_vec[i];
		vk_IMB_swap_TSO_to_PRS[i].subresourceRange 			= rpass_info.img_subres_range; }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RECORD IMAGEDATA INIT");	/**/
	///////////////////////////////////////////////////

	for(int i = 0; i < 1; i++) {
		vr("vkBeginCommandBuffer", &vkres, i,
			vkBeginCommandBuffer(combuf_work_imagedata_init[i].vk_command_buffer, &combuf_work_imagedata_init[i].comm_buff_begin_info) );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_work_imagedata_init[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_blit_imagedata_UND_to_TDO );

		vr("vkEndCommandBuffer", &vkres, i,
			vkEndCommandBuffer(combuf_work_imagedata_init[i].vk_command_buffer) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "SUBMIT IMAGEDATA INIT");	/**/
	///////////////////////////////////////////////////

	for(int i = 0; i < 1; i++) {
		if(valid) {
			rv("vkcombuf_work_init");
				qsync.sub_info.pCommandBuffers = &combuf_work_imagedata_init[i].vk_command_buffer;
			vr("vkQueueSubmit", &vkres, i,
				vkQueueSubmit(qsync.vk_queue, 1, &qsync.sub_info, VK_NULL_HANDLE) ); } }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RECORD IMAGEDATA OUT");	/**/
	///////////////////////////////////////////////////

	for(int i = 0; i < 2; i++) {
		vr("vkBeginCommandBuffer", &vkres, i,
			vkBeginCommandBuffer(combuf_work_imagedata[i].vk_command_buffer, &combuf_work_imagedata[i].comm_buff_begin_info) );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_work_imagedata[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_work_SRO_to_TSO[i] );

			rv("vkCmdBlitImage");
				vkCmdBlitImage (
					combuf_work_imagedata[i].vk_command_buffer, 
					work.vk_image[i], 			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					blit.vk_image, 				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1, &rpass_info.img_blit, 	VK_FILTER_NEAREST );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_work_imagedata[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_work_TSO_to_SRO[i] );

		vr("vkEndCommandBuffer", &vkres, i,
			vkEndCommandBuffer(combuf_work_imagedata[i].vk_command_buffer) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RECORD BLIT IMGUI OUT");	/**/
	///////////////////////////////////////////////////

	for(int i = 0; i < swap_image_count; i++) {
		vr("vkBeginCommandBuffer", &vkres, i,
			vkBeginCommandBuffer(combuf_blit_imgui_loop[i].vk_command_buffer, &combuf_blit_imgui_loop[i].comm_buff_begin_info) );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_blit_imgui_loop[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_swap_PRS_to_TSO[i] );

			rv("vkCmdBlitImage");
				vkCmdBlitImage (
					combuf_blit_imgui_loop[i].vk_command_buffer, 
					vk_image_swapimgs_vec[i], 		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					blit.vk_image, 				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1, &rpass_info.img_blit, 	VK_FILTER_NEAREST );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_blit_imgui_loop[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_swap_TSO_to_PRS[i] );

		vr("vkEndCommandBuffer", &vkres, i,
			vkEndCommandBuffer(combuf_blit_imgui_loop[i].vk_command_buffer) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RECORD BLIT2BUFF");		/**/
	///////////////////////////////////////////////////

	for(int i = 0; i < 1; i++) {
		vr("vkBeginCommandBuffer", &vkres, i,
			vkBeginCommandBuffer(combuf_blit2buff_sing[i].vk_command_buffer, &combuf_blit2buff_sing[i].comm_buff_begin_info) );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_blit2buff_sing[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_blit_TDO_to_TSO );

			rv("vkCmdCopyImageToBuffer");
				vkCmdCopyImageToBuffer (
					combuf_blit2buff_sing[i].vk_command_buffer, 
					blit.vk_image, 				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					blit2buff_data.vk_buffer,
					1, &rpass_info.buffer_img_cpy );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_blit2buff_sing[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_blit_TSO_to_TDO );

		vr("vkEndCommandBuffer", &vkres, i,
			vkEndCommandBuffer(combuf_blit2buff_sing[i].vk_command_buffer) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RECORD PRES INIT");		/**/
	///////////////////////////////////////////////////

	for(int i = 0; i < swap_image_count; i++) {
		vr("vkBeginCommandBuffer", &vkres, i,
			vkBeginCommandBuffer(combuf_pres_init[i].vk_command_buffer, &combuf_pres_init[i].comm_buff_begin_info) );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_pres_init[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_pres_UND_to_PRS[i] );

		vr("vkEndCommandBuffer", &vkres, i,
			vkEndCommandBuffer(combuf_pres_init[i].vk_command_buffer) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "SUBMIT PRES INIT");		/**/
	///////////////////////////////////////////////////

	for(int i = 0; i < swap_image_count; i++) {
		if(valid) {
			rv("vkcombuf_pres_init");
				qsync.sub_info.pCommandBuffers = &combuf_pres_init[i].vk_command_buffer;
			vr("vkQueueSubmit", &vkres, i,
				vkQueueSubmit(qsync.vk_queue, 1, &qsync.sub_info, VK_NULL_HANDLE) ); } }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "RECORD PRES LOOP");		/**/
	///////////////////////////////////////////////////

	// Split function into two, so that it can sync with the two "work" frames
	for(int i = 0; i < swap_image_count*2; i++) {
		vr("vkBeginCommandBuffer", &vkres, i,
			vkBeginCommandBuffer(combuf_pres_loop[i].vk_command_buffer, &combuf_pres_loop[i].comm_buff_begin_info) );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_pres_loop[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_pres_PRS_to_TDO[i%swap_image_count] );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_pres_loop[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_work_SRO_to_TSO[i/swap_image_count] );

			rv("vkCmdBlitImage");
				vkCmdBlitImage (
					combuf_pres_loop[i].vk_command_buffer, 
					work.vk_image[i/swap_image_count], 			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					vk_image_swapimgs_vec[i%swap_image_count], 		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1, &rpass_info.img_blit, 	VK_FILTER_NEAREST );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_pres_loop[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_work_TSO_to_SRO[i/swap_image_count] );

			rv("vkCmdPipelineBarrier");
				vkCmdPipelineBarrier (
					combuf_pres_loop[i].vk_command_buffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &vk_IMB_pres_TDO_to_PRS[i%swap_image_count] );

		vr("vkEndCommandBuffer", &vkres, i,
			vkEndCommandBuffer(combuf_pres_loop[i].vk_command_buffer) ); }

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "SWAPCHAIN PRESENT INFO");	/**/
	///////////////////////////////////////////////////

	VkPresentInfoKHR vk_present_info;
		vk_present_info.sType 					= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		vk_present_info.pNext 					= NULL;
		vk_present_info.waitSemaphoreCount 		= 1;
		vk_present_info.pWaitSemaphores 		= &vk_semaphore_swapchain_pres;
		vk_present_info.swapchainCount 			= 1;
		vk_present_info.pSwapchains 			= &vk_swapchain;
		vk_present_info.pImageIndices 			= &swap_image_index;
		vk_present_info.pResults 				= NULL;

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "DEAR IMGUI");				/**/
	///////////////////////////////////////////////////

	initImGui(
		glfw_W,                     // GLFW window handle
		&vob,                       // Core Vulkan objects
		qsync.vk_queue,             // Graphics queue
		rp_imgui.vk_render_pass,    // ImGui render pass
		vk_surface_capabilities.minImageCount, // Min swapchain image count
		swap_image_count,           // Actual swapchain image count
		&ei,                        // Engine info (for headless check)
		&vkres                      // Result vector
	);

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "MAIN LOOP INIT");			/**/
	///////////////////////////////////////////////////

	uint32_t 	frame_index 	= 0;	// Loop Frame Index
	uint32_t 	work_index 		= 0;	// Work Image Generation Frame Index
//	uint32_t	imgdat_idx		= 0;	// Export Image Data Index

//	Timers
	NS_Timer 	ftime;
	NS_Timer 	optime;
	NS_Timer 	prstime;
	NS_Timer 	uitime;
	NS_Timer 	guitime;
	NS_Timer 	cmdtime;
	int  		current_sec		= time(0);
	int  		fps_freq 		= 1;
	int  		fps_report 		= time(0) - fps_freq;


	UB32_64 pcd 	= new_PCD_256();

//	Uniform Buffer Object ( 64 * 32 bits maximum )
	UB32_64 ub;
	SB_4096 sb;

	update_ub( &pcd, &ub );

	UI_info ui;
		ui.mx 	= 0;
		ui.my 	= 0;
		ui.mbl 	= 0;
		ui.mbr 	= 0;
		ui.cmd 	= 0;

	FT_info ft;
		ft.frame = 0;
		ft.seed	 = INIT_TIME%256;

	VW_info vw;
		vw.pmap = 0;
		vw.sdat	= 0;

	IMGUI_Config gc;
		gc.load_shader 					= false;
		gc.load_pattern 				= false;
		gc.load_pattern_confirm 		= false;
		gc.load_pattern_check_instant 	=  true;
		gc.load_pattern_check_reseed 	=  true;
		gc.load_pattern_random 			= false;
		gc.save_to_archive				= false;
		gc.mutate_menu 					= false;
		gc.mutate_full_random 			= false;
		gc.mutate_backstep				= false;
		gc.mutate_backstep_retry		= false;
		gc.mutate_flip 					= false;
		gc.throttle_menu 				= false;
		gc.throttle_enabled 			= false;
		gc.mode_planar					=  true;
		gc.mode_linear 					= false;
		gc.mode_circular 				= false;
		gc.mode_showdata 				= false;
		gc.scale_zoom_menu				= false;
		gc.scale_update					= false;
		gc.zoom_update					= false;
		gc.glfw_mod_LCTRL				= false;
		gc.glfw_mod_LSHIFT				= false;
		gc.show_notification_float		= false;
		gc.scale_has_panned				= false;
		gc.recording_config 			= false;
		gc.record_imgui					=  true;
		gc.load_A256_confirm 			=  true;

		gc.load_pattern_last_value 		= ei.load_pattern;
		gc.mutate_flip_str 				= 80;
		gc.mutate_backstep_idx 			= -1;
		gc.mutate_backstep_last_value 	= gc.mutate_backstep_idx;
		gc.throttle_target				= 32;
		gc.pmap_index					= 0;
		gc.pmap_index_last				= gc.pmap_index;
		gc.glfw_mouse_xpos_last			= ui.mx;
		gc.load_A256_index				= -1;
		gc.load_A256_index_last			= gc.load_A256_index;
		gc.load_A256_count				= get_PCD256_count("sav/PCD256_archive.vkpat");

		gc.notification_float_value		= 0.0f;

		memcpy(&gc.scale_value, &pcd.u32[62], sizeof(uint32_t));
		gc.scale_last_value				= gc.scale_value;

		memcpy(&gc.zoom_value, &pcd.u32[61], sizeof(uint32_t));
		gc.zoom_last_value				= gc.zoom_value;

	NS_Timer nottime;
		gc.notification_timer = nottime;
		send_notif(0, &gc);

	fspec256 fs;
	fsmag256 fsm = new_fsmag256();

	bool do_ub_update = true;

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "MAIN LOOP");				/**/
	///////////////////////////////////////////////////

//	Main Loop code
	do {
	//	Record loop start time
    	ftime = start_timer(ftime);

	//	Reset shader commands
		if(!ei.paused && ei.tick_loop == 0) { ui.cmd = 0; }
		clear_glfw_key(&glfw_key);
		clear_glfw_mouse(&glfw_mouse);

	//	Timing (seconds) setup
		current_sec = time(0);
		if( current_sec - fps_report >= fps_freq + 1) { fps_report = current_sec; }

	//	'Render'
		if(valid) {

		//	Lower the workload / FPS if paused
			if(ei.paused) { framesleep( 1000/60 ); } else {
			if(gc.throttle_enabled) { framesleep( gc.throttle_target ); } }

			if(!ei.run_headless) {
			//	Poll for GLFW window events
				glfwPollEvents();

			//	Does IMGUI want to capture input?
				ImGuiIO& io = ImGui::GetIO();

				kc.has_keyboard = io.WantCaptureKeyboard;
				kc.has_mouse 	= io.WantCaptureMouse;

				glfwSetKeyCallback( glfw_W, glfw_keyboard_event	);

				if(!ei.show_gui) { kc.has_mouse = false; kc.has_keyboard = false; }

				if( !kc.has_mouse ) {

					glfwSetCursorPosCallback 	( glfw_W, glfw_mousemove_event 	 );
					glfwSetMouseButtonCallback	( glfw_W, glfw_mouseclick_event	 );
					glfwSetScrollCallback		( glfw_W, glfw_mousescroll_event );
					ui.mx = glfw_mouse.xpos;
					ui.my = glfw_mouse.ypos;

				//	MBR: Scale Panning
					if( gc.glfw_mouse_xpos_last != glfw_mouse.xpos
					&& 	glfw_mouse.button 		== 1
					&&	glfw_mouse.action 		!= 0
					&& 	gc.glfw_mod_LSHIFT ) {
								gc.scale_has_panned		= true;
						int 	mx_offset 				= glfw_mouse.xpos - gc.glfw_mouse_xpos_last;
						float 	xscale	 				= float((APP_W / 2.0) - float(mx_offset)) / float(APP_W);
								gc.scale_value 			= gc.scale_value + gc.scale_value * ((xscale - 0.5) * 2.0) * (1.0 / (1.0 + gc.zoom_value));
								gc.scale_update			= true;
						send_notif_float(15, gc.scale_value, &gc); }

					gc.glfw_mouse_xpos_last	= glfw_mouse.xpos;

				//	Left Click
					if( glfw_mouse.button == 0 ) { ui.mbl = glfw_mouse.action; }

				//	Right Click
					if( glfw_mouse.button == 1 ) { ui.mbr = glfw_mouse.action; }
					if( glfw_mouse.button == 1
					&&  gc.glfw_mod_LSHIFT ) 	 { ui.mbr = 0; }

				//	Middle Mouse / ScrollWheel Click
					if( glfw_mouse.button == 2
					&&	glfw_mouse.action == 1 ) {
						glfw_mouse.action  				= 0;
						gc.mutate_backstep_last_value 	= gc.mutate_backstep_idx;
						do_action(  9, &ui, &ei, &gc );
						if( gc.glfw_mod_LSHIFT ) { 
							gc.save_to_archive = true;
							gc.load_A256_index = -1; } }

				//	Mouse Wheel Up
					if( glfw_mouse.yoffset == -1 ) {
						glfw_mouse.yoffset  =  0;
						do_action( 36, &ui, &ei, &gc ); }

				//	Mouse Wheel Down
					if( glfw_mouse.yoffset ==  1 ) {
						glfw_mouse.yoffset  =  0;
						do_action( 35, &ui, &ei, &gc ); }

				//	Mouse Back
					if(glfw_mouse.button == 3 && glfw_mouse.action == 1) {
						glfw_mouse.action = 0;
						do_action( 38, &ui, &ei, &gc ); }

				//	Mouse Forward
					if(glfw_mouse.button == 4 && glfw_mouse.action == 1) {
						glfw_mouse.action = 0;
						do_action( 37, &ui, &ei, &gc ); } }

				else { ui.mbl = 0; ui.mbr = 0; }

				if( !kc.has_keyboard ) {

				//	Release 	L-Shift 	Reseed
					if( glfw_key.key 	== GLFW_KEY_LEFT_SHIFT 
					&&	glfw_key.action	== 0
					&& !gc.scale_has_panned ) { do_action( 11, &ui, &ei, &gc ); }

				//	Press 		L-Shift 	Sticky Modifier
					if( glfw_key.key 	== GLFW_KEY_LEFT_SHIFT
					&&	glfw_key.action	== 1 ) { gc.glfw_mod_LSHIFT = true; }

				//	Release 	L-Shift 	UnSticky Modifier
					if( glfw_key.key 	== GLFW_KEY_LEFT_SHIFT 
					&&	glfw_key.action	== 0 ) { gc.glfw_mod_LSHIFT = false; gc.scale_has_panned = false; }

				//	Press 		X 			Clear
					if( glfw_key.key 	== GLFW_KEY_X
		 			&& 	glfw_key.action >= 1 ) { do_action( 12, &ui, &ei, &gc ); }

				//	Press 		Z 			SymSeed
					if( glfw_key.key 	== GLFW_KEY_Z
		 			&& 	glfw_key.action >= 1 ) { do_action( 13, &ui, &ei, &gc ); }

				//	Press 		C 			BlendSeed
					if( glfw_key.key 	== GLFW_KEY_C
		 			&& 	glfw_key.action >= 1 ) { do_action( 33, &ui, &ei, &gc ); }

				//	Press 		TAB			Show Random Archive Pattern
					if( glfw_key.key 	== GLFW_KEY_TAB
		 			&& 	glfw_key.action == 1 ) { do_action( 39, &ui, &ei, &gc ); }

				//	Press 		RIGHT 		Show Prev Archive Pattern
					if( glfw_key.key 	== GLFW_KEY_RIGHT
		 			&& 	glfw_key.action >= 1 ) { do_action( 37, &ui, &ei, &gc ); }

				//	Press 		LEFT		Show Next Archive Pattern
					if( glfw_key.key 	== GLFW_KEY_LEFT
		 			&& 	glfw_key.action >= 1 ) { do_action( 38, &ui, &ei, &gc ); }

				//	Press 		CTRL-S		Save Archive Pattern
					if( glfw_key.key 	== GLFW_KEY_S
		 			&& (glfw_key.mods & GLFW_MOD_CONTROL)
		 			&& 	glfw_key.action == 1 ) { do_action(  7, &ui, &ei, &gc ); }

				//	Press 		T 			Toggle Throttle
					if( glfw_key.key 	== GLFW_KEY_T
		 			&& 	glfw_key.action >= 1 ) { do_action( 18, &ui, &ei, &gc ); }

				//	Press 		R 			Full Randomization
					if( glfw_key.key 	== GLFW_KEY_R
		 			&& 	glfw_key.action == 1 ) { do_action(  8, &ui, &ei, &gc ); }

				//	Press 		V 			Mutate Target
					if( glfw_key.key 	== GLFW_KEY_V
		 			&& 	glfw_key.action == 1 ) { do_action( 10, &ui, &ei, &gc ); }

				//	Press 		Q 			Reload Target
					if( glfw_key.key 	== GLFW_KEY_Q
		 			&& 	glfw_key.action == 1 ) { do_action( 24, &ui, &ei, &gc ); }

				//	Press 		KP_ENTER 	Toggle Recording
					if( glfw_key.key 	== GLFW_KEY_KP_ENTER
		 			&& 	glfw_key.action == 1 ) { do_action( 20, &ui, &ei, &gc ); }

				//	Press 		KP_ADD 		Increase Export Freq
					if( glfw_key.key 	== GLFW_KEY_KP_ADD
		 			&& 	glfw_key.action >= 1 ) { do_action( 26, &ui, &ei, &gc ); }

				//	Press 		KPSUBTRACT	Decrease Export Freq
					if( glfw_key.key 	== GLFW_KEY_KP_SUBTRACT
		 			&& 	glfw_key.action >= 1 ) { do_action( 27, &ui, &ei, &gc ); }

				//	Press 		1 			Planar Mapping
					if( glfw_key.key 	== GLFW_KEY_1
		 			&& 	glfw_key.action == 1 ) { do_action( 30, &ui, &ei, &gc ); do_action( 29, &ui, &ei, &gc ); }

				//	Press 		2 			Linear Mapping
					if( glfw_key.key 	== GLFW_KEY_2
		 			&& 	glfw_key.action == 1 ) { do_action( 31, &ui, &ei, &gc ); do_action( 29, &ui, &ei, &gc ); }

				//	Press 		3 			Circular Mapping
					if( glfw_key.key 	== GLFW_KEY_3
		 			&& 	glfw_key.action == 1 ) { do_action( 32, &ui, &ei, &gc ); do_action( 29, &ui, &ei, &gc ); }

				//	Press 		S 			Step[1]
					if( glfw_key.key 	== GLFW_KEY_S
		 			&& !(glfw_key.mods & GLFW_MOD_CONTROL)
		 			&& 	glfw_key.action >= 1 ) { do_action(  2, &ui, &ei, &gc ); }

				//	Press 		SPACE 		Toggle Pause
					if( glfw_key.key 	== GLFW_KEY_SPACE
		 			&& 	glfw_key.action == 1 ) { do_action(  1, &ui, &ei, &gc ); }

				//	Press 		ESCAPE 		Toggle IMGUI
					if( glfw_key.key 	== GLFW_KEY_ESCAPE
		 			&& 	glfw_key.action == 1 ) { do_action( 0, &ui, &ei, &gc ); } }

			//	Notifications
				if( gc.show_notification ) {
					uint32_t uint_notif_age =
						std::chrono::duration_cast<std::chrono::nanoseconds> (
							std::chrono::high_resolution_clock::now()
						- 	gc.notification_timer.st ).count();
					if( uint_notif_age < 1200000000 ) 	{ gc.notification_age = 1.0f - float(uint_notif_age) / float(1800000000); }
					if( uint_notif_age > 2400000000 ) 	{ tog(&gc.show_notification); } }

				if( gc.show_notification_float ) {
					uint32_t uint_notif_age =
						std::chrono::duration_cast<std::chrono::nanoseconds> (
							std::chrono::high_resolution_clock::now()
						- 	gc.notification_float_timer.st ).count();
					if( uint_notif_age < 1200000000 ) 	{ gc.notification_float_age = 1.0f - float(uint_notif_age) / float(1800000000); }
					if( uint_notif_age > 2400000000 ) 	{ tog(&gc.show_notification_float); } }


				if( gc.load_A256_confirm ) {
					gc.load_A256_confirm = false;
					ui.cmd = 1;
					ei.tick_loop = 1;
					pcd = load_PCD256("sav/PCD256_archive.vkpat", gc.load_A256_index);
					do_ub_update = true;
					memcpy(&gc.scale_value, &pcd.u32[62], sizeof(uint32_t));
					memcpy(&gc.zoom_value, &pcd.u32[61], sizeof(uint32_t));
					gc.scale_update = true;
					gc.zoom_update = true;
					for(int j = 0; j < 16; j++) {
						for(int i = 0; i < 48; i++) { sb.ub[j].u32[i] = pcd.u32[i]; }
					} }

			//	Load random Archive pattern
				if( gc.load_pattern_confirm ) {
					gc.load_pattern_confirm = false;
					if( gc.load_pattern_random ) {
						gc.load_pattern_random = false;
					//	Depreicated patterns under 17000
						ei.load_pattern = (rand()%(ei.PCD_count - 18080)) + 18080;	}
					loadPattern_PCD408_to_256( &ei, &pcd );
					do_ub_update = true;
					memcpy(&gc.scale_value, &pcd.u32[62], sizeof(uint32_t));
					memcpy(&gc.zoom_value, &pcd.u32[61], sizeof(uint32_t));
					if(gc.load_pattern_check_reseed) { ui.cmd = 1; ei.tick_loop = 1; } }

//
				if( gc.mutate_set_target ) {
					loglevel = MAXLOG;
					uint32_t panel_index = (uint32_t((float(ui.my)/float(APP_H))*float(panel_n_x_n))*panel_n_x_n) +(uint32_t((float(ui.mx)/float(APP_W))*float(panel_n_x_n)));
					ov("INDEX",panel_index);
					loglevel = -1;
					gc.mutate_set_target = false;
					for(int i = 0; i < 48; i++) { pcd.u32[i] = sb.ub[panel_index].u32[i]; }
					save_PCD256("sav/PCD256_global_all.vkpat", &pcd); }

				if( gc.mutate_backstep ) {
					gc.mutate_backstep = false;
					ui.cmd = 1;
					ei.tick_loop = 1;
					pcd = load_PCD256("sav/PCD256_global_all.vkpat", gc.mutate_backstep_idx);
					do_ub_update = true;
					memcpy(&gc.scale_value, &pcd.u32[62], sizeof(uint32_t));
					memcpy(&gc.zoom_value, &pcd.u32[61], sizeof(uint32_t));
					gc.scale_update = true;
					gc.zoom_update = true;
					for(int j = 0; j < 16; j++) {
						for(int i = 0; i < 48; i++) { sb.ub[j].u32[i] = pcd.u32[i]; }
					} }

				if( gc.mutate_full_random ) {
					gc.mutate_full_random = false;
					ui.cmd = 1;
					ei.tick_loop = 1;
					for(int i = 0; i < 48; i++) { pcd.u32[i] = mut_rnd(); }
					for(int j = 1; j < 16; j++) {
						for(int i = 0; i < 48; i++) { sb.ub[j].u32[i] = mut_rnd(); }
					}
					do_ub_update = true;
					save_PCD256("sav/PCD256_global_all.vkpat", &pcd); }

				if( gc.mutate_flip ) {
					gc.mutate_flip = false;
					ui.cmd = 1;
					ei.tick_loop = 1;

					for(int j = 1; j < 16; j++) {
						for(int i = 0; i < 48; i++) { sb.ub[j].u32[i] = bit_flp( pcd.u32[i], (rand()%(gc.mutate_flip_str*2))+(gc.mutate_flip_str/2)+1 ); }
					}

					for(int i = 0; i < 48; i++) { pcd.u32[i] = bit_flp( pcd.u32[i], (rand()%(gc.mutate_flip_str*2))+(gc.mutate_flip_str/2)+1 ); }

					for(int i = 0; i < 48; i++) {
						sb.ub[0].u32[i] = pcd.u32[i]; }

					do_ub_update = true;
					save_PCD256("sav/PCD256_global_all.vkpat", &pcd); }

				if( gc.mutate_backstep_retry ) {
					gc.mutate_backstep_retry = false;
					gc.mutate_backstep_idx--;
					gc.mutate_backstep_last_value = gc.mutate_backstep_idx; }

			//	Update special floats
				//	[62]	'Scale' value
				if( gc.scale_update ) {
					gc.scale_update = false;

					float fsc = gc.scale_value;
					memcpy(&pcd.u32[62], &fsc, sizeof(uint32_t));
					do_ub_update = true; }
				//	[61]	'Zoom' value
				if( gc.zoom_update ) {
					gc.zoom_update = false;
					float fzm = gc.zoom_value;// + gc.zoom_value * (float(sound) / float(INT16_MAX)) * 1.0f;
					memcpy(&pcd.u32[61], &fzm, sizeof(uint32_t));
					do_ub_update = true;	}

			//	Save current target to PCD256 Archive
				if( gc.save_to_archive ) {
					gc.save_to_archive = false;
					save_PCD256("sav/PCD256_archive.vkpat", &pcd);
					gc.load_A256_count = get_PCD256_count("sav/PCD256_archive.vkpat");
					send_notif(1, &gc); }

				if(ei.show_gui || ei.paused) {
					guitime = start_timer(guitime);
						ImGui_ImplVulkan_NewFrame();
						ImGui_ImplGlfw_NewFrame();
						ImGui::NewFrame();
						imgui_menu( glfw_W, &ui, &ei, &gc );
					//	ImGui::ShowDemoWindow();	// TODO TODO
						ImGui::Render();
					end_timer(guitime, "IMGUI Build Time");

					cmdtime = start_timer(cmdtime);
						for(int i = 0; i < swap_image_count; i++) {
							vr("vkBeginCommandBuffer", &vkres, i,
								vkBeginCommandBuffer(combuf_imgui_loop[i].vk_command_buffer, &combuf_imgui_loop[i].comm_buff_begin_info) );

								rv("vkCmdBeginRenderPass");
									vkCmdBeginRenderPass (
										combuf_imgui_loop[i].vk_command_buffer,
										&vkrpbegininfo_imgui[i%swap_image_count],
										VK_SUBPASS_CONTENTS_INLINE );

									ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), combuf_imgui_loop[i].vk_command_buffer);

								rv("vkCmdEndRenderPass");
									vkCmdEndRenderPass(combuf_imgui_loop[i].vk_command_buffer);

								rv("vkCmdPipelineBarrier");
									vkCmdPipelineBarrier (
										combuf_imgui_loop[i].vk_command_buffer,
										VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
										0, NULL, 0, NULL,
										1, &vk_IMB_pres_CAO_to_PRS[i%swap_image_count] );

							vr("vkEndCommandBuffer", &vkres, i,
								vkEndCommandBuffer(combuf_imgui_loop[i].vk_command_buffer) ); }
					end_timer(cmdtime, "Command Buffer Build Time"); } }

			if( do_ub_update ) { update_ub( &pcd, &ub ); update_ub( &ub, &sb.ub[0] ); }
		//	Update Uniform Buffer values
		//		[62]	'Scale' value
		//	if( gc.scale_update ) {
		//		gc.scale_update = false;
		//		memcpy(&pcd.u32[62], &gc.scale_value, sizeof(uint32_t));
		//		update_ub( &pcd, &ub );	}
		//		[61]	'Zoom' value
		//	if( gc.zoom_update ) {
		//		gc.zoom_update = false;
		//		memcpy(&pcd.u32[61], &gc.zoom_value, sizeof(uint32_t));
		//		update_ub( &pcd, &ub );	}
		//		[60]	Mouse info & Command IDs
			ub.u32[60] 		= pack_ui_info(ui);
		//		[59]	Views/Modes
			vw.pmap		= gc.pmap_index;
			vw.sdat		= (gc.mode_showdata) ? 1u : 0u;
			ub.u32[59] 	= pack_vw_info(vw);
			pcd.u32[59] = ub.u32[59];
		//		[63]	Frame Index, Time-Seed
			ft.frame 	= frame_index;
			ub.u32[63]	= pack_ft_info(ft);

		//	Send UB values to GPU
			rv("memcpy");
				memcpy(pvoid_memmap_work, &ub, sizeof(ub));


		//	Send SSBO values to GPU
//			sb.ub[0] = ub;
			rv("memcpy");
				memcpy(pvoid_memmap_ssbo, &sb, sizeof(sb));

    		optime = start_timer(optime);


			if(!ei.paused || ei.tick_loop) {
				if(valid) {
				//	Submit 'work' commands to the GPU graphics queue
					rv("vkcombuf_work");
						qsync.sub_info.pCommandBuffers = &combuf_work_loop[(frame_index+1)%2].vk_command_buffer;
					VkFence f = (ei.run_headless) ? qsync.vk_fence : VK_NULL_HANDLE;
					vr("vkQueueSubmit", &vkres, qsync.sub_info.pCommandBuffers,
						vkQueueSubmit(qsync.vk_queue, 1, &qsync.sub_info, f) ); } }

			if(!ei.run_headless) {
				if(valid) {
				//	Acquire a VkImage from the swapchain's pool
					vr("vkAcquireNextImageKHR", &vkres, swap_image_index,
						vkAcquireNextImageKHR(vob.VKL, vk_swapchain, UINT64_MAX, vk_semaphore_swapchain_img_acq, VK_NULL_HANDLE, &swap_image_index) );
					ov("swap_image_index", swap_image_index); }

				if(valid) {
				//	Copy the "Work" Image to the "Swap" Image
					rv("vkcombuf_pres");
						swpsync.sub_info.pCommandBuffers = &combuf_pres_loop[swap_image_index+((frame_index%2)*swap_image_count)].vk_command_buffer;
					VkFence f = (ei.show_gui || ei.paused) ? VK_NULL_HANDLE : qsync.vk_fence;
					swpsync.sub_info.pSignalSemaphores = (ei.show_gui || ei.paused) ? &vk_semaphore_swapchain_imgui : &vk_semaphore_swapchain_pres;
					vr("vkQueueSubmit", &vkres, swpsync.sub_info.pCommandBuffers,
						vkQueueSubmit(swpsync.vk_queue, 1, &swpsync.sub_info, f) ); }

				if(valid && (ei.show_gui || ei.paused)) {
				//	Add IMGUI interface
					rv("vkcombuf_IMGUI");
						swpsync_imgui.sub_info.pCommandBuffers = &combuf_imgui_loop[swap_image_index].vk_command_buffer;
					VkFence f = qsync.vk_fence;
					vr("vkQueueSubmit", &vkres, swpsync_imgui.sub_info.pCommandBuffers,
						vkQueueSubmit(swpsync_imgui.vk_queue, 1, &swpsync_imgui.sub_info, f) ); } }

			if(valid) {
			//	Wait for the queued commands to finish execution
				do {
					vr("vkWaitForFences <100ms>", &vkres, qsync.vk_fence,
						vkWaitForFences(vob.VKL, 1, &qsync.vk_fence, VK_TRUE, 100000000) );
				} while (vkres[vkres.size()-1] == VK_TIMEOUT); }

    		end_timer(optime, "Work Queue Time");

			if(valid) {
			//	Reset the fence for reuse in the next iteration
				vr("vkResetFences", &vkres, qsync.vk_fence,
					vkResetFences(vob.VKL, 1, &qsync.vk_fence) ); }

			if(valid && !ei.run_headless) {
				vr("vkQueuePresentKHR", &vkres, swap_image_index,
					vkQueuePresentKHR(swpsync.vk_queue, &vk_present_info) ); }

			if(!ei.paused || ei.tick_loop) { frame_index++; }

			if((!ei.paused || ei.tick_loop)
			&& 	valid
			&&  ei.export_enabled
			&&	ei.export_frequency > 0
			&&	frame_index % ei.export_frequency == 0
			&& 	frame_index > 0) {
			//	Submit 'imagedata' commands to the GPU graphics queue
				bool get_gui = false;
				if(ei.run_headless || !gc.record_imgui || !ei.show_gui) {
					rv("combuf_work_imagedata");
						qsync.sub_info.pCommandBuffers = &combuf_work_imagedata[(frame_index+0)%2].vk_command_buffer; }
				else {
					get_gui = true;
					rv("combuf_work_imagedata");
						qsync.sub_info.pCommandBuffers = &combuf_blit_imgui_loop[swap_image_index].vk_command_buffer; }
				vr("vkQueueSubmit", &vkres, qsync.sub_info.pCommandBuffers,
					vkQueueSubmit(qsync.vk_queue, 1, &qsync.sub_info, VK_NULL_HANDLE) );
				vr("vkDeviceWaitIdle", &vkres, "IDLE",
					vkDeviceWaitIdle(vob.VKL) );

			//	TODO use own sync object?
				rv("combuf_work_imagedata");
					qsync.sub_info.pCommandBuffers = &combuf_blit2buff_sing[0].vk_command_buffer;
				vr("vkQueueSubmit", &vkres, qsync.sub_info.pCommandBuffers,
					vkQueueSubmit(qsync.vk_queue, 1, &qsync.sub_info, VK_NULL_HANDLE) );
				vr("vkDeviceWaitIdle", &vkres, "IDLE",
					vkDeviceWaitIdle(vob.VKL) );
				

				optime = start_timer(optime);
				if(!verbose_loops) { loglevel = MAXLOG; }
				save_image(pvoid_blit2buff, "IMG"+std::to_string(ei.imgdat_idx), APP_W, APP_H, glfw_mouse, get_gui);
				end_timer(optime, "Save ImageData");
				if(!verbose_loops) { loglevel = -1; }
				ei.imgdat_idx++;
				dft1d(ei.imgdat_idx*735, 512, &fs, &fsm);
				if( ei.export_batch_size  > 0
				&&	ei.export_batch_left  > 0 ) { ei.export_batch_left--; }
				if( !ei.run_headless
				&&	ei.export_batch_size  > 0
				&&	ei.export_batch_left == 0 ) {
					ei.paused = true;
					ei.export_batch_left =  ei.export_batch_size; } }

		}

	//	End of loop
		if(fps_report == current_sec) {
			fps_report--;
			if(!verbose_loops) { loglevel = MAXLOG; }
			end_timer(ftime, "Full Loop Time");
			if(!verbose_loops) { loglevel = -1; } }

		if(verbose_loops) { end_timer(ftime, "Full Loop Time"); }

		hd("STAGE:", "LOOP");

		if(ei.tick_loop > 0) { ei.tick_loop--; }
		if(verbose_loops > 0) { verbose_loops--; } else { loglevel = -1; }

		do_ub_update = false;

	} while ( valid && ((!ei.run_headless && !glfwWindowShouldClose(glfw_W)) || ei.run_headless) );

	loglevel = MAXLOG;

	  ///////////////////////////////////////////////////
	 /**/	hd("STAGE:", "EXIT APPLICATION");		/**/
	///////////////////////////////////////////////////

	if(!valid) 	 							{ hd("STAGE:", "ABORTED"); }

	if(!ei.run_headless) {
		if(glfwWindowShouldClose(glfw_W)) 	{ hd("STAGE:",  "CLOSED"); }

		cleanupImGui(vob.VKL);
		cleanupGLFW(glfw_W);

	} else { rv("Headless Mode Enabled!"); }

	rv("glfwTerminate");
	glfwTerminate();

	rv("return");
	return 0;
}
