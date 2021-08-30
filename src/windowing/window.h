#ifndef STEROS_WINDOW_H
#define STEROS_WINDOW_H

// STD
#include <stdint.h>
#include <stdbool.h>

// LIB
#include "ntd/string.h"

// Vulkan
#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vulkan.h>

typedef struct strs_window_tag *strs_window;

struct strs_window_tag {
  uint8_t not_used;
};

STRS_LIB strs_window strs_window_create(uint64_t width, uint64_t height, strs_string *title);
STRS_LIB bool strs_window_closing(strs_window window);
STRS_LIB void strs_window_get_size(strs_window window, uint64_t *width, uint64_t *height);
STRS_LIB void strs_window_wait_events(strs_window window);
STRS_LIB void strs_window_poll_events(strs_window window);
STRS_LIB VkResult strs_window_create_vulkan_surface(
  strs_window window, VkInstance instance,
  VkSurfaceKHR *pSurface, const VkAllocationCallbacks *pAllocator);
STRS_LIB const char **strs_window_get_required_instance_extensions(strs_window window, uint32_t *count);
STRS_LIB void strs_window_set_user_pointer(strs_window window, void *pointer);
STRS_LIB void *strs_window_get_user_pointer(strs_window window);
STRS_LIB void strs_window_free(strs_window window);

#endif //STEROS_WINDOW_H
