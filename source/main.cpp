#define VK_USE_PLATFORM_WIN32_KHR 1

#include <vulkan/vulkan.h>
#include <stdio.h>
#include <windows.h>
#include <vector>
#include <thread>

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

	VkPipelineLayout hVkPipelineLayout = 0;

	VkPipeline hVkPipeline = 0;

	VkDeviceMemory hVertexPositionVkDeviceMemory = 0;
	VkBuffer hVertexPositionVkBuffer = 0;

	VkDeviceMemory hVertexColorVkDeviceMemory = 0;
	VkBuffer hVertexColorVkBuffer = 0;

	VkDeviceMemory hIndexVkDeviceMemory = 0;
	VkBuffer hIndexVkBuffer = 0;

	VkCommandPool hVkCommandPool = 0;

	std::vector<VkCommandBuffer> vectorVkCommandBuffers(0ULL);

	VkSemaphore hVkSemaphoreImageAvalable = 0;

	VkSemaphore hVkSemaphoreRenderFinished = 0;

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

			DWORD windowStyle = WS_SYSMENU | WS_BORDER | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
			DWORD windowExtendedStyle = WS_EX_APPWINDOW;

			RECT windowRect = { 0, 0, 640, 480 };

			AdjustWindowRectEx(&windowRect, windowStyle | WS_CAPTION, 0, windowExtendedStyle);

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

	{
		std::vector<VkImage> vectorVkImages(0ULL);

		int iQueueFamilyIndex = -1;

		VkExtent2D currentExtent = {};

		{
			VkPhysicalDevice hVkPhysicalDevice = nullptr;

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
		}

		// Create Vk Pipeline Layout 
		{
			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
			pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutCreateInfo.setLayoutCount = 0;
			pipelineLayoutCreateInfo.pushConstantRangeCount = 0;

			vkCreatePipelineLayout(hVkDevice, &pipelineLayoutCreateInfo, nullptr, &hVkPipelineLayout);
		}

		// Create Vk Pipeline 
		{
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

			VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
			vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertexShaderStageInfo.module = hVertexVkShaderModule;
			vertexShaderStageInfo.pName = "main";

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

			VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
			fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragmentShaderStageInfo.module = hFragmentVkShaderModule;
			fragmentShaderStageInfo.pName = "main";

			VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
			vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

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
			VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageInfo, fragmentShaderStageInfo };
			graphicsPipelineCreateInfo.pStages = shaderStages;
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

		// Vertex Position 
		{
			float vVertexPositions[8][3] = {
				{ -1.0f, -1.0f, -1.0f },
				{ -1.0f, -1.0f,  1.0f },
				{ -1.0f,  1.0f, -1.0f },
				{ -1.0f,  1.0f,  1.0f },
				{ 1.0f, -1.0f, -1.0f },
				{ 1.0f, -1.0f,  1.0f },
				{ 1.0f,  1.0f, -1.0f },
				{ 1.0f,  1.0f,  1.0f },
			};
		}

		// Vertex Color 
		{
			float vVertexColors[8][3] = {
				{ 0.0f, 0.0f, 0.0f },
				{ 0.0f, 0.0f, 1.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 1.0f },
				{ 1.0f, 0.0f, 0.0f },
				{ 1.0f, 0.0f, 1.0f },
				{ 1.0f, 1.0f, 0.0f },
				{ 1.0f, 1.0f, 1.0f },
			};
		}

		// Index 
		{
			unsigned short indices[] = {
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
		}

		// Create Vk Command Pool 
		{
			VkCommandPoolCreateInfo commandPoolCreateInfo = {};
			commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			commandPoolCreateInfo.queueFamilyIndex = iQueueFamilyIndex;

			vkCreateCommandPool(hVkDevice, &commandPoolCreateInfo, nullptr, &hVkCommandPool);
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

#if 1
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
#endif // 1

				// vkCmdClearColorImage(vectorVkCommandBuffers[i], vectorVkImages[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColorValue, 1, &imageSubresourceRange);

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

#if 1
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
#endif // 1

				vkEndCommandBuffer(vectorVkCommandBuffers[i]);
			}
		}
	}

	// Create Vk Semaphores: Image Avalable & Rendering Finished 
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		vkCreateSemaphore(hVkDevice, &semaphoreCreateInfo, nullptr, &hVkSemaphoreImageAvalable);

		vkCreateSemaphore(hVkDevice, &semaphoreCreateInfo, nullptr, &hVkSemaphoreRenderFinished);
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
			vkAcquireNextImageKHR(hVkDevice, hVkSwapchainKHR, static_cast<uint64_t>(-1), hVkSemaphoreImageAvalable, 0, &uImageIndex);

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &hVkSemaphoreImageAvalable;
			VkPipelineStageFlags pipelineStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			submitInfo.pWaitDstStageMask = &pipelineStageFlags;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &vectorVkCommandBuffers[uImageIndex];
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &hVkSemaphoreRenderFinished;

			vkQueueSubmit(hVkQueue, 1, &submitInfo, 0);

			VkPresentInfoKHR presentInfoKHR = {};
			presentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfoKHR.waitSemaphoreCount = 1;
			presentInfoKHR.pWaitSemaphores = &hVkSemaphoreRenderFinished;
			presentInfoKHR.swapchainCount = 1;
			presentInfoKHR.pSwapchains = &hVkSwapchainKHR;
			presentInfoKHR.pImageIndices = &uImageIndex;

			vkQueuePresentKHR(hVkQueue, &presentInfoKHR);
		}

		Sleep(15);
	}

	vkDestroySemaphore(hVkDevice, hVkSemaphoreRenderFinished, nullptr);
	hVkSemaphoreRenderFinished = 0;

	vkDestroySemaphore(hVkDevice, hVkSemaphoreImageAvalable, nullptr);
	hVkSemaphoreImageAvalable = 0;

	vkFreeCommandBuffers(hVkDevice, hVkCommandPool, static_cast<uint32_t>(vectorVkCommandBuffers.size()), vectorVkCommandBuffers.data());
	vectorVkCommandBuffers.clear();

	vkDestroyCommandPool(hVkDevice, hVkCommandPool, nullptr);
	hVkCommandPool = 0;

	for (const VkFramebuffer& hVkFramebuffer : vectorVkFramebuffers)
	{
		vkDestroyFramebuffer(hVkDevice, hVkFramebuffer, nullptr);
	}
	vectorVkFramebuffers.clear();

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

