#define VK_USE_PLATFORM_WIN32_KHR 1

#include <vulkan/vulkan.h>
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

	std::vector<VkImage> vectorVkImages(0);

	VkCommandPool hVkCommandPool = 0;

	std::vector<VkCommandBuffer> vectorVkCommandBuffers(0);

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
			
			PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(vkGetInstanceProcAddr(hVkInstance, "vkCreateWin32SurfaceKHR"));
			vkCreateWin32SurfaceKHR(hVkInstance, &win32SurfaceCreateInfoKHR, nullptr, &hVkSurfaceKHR);
		}
	}

	{
		int iQueueFamilyIndex = -1;

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

						if (uQueueFamilyPropertyCount)
						{
							VkQueueFamilyProperties* pAllVkQueueFamilyProperties = new VkQueueFamilyProperties[uQueueFamilyPropertyCount];
							vkGetPhysicalDeviceQueueFamilyProperties(hCurrentPhysicalDevice, &uQueueFamilyPropertyCount, pAllVkQueueFamilyProperties);

							for (int i = 0; i < uQueueFamilyPropertyCount; ++i)
							{
								VkBool32 bIsSupported = FALSE;
								vkGetPhysicalDeviceSurfaceSupportKHR(hCurrentPhysicalDevice, i, hVkSurfaceKHR, &bIsSupported);

								if (bIsSupported && pAllVkQueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
								{
									hVkPhysicalDevice = hCurrentPhysicalDevice;
									iQueueFamilyIndex = i;
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

			// Create Vk Swapchain 
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
				}

				VkSwapchainCreateInfoKHR swapchainCreateInfoKHR = {};
				swapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
				swapchainCreateInfoKHR.surface = hVkSurfaceKHR;
				swapchainCreateInfoKHR.minImageCount = surfaceCapabilitiesKHR.maxImageCount < 3 ? surfaceCapabilitiesKHR.maxImageCount : 3;
				swapchainCreateInfoKHR.imageFormat = surfaceFormatKHR.format;
				swapchainCreateInfoKHR.imageColorSpace = surfaceFormatKHR.colorSpace;
				swapchainCreateInfoKHR.imageExtent = surfaceCapabilitiesKHR.currentExtent;
				swapchainCreateInfoKHR.imageArrayLayers = 1;
				swapchainCreateInfoKHR.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				swapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				swapchainCreateInfoKHR.preTransform = surfaceCapabilitiesKHR.currentTransform;
				swapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
				swapchainCreateInfoKHR.presentMode = presentModeKHR;
				swapchainCreateInfoKHR.clipped = TRUE;

				vkCreateSwapchainKHR(hVkDevice, &swapchainCreateInfoKHR, nullptr, &hVkSwapchainKHR);
			}
		}

		// Get Vk Swapchain Images 
		{
			unsigned uSwapchainImageCount = 0;

			vkGetSwapchainImagesKHR(hVkDevice, hVkSwapchainKHR, &uSwapchainImageCount, nullptr);

			vectorVkImages.resize(uSwapchainImageCount);

			vkGetSwapchainImagesKHR(hVkDevice, hVkSwapchainKHR, &uSwapchainImageCount, vectorVkImages.data());
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
			commandBufferAllocateInfo.commandBufferCount = vectorVkImages.size();

			vectorVkCommandBuffers.resize(commandBufferAllocateInfo.commandBufferCount);

			vkAllocateCommandBuffers(hVkDevice, &commandBufferAllocateInfo, vectorVkCommandBuffers.data());
		}

		// Fill Vk Command Buffers 
		{
			VkCommandBufferBeginInfo commandBufferBeginInfo = {};
			commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

			VkClearColorValue clearColorValue = { 0.5f, 0.1f, 0.4f };

			VkViewport viewport = { 0.f, 0.f, 640.f, 480.f, 0.f, 1.f };

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

				vkCmdClearColorImage(vectorVkCommandBuffers[i], vectorVkImages[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColorValue, 1, &imageSubresourceRange);

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

	vkDestroySemaphore(hVkDevice, hVkSemaphoreImageAvalable, nullptr);
	hVkSemaphoreImageAvalable = 0;

	vkDestroySemaphore(hVkDevice, hVkSemaphoreRenderFinished, nullptr);
	hVkSemaphoreRenderFinished = 0;

	vkFreeCommandBuffers(hVkDevice, hVkCommandPool, vectorVkCommandBuffers.size(), vectorVkCommandBuffers.data());
	vectorVkCommandBuffers.clear();

	vkDestroyCommandPool(hVkDevice, hVkCommandPool, nullptr);
	hVkCommandPool = 0;

	for (const VkImage& hVkImage : vectorVkImages)
	{
		vkDestroyImage(hVkDevice, hVkImage, nullptr);
	}
	vectorVkImages.clear();

	// vkDestroySwapchainKHR(hVkDevice, hVkSwapchainKHR, nullptr);
	// hVkSwapchainKHR = 0;

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
