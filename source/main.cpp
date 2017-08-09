#define VK_USE_PLATFORM_WIN32_KHR 1

#include <vulkan/vulkan.h>

#include <stdio.h>
#include <windows.h>
#include <vector>
#include <thread>

#include "ShaderStructures.h"

#ifdef _MSC_VER
#pragma comment ( lib, "vulkan-1" )
#endif // _MSC_VER

// global variables 
char sAppName[] = "VKproject1";

static void mainWndProc(void);
static void renderProc(void);

std::vector<unsigned char> getBytesFromFile(const char* filename);
std::vector<char> getCharsFromFile(const char* filename);


// WNDPROC 
static LRESULT WINAPI WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);


int main(const int argc, const char* const argv[])
{
	HWND hWnd = nullptr;

	VkInstance hVkInstance = nullptr;

	VkSurfaceKHR hVkSurfaceKHR = 0;

	VkDevice hVkDevice = nullptr;

	VkQueue hVkQueue = nullptr;

	VkSwapchainKHR hVkSwapchainKHR = 0;

	std::vector<VkImageView> vectorVkImagesViews(0ULL);

	std::vector<VkFramebuffer> vectorVkFramebuffers(0ULL);

	VkRenderPass hVkRenderPass = 0;
	
	VkDescriptorSetLayout hVkDescriptorSetLayout = 0;

	VkPipelineLayout hVkPipelineLayout = 0;

	VkPipeline hVkPipeline = 0;

	VkDeviceMemory hVertexPositionColorVkDeviceMemory = 0;
	VkBuffer hVertexPositionColorVkBuffer = 0;

	VkDeviceMemory hIndexVkDeviceMemory = 0;
	VkBuffer hIndexVkBuffer = 0;

	VkBuffer hModelViewProjectionVkBuffer = 0;
	VkDeviceMemory hModelViewProjectionVkDeviceMemory = 0;

	VkDescriptorPool hVkDescriptorPool = 0;
	VkDescriptorSet hVkDescriptorSet = 0;

	VkCommandPool hVkCommandPool = 0;

	std::vector<VkCommandBuffer> vectorVkCommandBuffers(0ULL);

	VkSemaphore hImageAvalableVkSemaphore = 0;

	VkSemaphore hRenderFinishedVkSemaphore = 0;

	// Create Vk Instance 
	{
		VkApplicationInfo applicationInfo = {};
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.apiVersion = VK_API_VERSION_1_0;
		applicationInfo.pApplicationName = sAppName;

		VkInstanceCreateInfo instanceCreateInfo = {};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pApplicationInfo = &applicationInfo;
		instanceCreateInfo.enabledExtensionCount = 2;
		const char VK_KHR_SURFACE_EXTENSION_NAME_[] = VK_KHR_SURFACE_EXTENSION_NAME;
		const char VK_KHR_WIN32_SURFACE_EXTENSION_NAME_[] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
		const char* EnabledExtensionNames[] = { VK_KHR_SURFACE_EXTENSION_NAME_, VK_KHR_WIN32_SURFACE_EXTENSION_NAME_ };
		instanceCreateInfo.ppEnabledExtensionNames = EnabledExtensionNames;

		vkCreateInstance(&instanceCreateInfo, nullptr, &hVkInstance);
	}

	// Win32 
	{
		// Get Win32 Instance 
		HINSTANCE hInstance = GetModuleHandleA(nullptr);

		// Create Win32 Window 
		{
			WNDCLASSEXA wcex = {};

			// Register Window Class
			{
				wcex.cbSize = sizeof(WNDCLASSEXA);
				wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
				wcex.lpfnWndProc = WindowProc;
				wcex.hInstance = hInstance;
				const char icon_path[] = "../resources/Vulkan.ico";
				wcex.hIcon = static_cast<HICON>(LoadImageA(nullptr, icon_path, IMAGE_ICON, 256, 256, LR_LOADFROMFILE));
				wcex.hIconSm = static_cast<HICON>(LoadImageA(nullptr, icon_path, IMAGE_ICON, 32, 32, LR_LOADFROMFILE));
				wcex.hCursor = LoadCursorA(nullptr, reinterpret_cast<LPCSTR>(32512));
				wcex.hbrBackground = GetSysColorBrush(COLOR_WINDOWFRAME);
				wcex.lpszClassName = sAppName;

				RegisterClassExA(&wcex);
			}

			DWORD windowStyle = WS_SYSMENU | WS_BORDER | WS_DLGFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
			DWORD windowExtendedStyle = WS_EX_APPWINDOW;

			RECT windowRect = { 0, 0, 640, 480 };

			AdjustWindowRectEx(&windowRect, windowStyle, 0, windowExtendedStyle);

			hWnd = CreateWindowExA(
				windowExtendedStyle, // window extended style 
				wcex.lpszClassName, // name of a window class 
				sAppName, // title of a window 
				windowStyle, // window style 
				CW_USEDEFAULT, // x-position of a window 
				CW_USEDEFAULT, // y-position of a window 
				windowRect.right - windowRect.left, // width of a window 
				windowRect.bottom - windowRect.top, // height of a window 
				nullptr, // a parent window 
				nullptr, // a window menu
				hInstance, // application handle 
				nullptr);
		}

		// Create Vk Win32 Surface 
		{
			VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfoKHR = {};
			win32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			win32SurfaceCreateInfoKHR.hwnd = hWnd;
			win32SurfaceCreateInfoKHR.hinstance = hInstance;
			
			vkCreateWin32SurfaceKHR(hVkInstance, &win32SurfaceCreateInfoKHR, nullptr, &hVkSurfaceKHR);
		}
	}

	// Vk 
	{
		VkPhysicalDevice hVkPhysicalDevice = nullptr;

		int iQueueFamilyIndex = -1;

		std::vector<VkImage> vectorVkImages(0ULL);

		VkExtent2D currentExtent = {};

		// Get Vk PhysicalDevice 
		{
			unsigned uPhysicalDeviceCount = 0;
			vkEnumeratePhysicalDevices(hVkInstance, &uPhysicalDeviceCount, nullptr);

			if (uPhysicalDeviceCount)
			{
				std::vector<VkPhysicalDevice> vectorAllVkPhysicalDevices(uPhysicalDeviceCount);
				vkEnumeratePhysicalDevices(hVkInstance, &uPhysicalDeviceCount, vectorAllVkPhysicalDevices.data());

				for (const VkPhysicalDevice& hCurrentPhysicalDevice : vectorAllVkPhysicalDevices)
				{
					VkPhysicalDeviceProperties physicalDeviceProperties = {};
					vkGetPhysicalDeviceProperties(hCurrentPhysicalDevice, &physicalDeviceProperties);
					printf("%s\n", physicalDeviceProperties.deviceName);

					unsigned uQueueFamilyPropertyCount = 0;
					vkGetPhysicalDeviceQueueFamilyProperties(hCurrentPhysicalDevice, &uQueueFamilyPropertyCount, nullptr);

					// Get Queue Family Index 
					if (uQueueFamilyPropertyCount)
					{
						VkQueueFamilyProperties* pAllVkQueueFamilyProperties = new VkQueueFamilyProperties[uQueueFamilyPropertyCount];
						vkGetPhysicalDeviceQueueFamilyProperties(hCurrentPhysicalDevice, &uQueueFamilyPropertyCount, pAllVkQueueFamilyProperties);

						for (unsigned index = 0; index < uQueueFamilyPropertyCount; ++index)
						{
							VkBool32 bIsSupported = FALSE;
							vkGetPhysicalDeviceSurfaceSupportKHR(hCurrentPhysicalDevice, index, hVkSurfaceKHR, &bIsSupported);

							if (bIsSupported && pAllVkQueueFamilyProperties[index].queueFlags & VK_QUEUE_GRAPHICS_BIT && pAllVkQueueFamilyProperties[index].queueCount)
							{
								hVkPhysicalDevice = hCurrentPhysicalDevice;
								iQueueFamilyIndex = index;
								break;
							}
						}

						delete[] pAllVkQueueFamilyProperties;
						pAllVkQueueFamilyProperties = nullptr;
					}

					if (hVkPhysicalDevice)
					{
						break;
					}
				}
			}
		}

		// Create Vk Device 
		{
			VkDeviceQueueCreateInfo deviceQueueCreateInfo = {};
			deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			deviceQueueCreateInfo.queueFamilyIndex = iQueueFamilyIndex;
			deviceQueueCreateInfo.queueCount = 1;
			const float fQueuePriorities[] = { 1.f };
			deviceQueueCreateInfo.pQueuePriorities = fQueuePriorities;

			VkDeviceCreateInfo deviceCreateInfo = {};
			deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			deviceCreateInfo.queueCreateInfoCount = 1;
			deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
			deviceCreateInfo.enabledExtensionCount = 1;
			const char VK_KHR_SWAPCHAIN_EXTENSION_NAME_[] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
			const char* EnabledExtensionNames[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME_ };
			deviceCreateInfo.ppEnabledExtensionNames = EnabledExtensionNames;

			vkCreateDevice(hVkPhysicalDevice, &deviceCreateInfo, nullptr, &hVkDevice);
		}

		// Get Vk Queue 
		{
			vkGetDeviceQueue(hVkDevice, iQueueFamilyIndex, 0, &hVkQueue);
		}

		{
			VkSurfaceFormatKHR surfaceFormatKHR = {};

			// Get Vk Surface Format 
			{
				unsigned upSurfaceFormatCount = 0;
				vkGetPhysicalDeviceSurfaceFormatsKHR(hVkPhysicalDevice, hVkSurfaceKHR, &upSurfaceFormatCount, nullptr);

				if (upSurfaceFormatCount)
				{
					std::vector<VkSurfaceFormatKHR> vectorSurfaceFormats(upSurfaceFormatCount);

					vkGetPhysicalDeviceSurfaceFormatsKHR(hVkPhysicalDevice, hVkSurfaceKHR, &upSurfaceFormatCount, vectorSurfaceFormats.data());

					surfaceFormatKHR = vectorSurfaceFormats[0];

					if (upSurfaceFormatCount > 1)
					{
						for (const VkSurfaceFormatKHR& vkSurfaceFormatKHR : vectorSurfaceFormats)
						{
							if (vkSurfaceFormatKHR.format == VK_FORMAT_B8G8R8A8_UNORM && vkSurfaceFormatKHR.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
							{
								surfaceFormatKHR = vkSurfaceFormatKHR;
							}
						}
					}
					else if (vectorSurfaceFormats[0].format == VK_FORMAT_UNDEFINED)
					{
						surfaceFormatKHR = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
					}
				}
			}

			// Create Vk Swapchain 
			{
				VkPresentModeKHR presentModeKHR = VK_PRESENT_MODE_FIFO_KHR;
				// Get Vk Present Mode 
				{
					unsigned uPresentModeCount = 0;

					vkGetPhysicalDeviceSurfacePresentModesKHR(hVkPhysicalDevice, hVkSurfaceKHR, &uPresentModeCount, nullptr);

					if (uPresentModeCount)
					{
						std::vector<VkPresentModeKHR> vectorPresentModes(uPresentModeCount);

						vkGetPhysicalDeviceSurfacePresentModesKHR(hVkPhysicalDevice, hVkSurfaceKHR, &uPresentModeCount, vectorPresentModes.data());

						for (const VkPresentModeKHR& vkPresentModeKHR : vectorPresentModes)
						{
							if (vkPresentModeKHR == VK_PRESENT_MODE_MAILBOX_KHR)
							{
								presentModeKHR = vkPresentModeKHR;
								break;
							}
						}
					}
				}

				VkSurfaceCapabilitiesKHR surfaceCapabilitiesKHR = {};
				// Get Vk Surface Capabilities 
				{
					vkGetPhysicalDeviceSurfaceCapabilitiesKHR(hVkPhysicalDevice, hVkSurfaceKHR, &surfaceCapabilitiesKHR);
					currentExtent = surfaceCapabilitiesKHR.currentExtent;
				}

				VkSwapchainCreateInfoKHR swapchainCreateInfoKHR = {};
				swapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
				swapchainCreateInfoKHR.surface = hVkSurfaceKHR;
				swapchainCreateInfoKHR.minImageCount = surfaceCapabilitiesKHR.maxImageCount < 3 ? surfaceCapabilitiesKHR.maxImageCount : 3;
				swapchainCreateInfoKHR.imageFormat = surfaceFormatKHR.format;
				swapchainCreateInfoKHR.imageColorSpace = surfaceFormatKHR.colorSpace;
				swapchainCreateInfoKHR.imageExtent = currentExtent;
				swapchainCreateInfoKHR.imageArrayLayers = 1;
				swapchainCreateInfoKHR.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				swapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				swapchainCreateInfoKHR.preTransform = surfaceCapabilitiesKHR.currentTransform;
				swapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
				swapchainCreateInfoKHR.presentMode = presentModeKHR;
				swapchainCreateInfoKHR.clipped = TRUE;

				vkCreateSwapchainKHR(hVkDevice, &swapchainCreateInfoKHR, nullptr, &hVkSwapchainKHR);
			}

			// Get Vk Swapchain Images 
			{
			unsigned uSwapchainImageCount = 0;

			vkGetSwapchainImagesKHR(hVkDevice, hVkSwapchainKHR, &uSwapchainImageCount, nullptr);

			vectorVkImages.resize(uSwapchainImageCount);

			vkGetSwapchainImagesKHR(hVkDevice, hVkSwapchainKHR, &uSwapchainImageCount, vectorVkImages.data());
			}

			// Get Vk Swapchain Images Views 
			{
				vectorVkImagesViews.resize(vectorVkImages.size());

				for (size_t i = 0; i < vectorVkImagesViews.size(); i++)
				{
					VkImageViewCreateInfo imageViewCreateInfo = {};
					imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
					imageViewCreateInfo.image = vectorVkImages[i];
					imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
					imageViewCreateInfo.format = surfaceFormatKHR.format;
					imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
					imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
					imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
					imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
					imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
					imageViewCreateInfo.subresourceRange.levelCount = 1;
					imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
					imageViewCreateInfo.subresourceRange.layerCount = 1;

					vkCreateImageView(hVkDevice, &imageViewCreateInfo, nullptr, &vectorVkImagesViews[i]);
				}
			}

			// Create Vk Render Pass 
			{
				VkAttachmentDescription attachmentDescription = {};
				attachmentDescription.format = surfaceFormatKHR.format;
				attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
				attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

				VkAttachmentReference attachmentReference = {};
				attachmentReference.attachment = 0;
				attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				VkSubpassDescription subpassDescription = {};
				subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subpassDescription.colorAttachmentCount = 1;
				subpassDescription.pColorAttachments = &attachmentReference;

				VkSubpassDependency subpassDependency = {};
				subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
				subpassDependency.dstSubpass = 0;
				subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				subpassDependency.srcAccessMask = 0;
				subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

				VkRenderPassCreateInfo renderPassCreateInfo = {};
				renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
				renderPassCreateInfo.attachmentCount = 1;
				renderPassCreateInfo.pAttachments = &attachmentDescription;
				renderPassCreateInfo.subpassCount = 1;
				renderPassCreateInfo.pSubpasses = &subpassDescription;
				renderPassCreateInfo.dependencyCount = 1;
				renderPassCreateInfo.pDependencies = &subpassDependency;

				vkCreateRenderPass(hVkDevice, &renderPassCreateInfo, nullptr, &hVkRenderPass);
			}
		}

		// Create Vk DescriptorSet Layout 
		{
			VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
			descriptorSetLayoutBinding.binding = 0;
			descriptorSetLayoutBinding.descriptorCount = 1;
			descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorSetLayoutBinding.pImmutableSamplers = nullptr;
			descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

			VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
			descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptorSetLayoutCreateInfo.bindingCount = 1;
			descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding;

			vkCreateDescriptorSetLayout(hVkDevice, &descriptorSetLayoutCreateInfo, nullptr, &hVkDescriptorSetLayout);
		}

		// Create Vk Pipeline Layout 
		{
			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
			pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutCreateInfo.setLayoutCount = 1;
			pipelineLayoutCreateInfo.pSetLayouts = &hVkDescriptorSetLayout;

			vkCreatePipelineLayout(hVkDevice, &pipelineLayoutCreateInfo, nullptr, &hVkPipelineLayout);
		}

		// Create Vk Pipeline 
		{
			VkPipelineShaderStageCreateInfo shaderStagesCreateInfos[] = { {}, {} };

			VkShaderModule hVertexVkShaderModule = 0;
			// Create Vertex Vk Shader Module 
			{
				const char sVertexFile[] = "../resources/shaders/color.vertex.svp";
				std::vector<unsigned char> vectorCodeBytes = getBytesFromFile(sVertexFile);

				VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
				shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				shaderModuleCreateInfo.codeSize = vectorCodeBytes.size();
				shaderModuleCreateInfo.pCode = static_cast<const uint32_t*>(static_cast<const void*>(vectorCodeBytes.data()));

				vkCreateShaderModule(hVkDevice, &shaderModuleCreateInfo, nullptr, &hVertexVkShaderModule);
			}

			// VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
			shaderStagesCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStagesCreateInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
			shaderStagesCreateInfos[0].module = hVertexVkShaderModule;
			shaderStagesCreateInfos[0].pName = "main";

			VkShaderModule hFragmentVkShaderModule = 0;
			// Create Fragment Vk Shader Module 
			{
				const char sFragmentFile[] = "../resources/shaders/color.fragment.svp";
				std::vector<unsigned char> vectorCodeBytes = getBytesFromFile(sFragmentFile);

				VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
				shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				shaderModuleCreateInfo.codeSize = vectorCodeBytes.size();
				shaderModuleCreateInfo.pCode = static_cast<const uint32_t*>(static_cast<const void*>(vectorCodeBytes.data()));

				vkCreateShaderModule(hVkDevice, &shaderModuleCreateInfo, nullptr, &hFragmentVkShaderModule);
			}

			// VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
			shaderStagesCreateInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStagesCreateInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			shaderStagesCreateInfos[1].module = hFragmentVkShaderModule;
			shaderStagesCreateInfos[1].pName = "main";

			VkVertexInputBindingDescription vertexInputBindingDescription = {};
			vertexInputBindingDescription.binding = 0;
			vertexInputBindingDescription.stride = sizeof VertexPositionColor;
			vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			VkVertexInputAttributeDescription vertexInputAttributsDescriptions[] = { {}, {} };
			// Attribute location 0: Position 
			vertexInputAttributsDescriptions[0].binding = 0;
			vertexInputAttributsDescriptions[0].location = 0;
			vertexInputAttributsDescriptions[0].offset = offsetof(VertexPositionColor, position);
			vertexInputAttributsDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			// Attribute location 1: Color 
			vertexInputAttributsDescriptions[1].binding = 0;
			vertexInputAttributsDescriptions[1].location = 1;
			vertexInputAttributsDescriptions[1].offset = offsetof(VertexPositionColor, color);
			vertexInputAttributsDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;

			VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
			vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
			vertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
			vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 2;
			vertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributsDescriptions;

			VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
			inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssemblyStateCreateInfo.primitiveRestartEnable = FALSE;

			VkViewport viewport = { 0.f, 0.f, 
				static_cast<float>(currentExtent.width), static_cast<float>(currentExtent.height),
				0.f, 1.f };

			VkRect2D scissor = { 0, 0, currentExtent };

			VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
			viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportStateCreateInfo.viewportCount = 1;
			viewportStateCreateInfo.pViewports = &viewport;
			viewportStateCreateInfo.scissorCount = 1;
			viewportStateCreateInfo.pScissors = &scissor;

			VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {};
			rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizationStateCreateInfo.depthClampEnable = FALSE;
			rasterizationStateCreateInfo.rasterizerDiscardEnable = FALSE;
			rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizationStateCreateInfo.lineWidth = 1.f;
			rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
			rasterizationStateCreateInfo.depthBiasEnable = FALSE;

			VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
			multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampleStateCreateInfo.sampleShadingEnable = FALSE;
			multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

			VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
			colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachmentState.blendEnable = FALSE;

			VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
			colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlendStateCreateInfo.logicOpEnable = FALSE;
			colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
			colorBlendStateCreateInfo.attachmentCount = 1;
			colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
			colorBlendStateCreateInfo.blendConstants[0] = 0.f;
			colorBlendStateCreateInfo.blendConstants[1] = 0.f;
			colorBlendStateCreateInfo.blendConstants[2] = 0.f;
			colorBlendStateCreateInfo.blendConstants[3] = 0.f;

			VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
			graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			graphicsPipelineCreateInfo.stageCount = 2;
			graphicsPipelineCreateInfo.pStages = shaderStagesCreateInfos;
			graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
			graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
			graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
			graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
			graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
			graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
			graphicsPipelineCreateInfo.layout = hVkPipelineLayout;
			graphicsPipelineCreateInfo.renderPass = hVkRenderPass;

			vkCreateGraphicsPipelines(hVkDevice, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &hVkPipeline);

			vkDestroyShaderModule(hVkDevice, hVertexVkShaderModule, nullptr);
			vkDestroyShaderModule(hVkDevice, hFragmentVkShaderModule, nullptr);
		}

		// Create Vk Frame buffers 
		{
			vectorVkFramebuffers.resize(vectorVkImages.size());

			for (size_t i = 0; i < vectorVkFramebuffers.size(); ++i)
			{
				VkFramebufferCreateInfo framebufferCreateInfo = {};
				framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferCreateInfo.renderPass = hVkRenderPass;
				framebufferCreateInfo.attachmentCount = 1;
				framebufferCreateInfo.pAttachments = &vectorVkImagesViews[i];
				framebufferCreateInfo.width = currentExtent.width;
				framebufferCreateInfo.height = currentExtent.height;
				framebufferCreateInfo.layers = 1;

				vkCreateFramebuffer(hVkDevice, &framebufferCreateInfo, nullptr, &vectorVkFramebuffers[i]);
			}
		}

		// Create Vk Command Pool 
		{
			VkCommandPoolCreateInfo commandPoolCreateInfo = {};
			commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			commandPoolCreateInfo.queueFamilyIndex = iQueueFamilyIndex;

			vkCreateCommandPool(hVkDevice, &commandPoolCreateInfo, nullptr, &hVkCommandPool);
		}

		// Vertex Position Color Buffer 
		{
			VertexPositionColor vVertexPositionColor[] = {
				// { position, color }
				{ { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f } },
				{ { -1.0f, -1.0f,  1.0f }, { 0.0f, 0.0f, 1.0f } },
				{ { -1.0f,  1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f } },
				{ { -1.0f,  1.0f,  1.0f }, { 0.0f, 1.0f, 1.0f } },
				{ { 1.0f, -1.0f, -1.0f }, { 1.0f, 0.0f, 0.0f } },
				{ { 1.0f, -1.0f,  1.0f }, { 1.0f, 0.0f, 1.0f } },
				{ { 1.0f,  1.0f, -1.0f }, { 1.0f, 1.0f, 0.0f } },
				{ { 1.0f,  1.0f,  1.0f }, { 1.0f, 1.0f, 1.0f } },
			};

			const size_t size = sizeof vVertexPositionColor;

			VkBuffer hStagingVkBuffer = 0;
			VkDeviceMemory hStagingVkDeviceMemory = 0;

			{
				VkBufferCreateInfo bufferCreateInfo = {};
				bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

				// Create Staging Vk Buffer 
				{
					bufferCreateInfo.size = size;
					bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
					bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

					vkCreateBuffer(hVkDevice, &bufferCreateInfo, nullptr, &hStagingVkBuffer);
				}

				VkMemoryAllocateInfo memoryAllocateInfo = {};
				memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

				// Allocate Staging Vk Memory 
				{
					VkMemoryRequirements memoryRequirements;
					vkGetBufferMemoryRequirements(hVkDevice, hStagingVkBuffer, &memoryRequirements);

					memoryAllocateInfo.allocationSize = memoryRequirements.size;

					// Get memory Type Index 
					{
						VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
						VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
						vkGetPhysicalDeviceMemoryProperties(hVkPhysicalDevice, &physicalDeviceMemoryProperties);

						for (unsigned memoryTypeIndex = 0; memoryTypeIndex < physicalDeviceMemoryProperties.memoryTypeCount; ++memoryTypeIndex)
						{
							if ((memoryRequirements.memoryTypeBits & (1 << memoryTypeIndex))
								&& (physicalDeviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
							{
								memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
								break;
							}
						}
					}

					vkAllocateMemory(hVkDevice, &memoryAllocateInfo, nullptr, &hStagingVkDeviceMemory);
				}

				vkBindBufferMemory(hVkDevice, hStagingVkBuffer, hStagingVkDeviceMemory, 0);

				// Map and copy 
				{
					void* data;

					vkMapMemory(hVkDevice, hStagingVkDeviceMemory, 0, size, 0, &data);

					memcpy(data, vVertexPositionColor, size);

					vkUnmapMemory(hVkDevice, hStagingVkDeviceMemory);
				}

				vkCreateBuffer(hVkDevice, &bufferCreateInfo, nullptr, &hVertexPositionColorVkBuffer);

				vkAllocateMemory(hVkDevice, &memoryAllocateInfo, nullptr, &hVertexPositionColorVkDeviceMemory);

				vkBindBufferMemory(hVkDevice, hVertexPositionColorVkBuffer, hVertexPositionColorVkDeviceMemory, 0);
			}

			// Copy 
			{
				VkCommandBuffer commandBuffer = nullptr;

				// Allocate Vk Command Buffer 
				{
					VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
					commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
					commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
					commandBufferAllocateInfo.commandPool = hVkCommandPool;
					commandBufferAllocateInfo.commandBufferCount = 1;

					vkAllocateCommandBuffers(hVkDevice, &commandBufferAllocateInfo, &commandBuffer);
				}

				// Fill Vk Command Buffer 
				{
					VkCommandBufferBeginInfo commandBufferBeginInfo = {};
					commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
					commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

					vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
					{
						VkBufferCopy copyRegion = {};
						copyRegion.size = size;
						vkCmdCopyBuffer(commandBuffer, hStagingVkBuffer, hVertexPositionColorVkBuffer, 1, &copyRegion);
					}
					vkEndCommandBuffer(commandBuffer);
				}

				// Submit 
				{
					VkSubmitInfo submitInfo = {};
					submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
					submitInfo.commandBufferCount = 1;
					submitInfo.pCommandBuffers = &commandBuffer;

					vkQueueSubmit(hVkQueue, 1, &submitInfo, 0);
					vkQueueWaitIdle(hVkQueue);
				}

				vkFreeCommandBuffers(hVkDevice, hVkCommandPool, 1, &commandBuffer);
				commandBuffer = nullptr;
			}

			vkDestroyBuffer(hVkDevice, hStagingVkBuffer, nullptr);
			hStagingVkBuffer = 0;

			vkFreeMemory(hVkDevice, hStagingVkDeviceMemory, nullptr);
			hStagingVkDeviceMemory = 0;
		}

		// Index Buffer 
		{
			unsigned short indices[36] = {
				0, 1, 2, // -x 
				1, 3, 2,
				4, 6, 5, // +x 
				5, 6, 7,
				0, 5, 1, // -y 
				0, 4, 5,
				2, 7, 6, // +y 
				2, 3, 7,
				0, 6, 4, // -z 
				0, 2, 6,
				1, 7, 3, // +z 
				1, 5, 7,
			};

			size_t size = sizeof indices;

			VkBuffer hStagingVkBuffer = 0;
			VkDeviceMemory hStagingVkDeviceMemory = 0;

			{
				VkBufferCreateInfo bufferCreateInfo = {};
				bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

				// Create Staging Vk Buffer 
				{
					bufferCreateInfo.size = size;
					bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
					bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

					vkCreateBuffer(hVkDevice, &bufferCreateInfo, nullptr, &hStagingVkBuffer);
				}

				VkMemoryAllocateInfo memoryAllocateInfo = {};
				memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

				// Allocate Staging Vk Memory 
				{
					VkMemoryRequirements memoryRequirements;
					vkGetBufferMemoryRequirements(hVkDevice, hStagingVkBuffer, &memoryRequirements);

					memoryAllocateInfo.allocationSize = memoryRequirements.size;

					// Get memory Type Index 
					{
						VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
						VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
						vkGetPhysicalDeviceMemoryProperties(hVkPhysicalDevice, &physicalDeviceMemoryProperties);

						for (unsigned memoryTypeIndex = 0; memoryTypeIndex < physicalDeviceMemoryProperties.memoryTypeCount; ++memoryTypeIndex)
						{
							if ((memoryRequirements.memoryTypeBits & (1 << memoryTypeIndex))
								&& (physicalDeviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
							{
								memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
								break;
							}
						}
					}

					vkAllocateMemory(hVkDevice, &memoryAllocateInfo, nullptr, &hStagingVkDeviceMemory);
				}

				vkBindBufferMemory(hVkDevice, hStagingVkBuffer, hStagingVkDeviceMemory, 0);

				// Map and copy 
				{
					void* data;

					vkMapMemory(hVkDevice, hStagingVkDeviceMemory, 0, size, 0, &data);

					memcpy(data, indices, size);

					vkUnmapMemory(hVkDevice, hStagingVkDeviceMemory);
				}

				vkCreateBuffer(hVkDevice, &bufferCreateInfo, nullptr, &hIndexVkBuffer);

				vkAllocateMemory(hVkDevice, &memoryAllocateInfo, nullptr, &hIndexVkDeviceMemory);

				vkBindBufferMemory(hVkDevice, hIndexVkBuffer, hIndexVkDeviceMemory, 0);
			}

			// Copy 
			{
				VkCommandBuffer commandBuffer = nullptr;

				// Allocate Vk Command Buffer 
				{
					VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
					commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
					commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
					commandBufferAllocateInfo.commandPool = hVkCommandPool;
					commandBufferAllocateInfo.commandBufferCount = 1;

					vkAllocateCommandBuffers(hVkDevice, &commandBufferAllocateInfo, &commandBuffer);
				}

				// Fill Vk Command Buffer 
				{
					VkCommandBufferBeginInfo commandBufferBeginInfo = {};
					commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
					commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

					vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
					{
						VkBufferCopy copyRegion = {};
						copyRegion.size = size;
						vkCmdCopyBuffer(commandBuffer, hStagingVkBuffer, hIndexVkBuffer, 1, &copyRegion);
					}
					vkEndCommandBuffer(commandBuffer);
				}

				// Submit 
				{
					VkSubmitInfo submitInfo = {};
					submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
					submitInfo.commandBufferCount = 1;
					submitInfo.pCommandBuffers = &commandBuffer;

					vkQueueSubmit(hVkQueue, 1, &submitInfo, 0);
					vkQueueWaitIdle(hVkQueue);
				}

				vkFreeCommandBuffers(hVkDevice, hVkCommandPool, 1, &commandBuffer);
				commandBuffer = nullptr;
			}

			vkDestroyBuffer(hVkDevice, hStagingVkBuffer, nullptr);
			hStagingVkBuffer = 0;

			vkFreeMemory(hVkDevice, hStagingVkDeviceMemory, nullptr);
			hStagingVkDeviceMemory = 0;
		}

		// ModelViewProjectionBuffer 
		{
			const size_t size = sizeof ModelViewProjectionBuffer;

			// Create Vk Buffer 
			{
				VkBufferCreateInfo bufferCreateInfo = {};
				bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				bufferCreateInfo.size = size;
				bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				vkCreateBuffer(hVkDevice, &bufferCreateInfo, nullptr, &hModelViewProjectionVkBuffer);
			}

			// Allocate Vk Memory 
			{
				VkMemoryRequirements memoryRequirements;
				vkGetBufferMemoryRequirements(hVkDevice, hModelViewProjectionVkBuffer, &memoryRequirements);

				VkMemoryAllocateInfo memoryAllocateInfo = {};
				memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				memoryAllocateInfo.allocationSize = memoryRequirements.size;

				// Get memory Type Index 
				{
					VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
					VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
					vkGetPhysicalDeviceMemoryProperties(hVkPhysicalDevice, &physicalDeviceMemoryProperties);

					for (unsigned memoryTypeIndex = 0; memoryTypeIndex < physicalDeviceMemoryProperties.memoryTypeCount; ++memoryTypeIndex)
					{
						if ((memoryRequirements.memoryTypeBits & (1 << memoryTypeIndex))
							&& (physicalDeviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
						{
							memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
							break;
						}
					}
				}

				vkAllocateMemory(hVkDevice, &memoryAllocateInfo, nullptr, &hModelViewProjectionVkDeviceMemory);
			}

			{
				vkBindBufferMemory(hVkDevice, hModelViewProjectionVkBuffer, hModelViewProjectionVkDeviceMemory, 0);
			}

			// Create Vk DescriptorPool 
			{
				VkDescriptorPoolSize descriptorPoolSize = {};
				descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorPoolSize.descriptorCount = 1;

				VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
				descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				descriptorPoolCreateInfo.poolSizeCount = 1;
				descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;
				descriptorPoolCreateInfo.maxSets = 1;

				vkCreateDescriptorPool(hVkDevice, &descriptorPoolCreateInfo, nullptr, &hVkDescriptorPool);
			}

			// Allocate Vk DescriptorSet 
			{
				VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
				descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				descriptorSetAllocateInfo.descriptorPool = hVkDescriptorPool;
				descriptorSetAllocateInfo.descriptorSetCount = 1;
				descriptorSetAllocateInfo.pSetLayouts = &hVkDescriptorSetLayout;

				vkAllocateDescriptorSets(hVkDevice, &descriptorSetAllocateInfo, &hVkDescriptorSet);
			}

			// Update Vk DescriptorSet 
			{
				VkDescriptorBufferInfo descriptorBufferInfo = {};
				descriptorBufferInfo.buffer = hModelViewProjectionVkBuffer;
				descriptorBufferInfo.offset = 0;
				descriptorBufferInfo.range = size;

				VkWriteDescriptorSet writeDescriptorSet = {};
				writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSet.dstSet = hVkDescriptorSet;
				writeDescriptorSet.dstBinding = 0;
				writeDescriptorSet.dstArrayElement = 0;
				writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeDescriptorSet.descriptorCount = 1;
				writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;

				vkUpdateDescriptorSets(hVkDevice, 1, &writeDescriptorSet, 0, nullptr);
			}
		}

		// Allocate Vk Command Buffers 
		{
			VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
			commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			commandBufferAllocateInfo.commandPool = hVkCommandPool;
			commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(vectorVkImages.size());

			vectorVkCommandBuffers.resize(commandBufferAllocateInfo.commandBufferCount);

			vkAllocateCommandBuffers(hVkDevice, &commandBufferAllocateInfo, vectorVkCommandBuffers.data());
		}

		// Fill Vk Command Buffers 
		{
			VkCommandBufferBeginInfo commandBufferBeginInfo = {};
			commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

			VkClearColorValue clearColorValue = { 0.5f, 0.1f, 0.4f, 1.f };

			VkImageSubresourceRange imageSubresourceRange = {};
			imageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageSubresourceRange.baseMipLevel = 0;
			imageSubresourceRange.levelCount = 1;
			imageSubresourceRange.baseArrayLayer = 0;
			imageSubresourceRange.layerCount = 1;
			
			for (int i = 0; i < vectorVkImages.size(); ++i)
			{
				vkBeginCommandBuffer(vectorVkCommandBuffers[i], &commandBufferBeginInfo);

				// Vk Image Memory Barrier Present To Clear 
				{
					VkImageMemoryBarrier imageMemoryBarrierPresentToClear = {};
					imageMemoryBarrierPresentToClear.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					imageMemoryBarrierPresentToClear.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
					imageMemoryBarrierPresentToClear.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					imageMemoryBarrierPresentToClear.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					imageMemoryBarrierPresentToClear.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
					imageMemoryBarrierPresentToClear.srcQueueFamilyIndex = iQueueFamilyIndex;
					imageMemoryBarrierPresentToClear.dstQueueFamilyIndex = iQueueFamilyIndex;
					imageMemoryBarrierPresentToClear.image = vectorVkImages[i];
					imageMemoryBarrierPresentToClear.subresourceRange = imageSubresourceRange;

					vkCmdPipelineBarrier(vectorVkCommandBuffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrierPresentToClear);
				}

				// Vk Render Pass 
				{
					VkRenderPassBeginInfo renderPassBeginInfo = {};
					renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
					renderPassBeginInfo.renderPass = hVkRenderPass;
					renderPassBeginInfo.framebuffer = vectorVkFramebuffers[i];
					renderPassBeginInfo.renderArea.offset = { 0, 0 };
					renderPassBeginInfo.renderArea.extent = currentExtent;

					VkClearValue clearColor = { clearColorValue };
					renderPassBeginInfo.clearValueCount = 1;
					renderPassBeginInfo.pClearValues = &clearColor;

					vkCmdBeginRenderPass(vectorVkCommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
					{
						vkCmdBindPipeline(vectorVkCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, hVkPipeline);
					}
					vkCmdEndRenderPass(vectorVkCommandBuffers[i]);
				}

				// Vk Image Memory Barrier Clear To Present 
				{
					VkImageMemoryBarrier imageMemoryBarrierClearToPresent = {};
					imageMemoryBarrierClearToPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					imageMemoryBarrierClearToPresent.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					imageMemoryBarrierClearToPresent.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
					imageMemoryBarrierClearToPresent.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
					imageMemoryBarrierClearToPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
					imageMemoryBarrierClearToPresent.srcQueueFamilyIndex = iQueueFamilyIndex;
					imageMemoryBarrierClearToPresent.dstQueueFamilyIndex = iQueueFamilyIndex;
					imageMemoryBarrierClearToPresent.image = vectorVkImages[i];
					imageMemoryBarrierClearToPresent.subresourceRange = imageSubresourceRange;

					vkCmdPipelineBarrier(vectorVkCommandBuffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrierClearToPresent);
				}

				vkEndCommandBuffer(vectorVkCommandBuffers[i]);
			}
		}
	}

	// Create Vk Semaphores: Image Avalable & Rendering Finished 
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		vkCreateSemaphore(hVkDevice, &semaphoreCreateInfo, nullptr, &hImageAvalableVkSemaphore);

		vkCreateSemaphore(hVkDevice, &semaphoreCreateInfo, nullptr, &hRenderFinishedVkSemaphore);
	}

	ShowWindow(hWnd, SW_NORMAL);
	SetForegroundWindow(hWnd);

	MSG msg = {};

	while (msg.message != WM_QUIT)
	{
		// Win Peek Messages 
		if (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}

		// Vk Draw 
		{
			unsigned uImageIndex;
			vkAcquireNextImageKHR(hVkDevice, hVkSwapchainKHR, static_cast<uint64_t>(-1), hImageAvalableVkSemaphore, 0, &uImageIndex);

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &hImageAvalableVkSemaphore;
			VkPipelineStageFlags pipelineStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			submitInfo.pWaitDstStageMask = &pipelineStageFlags;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &vectorVkCommandBuffers[uImageIndex];
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &hRenderFinishedVkSemaphore;

			vkQueueSubmit(hVkQueue, 1, &submitInfo, 0);

			VkPresentInfoKHR presentInfoKHR = {};
			presentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfoKHR.waitSemaphoreCount = 1;
			presentInfoKHR.pWaitSemaphores = &hRenderFinishedVkSemaphore;
			presentInfoKHR.swapchainCount = 1;
			presentInfoKHR.pSwapchains = &hVkSwapchainKHR;
			presentInfoKHR.pImageIndices = &uImageIndex;

			vkQueuePresentKHR(hVkQueue, &presentInfoKHR);
		}

		Sleep(15);
	}

	vkDestroySemaphore(hVkDevice, hRenderFinishedVkSemaphore, nullptr);
	hRenderFinishedVkSemaphore = 0;

	vkDestroySemaphore(hVkDevice, hImageAvalableVkSemaphore, nullptr);
	hImageAvalableVkSemaphore = 0;

	vkFreeCommandBuffers(hVkDevice, hVkCommandPool, static_cast<uint32_t>(vectorVkCommandBuffers.size()), vectorVkCommandBuffers.data());
	vectorVkCommandBuffers.clear();

	vkDestroyCommandPool(hVkDevice, hVkCommandPool, nullptr);
	hVkCommandPool = 0;

	vkDestroyBuffer(hVkDevice, hIndexVkBuffer, nullptr);
	hIndexVkBuffer = 0;

	vkFreeMemory(hVkDevice, hIndexVkDeviceMemory, nullptr);
	hIndexVkDeviceMemory = 0;

	vkDestroyBuffer(hVkDevice, hVertexPositionColorVkBuffer, nullptr);
	hVertexPositionColorVkBuffer = 0;

	vkFreeMemory(hVkDevice, hVertexPositionColorVkDeviceMemory, nullptr);
	hVertexPositionColorVkDeviceMemory = 0;

	for (const VkFramebuffer& hVkFramebuffer : vectorVkFramebuffers)
	{
		vkDestroyFramebuffer(hVkDevice, hVkFramebuffer, nullptr);
	}
	vectorVkFramebuffers.clear();

	vkDestroyDescriptorSetLayout(hVkDevice, hVkDescriptorSetLayout, nullptr);
	hVkDescriptorSetLayout = 0;

	vkDestroyBuffer(hVkDevice, hModelViewProjectionVkBuffer, nullptr);
	hModelViewProjectionVkBuffer = 0;

	vkFreeMemory(hVkDevice, hModelViewProjectionVkDeviceMemory, nullptr);
	hModelViewProjectionVkDeviceMemory = 0;

	vkDestroyPipeline(hVkDevice, hVkPipeline, nullptr);
	hVkPipeline = 0;

	vkDestroyPipelineLayout(hVkDevice, hVkPipelineLayout, nullptr);
	hVkPipelineLayout = 0;

	vkDestroyRenderPass(hVkDevice, hVkRenderPass, nullptr);
	hVkRenderPass = 0;

	for (const VkImageView& hVkImageView : vectorVkImagesViews)
	{
		vkDestroyImageView(hVkDevice, hVkImageView, nullptr);
	}
	vectorVkImagesViews.clear();

	vkDestroySwapchainKHR(hVkDevice, hVkSwapchainKHR, nullptr);
	hVkSwapchainKHR = 0;

	vkDestroyDevice(hVkDevice, nullptr);
	hVkDevice = nullptr;

	vkDestroySurfaceKHR(hVkInstance, hVkSurfaceKHR, nullptr);
	hVkSurfaceKHR = 0;

	vkDestroyInstance(hVkInstance, nullptr);
	hVkInstance = nullptr;

	return 0;
}

void mainWndProc(void)
{
}

void renderProc(void)
{
}

std::vector<char> getCharsFromFile(const char* filename)
{
	std::vector<char> result(0ULL);

	FILE* filePtr = fopen(filename, "rb");

	if (!filePtr)
	{
		char path[2048];
		sprintf(path, "./%s", filename);
		filePtr = fopen(path, "rb");
	}

	if (filePtr)
	{
		fseek(filePtr, 0L, SEEK_END);
		const long lSize = ftell(filePtr);

		result.resize(lSize);
		
		fread(result.data(), 1, lSize, filePtr);

		fclose(filePtr);
	}

	return result;
}

std::vector<unsigned char> getBytesFromFile(const char* filename)
{
	std::vector<unsigned char> result(0ULL);

	FILE* filePtr = fopen(filename, "rb");

	if (!filePtr)
	{
		char path[2048];
		sprintf(path, "./%s", filename);
		filePtr = fopen(path, "rb");
	}

	if (filePtr)
	{
		fseek(filePtr, 0L, SEEK_END);
		const long lSize = ftell(filePtr);

		result.resize(lSize);

		fread(result.data(), 1, lSize, filePtr);

		fclose(filePtr);
	}

	return result;
}

static LRESULT WINAPI WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		return 0;

	}

	return DefWindowProcA(hWnd, Msg, wParam, lParam);
}

