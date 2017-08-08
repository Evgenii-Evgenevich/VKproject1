#ifdef _WIN32

#define VK_USE_PLATFORM_WIN32_KHR 1

#include <vulkan/vulkan.h>

#if 0

VkResult VKAPI_CALL vkCreateWin32SurfaceKHR(
	VkInstance                                  instance,
	const VkWin32SurfaceCreateInfoKHR*          pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkSurfaceKHR*                               pSurface)
{
	static PFN_vkCreateWin32SurfaceKHR proc = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR"));
	if (proc)
	{
		return proc(instance, pCreateInfo, pAllocator, pSurface);
	}
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

#endif 

#endif // _WIN32

