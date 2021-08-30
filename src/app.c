// STD
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

// LIB
#include "app.h"
#include "ntd/string.h"

#define IMPL_OPTION_DEF
#include "helper/option.h"

// Vendor
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <cglm/affine.h>

#ifndef NDEBUG
#define dbg_assert(x) assert(x)
static bool enable_validation_layers = true;
#else
#define dbg_assert
static bool enable_validation_layers = false;
#endif

typedef struct  {
  strs_vertex *vertices;
  uint64_t vertex_count;
  uint16_t *indices;
  uint64_t index_count;

  strs_widget *widgets;

  strs_window window;

  VkInstance instance;
  VkSurfaceKHR surface;
  VkPhysicalDevice physical_device;

  VkDevice logical_device;
  VkQueue present_queue;
  VkQueue graphics_queue;
  VkSwapchainKHR swap_chain;
  VkImage *swap_chain_images;

  uint32_t number_of_images;
  VkFormat swap_chain_image_format;
  VkExtent2D swap_chain_extent;
  VkImageView *swap_chain_image_views;
  VkRenderPass render_pass;

  VkShaderModule vert_shader_module;
  VkShaderModule frag_shader_module;

  VkDescriptorSetLayout descriptor_set_layout;

  pipeline_config_info pipeline_config;
  VkPipelineLayout pipeline_layout;
  VkPipeline pipeline;
  VkFramebuffer *swap_chain_frame_buffers;
  VkCommandPool command_pool;

  VkDescriptorPool descriptor_pool;
  VkDescriptorSet *descriptor_sets;

  VkCommandBuffer *command_buffers;

  VkSemaphore *image_available_semaphores;
  VkSemaphore *render_finished_semaphores;
  VkFence *in_flight_fences;
  VkFence *images_in_flight;

  vulkan_buffer vertex_buffer;
  vulkan_buffer index_buffer;

  VkBuffer *uniform_buffers;
  VkDeviceMemory *uniform_buffers_memory;

  VkImage texture_image;
  VkDeviceMemory texture_image_memory;

  size_t current_frame;
  bool frame_buffer_resized;

  // PThread
  pthread_t thread;
} internal_strs_app;

typedef struct {
  mat4 model;
  mat4 view;
  mat4 proj;
} UniformBufferObject;

typedef struct {
  VkSurfaceCapabilitiesKHR capabilities;
  VkSurfaceFormatKHR *formats;
  uint32_t formatsSize;
  VkPresentModeKHR *presentModes;
  uint32_t presentModesSize;
} SwapChainSupportDetails;

typedef struct {
  option_uint graphics_family;
  option_uint present_family;
} QueueFamilyIndices;

static bool init = false;
static const char *validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
static const char *device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
const int MAX_FRAMES_IN_FLIGHT = 2;

static float time_passed;

STRS_INTERN void *multithread_create_app(void *data);

STRS_INTERN void draw_frame(internal_strs_app *app);
STRS_INTERN void recreate_swap_chain(internal_strs_app *app);
STRS_INTERN void cleanup_swap_chain(internal_strs_app *app);
STRS_INTERN void create_instance(internal_strs_app *app);
STRS_INTERN void create_surface(internal_strs_app *app);
STRS_INTERN void pick_physical_device(internal_strs_app *app);
STRS_INTERN void create_logical_device(internal_strs_app *app);
STRS_INTERN void create_swap_chain(internal_strs_app *app);
STRS_INTERN void create_image_views(internal_strs_app *app);
STRS_INTERN void create_render_pass(internal_strs_app *app);
STRS_INTERN void create_shader_modules(internal_strs_app *app);
STRS_INTERN void create_descriptor_set_layout(internal_strs_app *app);
STRS_INTERN void create_graphics_pipeline(internal_strs_app *app);
STRS_INTERN void create_frame_buffers(internal_strs_app *app);
STRS_INTERN void create_command_pool(internal_strs_app *app);
STRS_INTERN void create_texture_image(internal_strs_app *app);
STRS_INTERN void create_vertex_buffer(internal_strs_app *app);
STRS_INTERN void create_index_buffer(internal_strs_app *app);
STRS_INTERN void create_uniform_buffers(internal_strs_app *app);
STRS_INTERN void create_descriptor_pool(internal_strs_app *app);
STRS_INTERN void create_descriptor_sets(internal_strs_app *app);
STRS_INTERN void create_command_buffers(internal_strs_app *app);
STRS_INTERN void create_sync_objects(internal_strs_app *app);

STRS_INTERN void update_uniform_buffers(internal_strs_app *app, uint32_t current_image);
STRS_INTERN void update_vertex_buffer(internal_strs_app *app);
STRS_INTERN void update_index_buffer(internal_strs_app *app);

STRS_INTERN void destroy_buffer(internal_strs_app *app, vulkan_buffer* buffer);

STRS_INTERN VkVertexInputBindingDescription get_binding_description();
STRS_INTERN inline vk_vertex_input_attribute_description_array get_attribute_descriptions();

STRS_INTERN uint32_t find_memory_type(VkPhysicalDevice physical_device,
                                      uint32_t type_filter,
                                      VkMemoryPropertyFlags properties);

STRS_INTERN char *read_shader(const char *filename, long *size);
STRS_INTERN void swap_chain_support_details_free(SwapChainSupportDetails *ptr);
STRS_INTERN uint32_t clamp_uint(uint32_t d, uint32_t min, uint32_t max);
STRS_INTERN QueueFamilyIndices find_queue_family_indices(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
STRS_INTERN bool queue_family_indices_is_complete(QueueFamilyIndices *indices);
STRS_INTERN bool check_device_extension_support(VkPhysicalDevice device);
STRS_INTERN SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR surface);
STRS_INTERN void create_buffer(internal_strs_app *app, VkDeviceSize size,
                               VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                               VkBuffer *buffer, VkDeviceMemory *bufferMemory);
STRS_INTERN void copy_buffer(internal_strs_app *app, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
STRS_INTERN void fill_config_info(internal_strs_app *app);
void endSingleTimeCommands(internal_strs_app *app, VkCommandBuffer commandBuffer);
VkCommandBuffer beginSingleTimeCommands(internal_strs_app *app);
void createImage(internal_strs_app *app, uint32_t width, uint32_t height,
                 VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                 VkMemoryPropertyFlags properties, VkImage *image,
                 VkDeviceMemory *imageMemory);

STRS_INTERN inline double degrees_to_radians(double degrees) {
  return degrees * M_PI / 180.0;
}

STRS_INTERN inline vk_vertex_input_attribute_description_array get_attribute_descriptions() {
  vk_vertex_input_attribute_description_array attribute_descriptions;
  attribute_descriptions = (vk_vertex_input_attribute_description_array){
    .array = {
      {.binding = 0,
        .location = 0,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offsetof(strs_vertex, pos)},
      {.binding = 0,
        .location = 1,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(strs_vertex, color)}},
    .size = 2};

  return attribute_descriptions;
}

STRS_INTERN VkVertexInputBindingDescription get_binding_description() {
  VkVertexInputBindingDescription bindingDescription;

  bindingDescription = (VkVertexInputBindingDescription){
    .binding = 0,
    .stride = sizeof(strs_vertex),
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};

  return bindingDescription;
}

STRS_INTERN uint32_t find_memory_type(VkPhysicalDevice physical_device,
                                      uint32_t type_filter,
                                      VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties mem_properties;
  vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

  for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
    if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  exit(-1);
}

STRS_INTERN uint32_t clamp_uint(uint32_t d, uint32_t min, uint32_t max) {
  const uint32_t t = d < min ? min : d;
  return t > max ? max : t;
}

STRS_INTERN char *read_shader(const char *filename, long *size) {
  FILE *fp;
  struct stat sb;
  char *file_contents;

  fp = fopen(filename, "rb");
  if (fp == NULL) {
    fprintf(stderr, "cannot open input file\n");
    return NULL;
  }

  if (stat(filename, &sb) == -1) {
    perror("error");
    exit(-1);
  }

  file_contents = malloc(sizeof(char) * sb.st_size);
  fread(file_contents, sb.st_size, 1, fp);

  fclose(fp);
  *size = sb.st_size;
  return file_contents;
}

STRS_INTERN void swap_chain_support_details_free(SwapChainSupportDetails *ptr) {
  free(ptr->presentModes);
  free(ptr->formats);
  ptr = NULL;
}

STRS_INTERN bool queue_family_indices_is_complete(QueueFamilyIndices *indices) {
  return indices->graphics_family.has_value && indices->present_family.has_value;
}

STRS_INTERN QueueFamilyIndices find_queue_family_indices(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
  QueueFamilyIndices queue_family_indices;
  uint32_t queue_family_count;
  VkQueueFamilyProperties *queue_families;
  VkBool32 presentSupport;

  option_uint_create(&queue_family_indices.graphics_family);

  queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);

  queue_families = malloc(sizeof(VkQueueFamilyProperties) * queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);

  for (int i = 0; i < queue_family_count; i++) {
    if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      option_uint_set_value(&queue_family_indices.graphics_family, i);
    }

    presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &presentSupport);

    if (presentSupport) {
      option_uint_set_value(&queue_family_indices.present_family, i);
    }

    if (queue_family_indices_is_complete(&queue_family_indices)) {
      break;
    }
  }
  free(queue_families);
  return queue_family_indices;
}

STRS_INTERN bool check_device_extension_support(VkPhysicalDevice device) { // TODO: Finish this function
  /*uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

  VkExtensionProperties *availableExtensions = malloc(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);

  for(int i = 0; i < extensionCount; i++) {
      if (device_extensions[0] == availableExtensions[i].extensionName) {
          free(availableExtensions);
          return true;
      }
  }

  free(availableExtensions);
  return false;*/

  return true;
}

STRS_INTERN SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR surface) {
  SwapChainSupportDetails details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL);

  if (formatCount != 0) {
    details.formatsSize = formatCount;
    details.formats = malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats);
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, NULL);

  if (presentModeCount != 0) {
    details.presentModesSize = presentModeCount;
    details.presentModes = malloc(sizeof(uint32_t) * presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes);
  }

  return details;
}

STRS_INTERN bool is_device_suitable(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
  QueueFamilyIndices queueFamilyIndices = find_queue_family_indices(physical_device, surface);

  bool extensionsSupported = check_device_extension_support(physical_device);

  bool swapChainAdequate = false;
  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport = query_swap_chain_support(physical_device, surface);
    swapChainAdequate = swapChainSupport.formats != NULL && swapChainSupport.presentModes != NULL;
    swap_chain_support_details_free(&swapChainSupport);
  }

  return queue_family_indices_is_complete(&queueFamilyIndices) && swapChainAdequate;
}

STRS_INTERN VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkSurfaceFormatKHR *formats, uint32_t sizeOfArray) {
  for (uint32_t i = 0; i < sizeOfArray; i++) {
    if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return formats[i];
    }
  }

  return formats[0];
}

STRS_INTERN VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR *presentModes, uint32_t sizeOfArray) {
  for (int i = 0; i < sizeOfArray; i++) {
    if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
      return presentModes[i];
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

STRS_INTERN VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR capabilities, strs_window window) {
  if (capabilities.currentExtent.width != UINT32_MAX) {
    return capabilities.currentExtent;
  } else {
    uint64_t width, height;
    strs_window_get_size(window, &width, &height);

    VkExtent2D actualExtent = {
      width,
      height};

    actualExtent.width =
      clamp_uint(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height =
      clamp_uint(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
  }
}

STRS_INTERN void create_buffer(internal_strs_app *app, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                               VkBuffer *buffer, VkDeviceMemory *bufferMemory) {
  VkBufferCreateInfo bufferInfo = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = size,
    .usage = usage,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

  VkResult result = vkCreateBuffer(app->logical_device, &bufferInfo, NULL, buffer);
  dbg_assert(result == VK_SUCCESS);

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(app->logical_device, *buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize = memRequirements.size,
    .memoryTypeIndex = find_memory_type(app->physical_device, memRequirements.memoryTypeBits, properties)};

  result = vkAllocateMemory(app->logical_device, &allocInfo, NULL, bufferMemory);
  dbg_assert(result == VK_SUCCESS);

  vkBindBufferMemory(app->logical_device, *buffer, *bufferMemory, 0);
}

STRS_INTERN void copy_buffer(internal_strs_app *app, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
  VkCommandBufferAllocateInfo allocInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandPool = app->command_pool,
    .commandBufferCount = 1};

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(app->logical_device, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  VkBufferCopy copyRegion = {
    .size = size};

  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .commandBufferCount = 1,
    .pCommandBuffers = &commandBuffer};

  vkQueueSubmit(app->graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(app->graphics_queue);

  vkFreeCommandBuffers(app->logical_device, app->command_pool, 1, &commandBuffer);
}

STRS_INTERN void create_sync_objects(internal_strs_app *app) {
  app->image_available_semaphores = malloc(sizeof(VkSemaphore *) * MAX_FRAMES_IN_FLIGHT);
  app->render_finished_semaphores = malloc(sizeof(VkSemaphore *) * MAX_FRAMES_IN_FLIGHT);
  app->in_flight_fences = malloc(sizeof(VkFence *) * MAX_FRAMES_IN_FLIGHT);
  app->images_in_flight = malloc(sizeof(VkFence *) * app->number_of_images);

  VkSemaphoreCreateInfo semaphoreInfo = {
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

  VkFenceCreateInfo fenceInfo = {
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .flags = VK_FENCE_CREATE_SIGNALED_BIT};

  VkResult result;
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    result = vkCreateSemaphore(app->logical_device,
                               &semaphoreInfo, NULL, &app->image_available_semaphores[i]);
    dbg_assert(result == VK_SUCCESS);
    result = vkCreateSemaphore(app->logical_device,
                               &semaphoreInfo, NULL, &app->render_finished_semaphores[i]);
    dbg_assert(result == VK_SUCCESS);
    result = vkCreateFence(app->logical_device,
                           &fenceInfo, NULL, &app->in_flight_fences[i]);
    dbg_assert(result == VK_SUCCESS);
  }
  for (int i = 0; i < app->number_of_images; i++) {
    app->images_in_flight[i] = 0;
  }
}

STRS_INTERN void create_command_buffers(internal_strs_app *app) {
  app->command_buffers = malloc(sizeof(VkCommandBuffer *) * app->number_of_images);

  VkCommandBufferAllocateInfo allocInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = app->command_pool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = app->number_of_images};

  VkResult result = vkAllocateCommandBuffers(app->logical_device, &allocInfo, app->command_buffers);
  dbg_assert(result == VK_SUCCESS);

  for (size_t i = 0; i < app->number_of_images; i++) {
    VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

    result = vkBeginCommandBuffer(app->command_buffers[i], &beginInfo);
    dbg_assert(result == VK_SUCCESS);

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

    VkRenderPassBeginInfo renderPassInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = app->render_pass,
      .framebuffer = app->swap_chain_frame_buffers[i],
      .renderArea.offset = {0, 0},
      .renderArea.extent = app->swap_chain_extent,
      .clearValueCount = 1,
      .pClearValues = &clearColor};

    vkCmdBeginRenderPass(app->command_buffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {
      .x = 0.0f,
      .y = 0.0f,
      .width = app->swap_chain_extent.width,
      .height = app->swap_chain_extent.height,
      .minDepth = 0.0f,
      .maxDepth = 0.0f};

    VkRect2D scissor = {
      .offset = {0, 0},
      .extent = app->swap_chain_extent};

    vkCmdSetViewport(app->command_buffers[i], 0, 1, &viewport);
    vkCmdSetScissor(app->command_buffers[i], 0, 1, &scissor);

    vkCmdBindPipeline(app->command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, app->pipeline);

    VkBuffer vertexBuffers[] = {app->vertex_buffer.buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(app->command_buffers[i], 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(app->command_buffers[i], app->index_buffer.buffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(app->command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                            app->pipeline_layout,
                            0,
                            1,
                            &app->descriptor_sets[i], 0, NULL);
    vkCmdDrawIndexed(app->command_buffers[i],
                     app->index_buffer.bufferSize / sizeof(uint16_t),
                     1, 0, 0, 0);

    vkCmdEndRenderPass(app->command_buffers[i]);

    result = vkEndCommandBuffer(app->command_buffers[i]);
    dbg_assert(result == VK_SUCCESS);
  }
}

STRS_INTERN void create_uniform_buffers(internal_strs_app *app) {
  VkDeviceSize bufferSize = sizeof(UniformBufferObject);

  app->uniform_buffers = malloc(sizeof(VkBuffer *) * app->number_of_images);
  app->uniform_buffers_memory = malloc(sizeof(VkDeviceMemory *) * app->number_of_images);

  for (size_t i = 0; i < app->number_of_images; i++) {
    create_buffer(app, bufferSize,
                  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  &app->uniform_buffers[i], &app->uniform_buffers_memory[i]);
  }
}

STRS_INTERN void create_index_buffer(internal_strs_app *app) {
  app->index_buffer.bufferSize = sizeof(uint16_t) * app->index_count;

  create_buffer(app, app->index_buffer.bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &app->index_buffer.stagingBuffer, &app->index_buffer.stagingBufferMemory);

  vkMapMemory(app->logical_device,
              app->index_buffer.stagingBufferMemory,
              0, app->index_buffer.bufferSize, 0, &app->index_buffer.data);
  memcpy(app->index_buffer.data, app->indices, (size_t) app->index_buffer.bufferSize);

  create_buffer(app, app->index_buffer.bufferSize,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                &app->index_buffer.buffer, &app->index_buffer.bufferMemory);

  copy_buffer(app, app->index_buffer.stagingBuffer, app->index_buffer.buffer, app->index_buffer.bufferSize);
}

STRS_INTERN void create_vertex_buffer(internal_strs_app *app) {
  app->vertex_buffer.bufferSize = sizeof(strs_vertex) * app->vertex_count;

  create_buffer(app, app->vertex_buffer.bufferSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &app->vertex_buffer.stagingBuffer, &app->vertex_buffer.stagingBufferMemory);

  vkMapMemory(app->logical_device,
              app->vertex_buffer.stagingBufferMemory,
              0, app->vertex_buffer.bufferSize, 0, &app->vertex_buffer.data);
  memcpy(app->vertex_buffer.data, app->vertices, (size_t) app->vertex_buffer.bufferSize);

  create_buffer(app, app->vertex_buffer.bufferSize,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                &app->vertex_buffer.buffer, &app->vertex_buffer.bufferMemory);

  copy_buffer(app, app->vertex_buffer.stagingBuffer, app->vertex_buffer.buffer, app->vertex_buffer.bufferSize);
}

STRS_INTERN void create_command_pool(internal_strs_app *app) {
  QueueFamilyIndices queueFamilyIndices = find_queue_family_indices(app->physical_device, app->surface);

  VkCommandPoolCreateInfo poolInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .queueFamilyIndex = queueFamilyIndices.graphics_family.value};

  VkResult result = vkCreateCommandPool(app->logical_device, &poolInfo, NULL, &app->command_pool);
  dbg_assert(result == VK_SUCCESS);
}

STRS_INTERN void create_frame_buffers(internal_strs_app *app) {
  app->swap_chain_frame_buffers = malloc(sizeof(VkFramebuffer *) * app->number_of_images);

  for (size_t i = 0; i < app->number_of_images; i++) {
    VkImageView attachments[] = {
      app->swap_chain_image_views[i]};

    VkFramebufferCreateInfo framebufferInfo = {
      .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass = app->render_pass,
      .attachmentCount = 1,
      .pAttachments = attachments,
      .width = app->swap_chain_extent.width,
      .height = app->swap_chain_extent.height,
      .layers = 1};

    VkResult result =
      vkCreateFramebuffer(app->logical_device,
                          &framebufferInfo,
                          NULL,
                          &app->swap_chain_frame_buffers[i]);
    dbg_assert(result == VK_SUCCESS);
  }
}

STRS_INTERN void create_graphics_pipeline(internal_strs_app *app) {

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = 1,
    .pSetLayouts = &app->descriptor_set_layout,
    .pushConstantRangeCount = 0,
  };

  VkResult result =
    vkCreatePipelineLayout(app->logical_device,
                           &pipelineLayoutInfo,
                           NULL, &app->pipeline_layout);
  dbg_assert(result == VK_SUCCESS);

  VkGraphicsPipelineCreateInfo pipelineInfo = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .stageCount = 2,
    .pStages = app->pipeline_config.shader_stages,
    .pVertexInputState = &app->pipeline_config.vertex_input_info,
    .pInputAssemblyState = &app->pipeline_config.input_assembly,
    .pViewportState = &app->pipeline_config.viewport_state,
    .pRasterizationState = &app->pipeline_config.rasterizer,
    .pMultisampleState = &app->pipeline_config.multisampling,
    .pColorBlendState = &app->pipeline_config.color_blending,
    .layout = app->pipeline_layout,
    .renderPass = app->render_pass,
    .pDynamicState = &app->pipeline_config.dynamic_state_info,
    .subpass = 0,
    .basePipelineHandle = NULL,
  };

  result =
    vkCreateGraphicsPipelines(
      app->logical_device,
      NULL,
      1,
      &pipelineInfo,
      NULL,
      &app->pipeline);
  dbg_assert(result == VK_SUCCESS);
}

STRS_INTERN void fill_config_info(internal_strs_app *app) {
  app->pipeline_config.shader_stages[0] = (VkPipelineShaderStageCreateInfo) {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_VERTEX_BIT,
    .module = app->vert_shader_module,
    .pName = "main"};
  app->pipeline_config.shader_stages[1] = (VkPipelineShaderStageCreateInfo) {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
    .module = app->frag_shader_module,
    .pName = "main"};

  app->pipeline_config.binding_description = get_binding_description();
  app->pipeline_config.attribute_descriptions = get_attribute_descriptions();

  app->pipeline_config.vertex_input_info = (VkPipelineVertexInputStateCreateInfo) {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 1,
    .pVertexBindingDescriptions = &app->pipeline_config.binding_description,
    .vertexAttributeDescriptionCount = app->pipeline_config.attribute_descriptions.size,
    .pVertexAttributeDescriptions = app->pipeline_config.attribute_descriptions.array};

  app->pipeline_config.input_assembly = (VkPipelineInputAssemblyStateCreateInfo) {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE};

  app->pipeline_config.viewport_state = (VkPipelineViewportStateCreateInfo) {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .scissorCount = 1,
  };

  app->pipeline_config.rasterizer = (VkPipelineRasterizationStateCreateInfo) {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .depthClampEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .lineWidth = 1.0f,
    .cullMode = VK_CULL_MODE_BACK_BIT,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .depthBiasEnable = VK_FALSE};

  app->pipeline_config.multisampling = (VkPipelineMultisampleStateCreateInfo) {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .sampleShadingEnable = VK_FALSE,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT};

  app->pipeline_config.color_blend_attachment = (VkPipelineColorBlendAttachmentState) {
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                      VK_COLOR_COMPONENT_G_BIT |
                      VK_COLOR_COMPONENT_B_BIT |
                      VK_COLOR_COMPONENT_A_BIT,
    .blendEnable = VK_FALSE};

  app->pipeline_config.color_blending = (VkPipelineColorBlendStateCreateInfo) {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .logicOpEnable = VK_FALSE,
    .logicOp = VK_LOGIC_OP_COPY,
    .attachmentCount = 1,
    .pAttachments = &app->pipeline_config.color_blend_attachment,
    .blendConstants[0] = 0.0f,
    .blendConstants[1] = 0.0f,
    .blendConstants[2] = 0.0f,
    .blendConstants[3] = 0.0f};

  app->pipeline_config.dynamic_state_enables[0] = VK_DYNAMIC_STATE_VIEWPORT;
  app->pipeline_config.dynamic_state_enables[1] = VK_DYNAMIC_STATE_SCISSOR;
  app->pipeline_config.dynamic_state_count = 2;

  app->pipeline_config.dynamic_state_info = (VkPipelineDynamicStateCreateInfo) {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = app->pipeline_config.dynamic_state_count,
    .pDynamicStates = app->pipeline_config.dynamic_state_enables,
  };
}

STRS_INTERN void create_descriptor_set_layout(internal_strs_app *app) {
  VkDescriptorSetLayoutBinding uboLayoutBinding = {
    .binding = 0,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    .pImmutableSamplers = NULL,
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT};

  VkDescriptorSetLayoutCreateInfo layoutInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .bindingCount = 1,
    .pBindings = &uboLayoutBinding};

  VkResult result = vkCreateDescriptorSetLayout(app->logical_device,
                                                &layoutInfo,
                                                NULL,
                                                &app->descriptor_set_layout);
  dbg_assert(result == VK_SUCCESS);
}

STRS_INTERN void create_shader_modules(internal_strs_app *app) {
  long vertSize = 0;
  long fragSize = 0;
  char *vertShaderCode = read_shader("shaders/shader.vert.spv", &vertSize);
  char *fragShaderCode = read_shader("shaders/shader.frag.spv", &fragSize);

  VkShaderModuleCreateInfo vertModuleCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .codeSize = vertSize,
    .pCode = (const uint32_t *) vertShaderCode};
  VkShaderModuleCreateInfo fragModuleCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .codeSize = fragSize,
    .pCode = (const uint32_t *) fragShaderCode};

  VkResult result;
  result = vkCreateShaderModule(app->logical_device, &vertModuleCreateInfo, NULL, &app->vert_shader_module);
  dbg_assert(result == VK_SUCCESS);
  result = vkCreateShaderModule(app->logical_device, &fragModuleCreateInfo, NULL, &app->frag_shader_module);
  dbg_assert(result == VK_SUCCESS);

  free(vertShaderCode);
  free(fragShaderCode);
}

STRS_INTERN void create_render_pass(internal_strs_app *app) {
  VkAttachmentDescription colorAttachment = {
    .format = app->swap_chain_image_format,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

  VkAttachmentReference colorAttachmentRef = {
    .attachment = 0,
    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

  VkSubpassDescription subpass = {
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .colorAttachmentCount = 1,
    .pColorAttachments = &colorAttachmentRef};

  VkSubpassDependency dependency = {
    .srcSubpass = VK_SUBPASS_EXTERNAL,
    .dstSubpass = 0,
    .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .srcAccessMask = 0,
    .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT};

  VkRenderPassCreateInfo renderPassInfo = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .attachmentCount = 1,
    .pAttachments = &colorAttachment,
    .subpassCount = 1,
    .pSubpasses = &subpass,
    .dependencyCount = 1,
    .pDependencies = &dependency};

  VkResult result =
    vkCreateRenderPass(app->logical_device, &renderPassInfo, NULL, &app->render_pass);
  dbg_assert(result == VK_SUCCESS);
}

STRS_INTERN void create_image_views(internal_strs_app *app) {
  app->swap_chain_image_views = malloc(sizeof(VkImageView *) * app->number_of_images);

  for (size_t i = 0; i < app->number_of_images; i++) {
    VkImageViewCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = app->swap_chain_images[i],
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = app->swap_chain_image_format,
      .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
      .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
      .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
      .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
      .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .subresourceRange.baseMipLevel = 0,
      .subresourceRange.levelCount = 1,
      .subresourceRange.baseArrayLayer = 0,
      .subresourceRange.layerCount = 1};

    VkResult result =
      vkCreateImageView(app->logical_device, &createInfo, NULL, &app->swap_chain_image_views[i]);
    dbg_assert(result == VK_SUCCESS);
  }
}

STRS_INTERN void create_swap_chain(internal_strs_app *app) {
  SwapChainSupportDetails swapChainSupport = query_swap_chain_support(app->physical_device, app->surface);

  VkSurfaceFormatKHR surfaceFormat =
    chooseSwapSurfaceFormat(swapChainSupport.formats, swapChainSupport.formatsSize);
  VkPresentModeKHR presentMode =
    chooseSwapPresentMode(swapChainSupport.presentModes, swapChainSupport.presentModesSize);
  VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, app->window);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo = {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = app->surface,
    .minImageCount = imageCount,
    .imageFormat = surfaceFormat.format,
    .imageColorSpace = surfaceFormat.colorSpace,
    .imageExtent = extent,
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT};

  QueueFamilyIndices familyIndices = find_queue_family_indices(app->physical_device, app->surface);
  uint32_t queueFamilyIndices[] = {familyIndices.graphics_family.value, familyIndices.present_family.value};

  if (familyIndices.graphics_family.value != familyIndices.present_family.value) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  createInfo.oldSwapchain = app->swap_chain == NULL ? VK_NULL_HANDLE : app->swap_chain;

  VkResult result = vkCreateSwapchainKHR(app->logical_device, &createInfo, NULL, &app->swap_chain);
  dbg_assert(result == VK_SUCCESS);

  vkGetSwapchainImagesKHR(app->logical_device, app->swap_chain, &imageCount, NULL);
  app->swap_chain_images = malloc(sizeof(VkImage *) * imageCount);
  vkGetSwapchainImagesKHR(app->logical_device, app->swap_chain, &imageCount, app->swap_chain_images);

  app->swap_chain_image_format = surfaceFormat.format;
  app->swap_chain_extent = extent;
  app->number_of_images = imageCount;

  swap_chain_support_details_free(&swapChainSupport);
}

STRS_INTERN void create_logical_device(internal_strs_app *app) {
  QueueFamilyIndices queueFamilyIndices = find_queue_family_indices(app->physical_device, app->surface);

  VkDeviceQueueCreateInfo *queueCreateInfos;

  float queuePriority = 1.0f;
  int sizeOfCreateInfo;
  if (queueFamilyIndices.graphics_family.value == queueFamilyIndices.present_family.value) {
    VkDeviceQueueCreateInfo queueCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = queueFamilyIndices.graphics_family.value,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority};
    sizeOfCreateInfo = 1;
    queueCreateInfos = malloc(sizeof(VkDeviceQueueCreateInfo) * sizeOfCreateInfo);
    queueCreateInfos[0] = queueCreateInfo;
  } else {
    VkDeviceQueueCreateInfo graphicsQueueCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = queueFamilyIndices.present_family.value,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority};
    VkDeviceQueueCreateInfo presentQueueCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = queueFamilyIndices.present_family.value,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority};
    sizeOfCreateInfo = 2;
    queueCreateInfos = malloc(sizeof(VkDeviceQueueCreateInfo) * sizeOfCreateInfo);
    queueCreateInfos[0] = graphicsQueueCreateInfo;
    queueCreateInfos[1] = presentQueueCreateInfo;
  }

  VkPhysicalDeviceFeatures deviceFeatures = {0};

  VkDeviceCreateInfo createInfo = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .queueCreateInfoCount = sizeOfCreateInfo,
    .pQueueCreateInfos = queueCreateInfos,
    .pEnabledFeatures = &deviceFeatures};

  createInfo.enabledExtensionCount = sizeof(device_extensions) / sizeof(const char *);
  createInfo.ppEnabledExtensionNames = device_extensions;

  if (enable_validation_layers) {
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = validation_layers;
  }

  VkResult result = vkCreateDevice(app->physical_device, &createInfo, NULL, &app->logical_device);
  dbg_assert(result == VK_SUCCESS);
  vkGetDeviceQueue(app->logical_device, queueFamilyIndices.graphics_family.value, 0, &app->graphics_queue);
  vkGetDeviceQueue(app->logical_device, queueFamilyIndices.present_family.value, 0, &app->present_queue);

  free(queueCreateInfos);
}

STRS_INTERN void pick_physical_device(internal_strs_app *app) {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(app->instance, &deviceCount, NULL);
  dbg_assert(deviceCount != 0);

  VkPhysicalDevice *physicalDevices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
  vkEnumeratePhysicalDevices(app->instance, &deviceCount, physicalDevices);

  for (int i = 0; i < deviceCount; i++) {
    if (is_device_suitable(physicalDevices[i], app->surface)) {
      app->physical_device = physicalDevices[i];
      free(physicalDevices);
      return;
    }
  }

  printf("No suitable GPU found, sorry\n");
  exit(-1);
}

STRS_INTERN void create_surface(internal_strs_app *app) {
  strs_window_create_vulkan_surface(app->window, app->instance, &app->surface, NULL);
}

STRS_INTERN void create_instance(internal_strs_app *app) {
  VkApplicationInfo appInfo = {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pNext = NULL,
    .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
    .pApplicationName = "Steros",
    .engineVersion = VK_MAKE_VERSION(1, 0, 0),
    .pEngineName = "Steros",
    .apiVersion = VK_API_VERSION_1_2};

  uint32_t instance_extension_count = 0;
  const char **instance_extensions =
    strs_window_get_required_instance_extensions(app->window, &instance_extension_count);

  VkInstanceCreateInfo instanceInfo = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pNext = NULL,
    .flags = 0,
    .pApplicationInfo = &appInfo,
    .enabledExtensionCount = instance_extension_count,
    .ppEnabledExtensionNames = instance_extensions,
  };

  if (enable_validation_layers) {
    instanceInfo.enabledLayerCount = 1;
    instanceInfo.ppEnabledLayerNames = validation_layers;
  }

  VkResult result = vkCreateInstance(&instanceInfo, NULL, &app->instance);
  dbg_assert(result == VK_SUCCESS);
}

STRS_INTERN void cleanup_swap_chain(internal_strs_app *app) {
  for (size_t i = 0; i < app->number_of_images; i++) {
    vkDestroyFramebuffer(app->logical_device, app->swap_chain_frame_buffers[i], NULL);
    vkDestroyBuffer(app->logical_device, app->uniform_buffers[i], NULL);
    vkFreeMemory(app->logical_device, app->uniform_buffers_memory[i], NULL);
  }

  vkDestroyDescriptorPool(app->logical_device, app->descriptor_pool, NULL);

  vkFreeCommandBuffers(app->logical_device, app->command_pool, app->number_of_images, app->command_buffers);

  vkDestroyPipeline(app->logical_device, app->pipeline, NULL);
  vkDestroyPipelineLayout(app->logical_device, app->pipeline_layout, NULL);
  vkDestroyRenderPass(app->logical_device, app->render_pass, NULL);

  for (size_t i = 0; i < app->number_of_images; i++) {
    vkDestroyImageView(app->logical_device, app->swap_chain_image_views[i], NULL);
  }

  vkDestroySwapchainKHR(app->logical_device, app->swap_chain, NULL);

  free(app->swap_chain_frame_buffers);
  free(app->swap_chain_image_views);
  free(app->swap_chain_images);

  free(app->uniform_buffers_memory);
  free(app->uniform_buffers);

  app->swap_chain = NULL;
}

STRS_INTERN void recreate_swap_chain(internal_strs_app *app) {
  uint64_t width = 0, height = 0;
  strs_window_get_size(app->window, &width, &height);
  while (width == 0 || height == 0) {
    strs_window_get_size(app->window, &width, &height);
    strs_window_wait_events(app->window);
  }
  vkDeviceWaitIdle(app->logical_device);

  cleanup_swap_chain(app);

  create_swap_chain(app);
  create_image_views(app);
  create_render_pass(app);
  create_graphics_pipeline(app);
  create_frame_buffers(app);
  create_uniform_buffers(app);
  create_descriptor_pool(app);
  create_descriptor_sets(app);
  create_command_buffers(app);
}

STRS_INTERN void draw_frame(internal_strs_app *app) {
  vkWaitForFences(app->logical_device, 1, &app->in_flight_fences[app->current_frame], VK_TRUE, UINT64_MAX);

  uint32_t imageIndex = 0;
  VkResult result = vkAcquireNextImageKHR(
    app->logical_device,
    app->swap_chain,
    UINT64_MAX,
    app->image_available_semaphores[app->current_frame],
    VK_NULL_HANDLE,
    &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreate_swap_chain(app);
    return;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    exit(-1);
  }

  if (app->images_in_flight[imageIndex] != VK_NULL_HANDLE) {
    vkWaitForFences(app->logical_device, 1, &app->images_in_flight[imageIndex], VK_TRUE, UINT64_MAX);
  }
  app->images_in_flight[imageIndex] = app->in_flight_fences[app->current_frame];

  VkSemaphore waitSemaphores[] = {app->image_available_semaphores[app->current_frame]};
  VkSemaphore signalSemaphores[] = {app->render_finished_semaphores[app->current_frame]};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

  update_uniform_buffers(app, imageIndex);
  update_vertex_buffer(app);
  update_index_buffer(app);

  VkSubmitInfo submitInfo = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = waitSemaphores,
    .pWaitDstStageMask = waitStages,
    .commandBufferCount = 1,
    .pCommandBuffers = &app->command_buffers[imageIndex],
    .signalSemaphoreCount = 1,
    .pSignalSemaphores = signalSemaphores};

  vkResetFences(app->logical_device, 1, &app->in_flight_fences[app->current_frame]);

  result = vkQueueSubmit(app->graphics_queue, 1, &submitInfo, app->in_flight_fences[app->current_frame]);
  dbg_assert(result == VK_SUCCESS);

  VkSwapchainKHR swapChains[] = {app->swap_chain};

  VkPresentInfoKHR presentInfo = {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = signalSemaphores,
    .swapchainCount = 1,
    .pSwapchains = swapChains,
    .pImageIndices = &imageIndex};

  result = vkQueuePresentKHR(app->present_queue, &presentInfo);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || app->frame_buffer_resized) {
    app->frame_buffer_resized = false;
    recreate_swap_chain(app);
  } else if (result != VK_SUCCESS) {
    exit(-1);
  }

  app->current_frame = (app->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void update_uniform_buffers(internal_strs_app *app, uint32_t current_image) {
  time_passed += 0.01f;

  UniformBufferObject ubo = {0};
  ubo.model[0][0] = 1.0f;
  ubo.model[1][1] = 1.0f;
  ubo.model[2][2] = 1.0f;
  ubo.model[3][3] = 1.0f;
  glm_rotate(ubo.model, (float) (time_passed * degrees_to_radians(90.0f)), (vec3) {0.0f, 0.0f, 1.0f});
  glm_lookat((vec3) {2.0f, 2.0f, 2.0f}, (vec3) {0.0f, 0.0f, 0.0f}, (vec3) {0.0f, 0.0f, 1.0f}, ubo.view);
  glm_perspective((float) degrees_to_radians(45.0f),
                  (float) app->swap_chain_extent.width / (float) app->swap_chain_extent.height,
                  0.1f, 10.0f, ubo.proj);
  ubo.proj[1][1] *= -1;

  void *data;
  vkMapMemory(app->logical_device, app->uniform_buffers_memory[current_image], 0, sizeof(ubo), 0, &data);
  memcpy(data, &ubo, sizeof(ubo));
  vkUnmapMemory(app->logical_device, app->uniform_buffers_memory[current_image]);
}

STRS_INTERN void create_descriptor_pool(internal_strs_app *app) {
  VkDescriptorPoolSize poolSize = {
    .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    .descriptorCount = app->number_of_images};

  VkDescriptorPoolCreateInfo poolInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .poolSizeCount = 1,
    .pPoolSizes = &poolSize,
    .maxSets = app->number_of_images};

  VkResult result = vkCreateDescriptorPool(app->logical_device, &poolInfo, NULL, &app->descriptor_pool);
  dbg_assert(result == VK_SUCCESS);
}

STRS_INTERN void create_descriptor_sets(internal_strs_app *app) {
  VkDescriptorSetLayout *layouts = malloc(sizeof(VkDescriptorSetLayout) * app->number_of_images);
  for (int i = 0; i < app->number_of_images; i++) {
    layouts[i] = app->descriptor_set_layout;
  }
  VkDescriptorSetAllocateInfo allocInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .descriptorPool = app->descriptor_pool,
    .descriptorSetCount = app->number_of_images,
    .pSetLayouts = layouts};

  app->descriptor_sets = malloc(sizeof(VkDescriptorSet) * app->number_of_images);
  VkResult result = vkAllocateDescriptorSets(app->logical_device, &allocInfo, app->descriptor_sets);
  dbg_assert(result == VK_SUCCESS);

  for (size_t i = 0; i < app->number_of_images; i++) {
    VkDescriptorBufferInfo bufferInfo = {
      .buffer = app->uniform_buffers[i],
      .offset = 0,
      .range = sizeof(UniformBufferObject)};

    VkWriteDescriptorSet descriptorWrite = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = app->descriptor_sets[i],
      .dstBinding = 0,
      .dstArrayElement = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = 1,
      .pBufferInfo = &bufferInfo};

    vkUpdateDescriptorSets(app->logical_device,
                           1,
                           &descriptorWrite,
                           0, NULL);
  }
}

void endSingleTimeCommands(internal_strs_app *app, VkCommandBuffer commandBuffer) {
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .commandBufferCount = 1,
    .pCommandBuffers = &commandBuffer};

  vkQueueSubmit(app->graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(app->graphics_queue);

  vkFreeCommandBuffers(app->logical_device, app->command_pool, 1, &commandBuffer);
}

VkCommandBuffer beginSingleTimeCommands(internal_strs_app *app) {
  VkCommandBufferAllocateInfo allocInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandPool = app->command_pool,
    .commandBufferCount = 1};

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(app->logical_device, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  return commandBuffer;
}

void copyBufferToImage(strs_app *app, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands(app);

  VkBufferImageCopy region = {
    .bufferOffset = 0,
    .bufferRowLength = 0,
    .bufferImageHeight = 0,
    .imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    .imageSubresource.mipLevel = 0,
    .imageSubresource.baseArrayLayer = 0,
    .imageSubresource.layerCount = 1,
    .imageOffset = (VkOffset3D) {0, 0, 0},
    .imageExtent = {width, height, 1}};

  vkCmdCopyBufferToImage(commandBuffer,
                         buffer, image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  endSingleTimeCommands(app, commandBuffer);
}

void transitionImageLayout(strs_app *app, VkImage image,
                           VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands(app);

  VkImageMemoryBarrier barrier = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    .oldLayout = oldLayout,
    .newLayout = newLayout,
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .image = image,
    .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    .subresourceRange.baseMipLevel = 0,
    .subresourceRange.levelCount = 1,
    .subresourceRange.baseArrayLayer = 0,
    .subresourceRange.layerCount = 1};

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    exit(-1);
  }

  vkCmdPipelineBarrier(
    commandBuffer,
    sourceStage, destinationStage,
    0,
    0, NULL,
    0, NULL,
    1, &barrier);

  endSingleTimeCommands(app, commandBuffer);
}

void createImage(internal_strs_app *app, uint32_t width, uint32_t height,
                 VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                 VkMemoryPropertyFlags properties, VkImage *image,
                 VkDeviceMemory *imageMemory) {
  VkImageCreateInfo imageInfo = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .imageType = VK_IMAGE_TYPE_2D,
    .extent.width = width,
    .extent.height = height,
    .extent.depth = 1,
    .mipLevels = 1,
    .arrayLayers = 1,
    .format = format,
    .tiling = tiling,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .usage = usage,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

  VkResult result = vkCreateImage(app->logical_device, &imageInfo, NULL, image);

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(app->logical_device, *image, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize = memRequirements.size,
    .memoryTypeIndex = find_memory_type(app->physical_device, memRequirements.memoryTypeBits, properties)};

  result = vkAllocateMemory(app->logical_device, &allocInfo, NULL, imageMemory);

  vkBindImageMemory(app->logical_device, *image, *imageMemory, 0);
}

void create_texture_image(internal_strs_app *app) {
  int texWidth, texHeight, texChannels;
  stbi_uc *pixels = stbi_load("texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  dbg_assert(pixels != NULL);

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  create_buffer(app, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &stagingBuffer, &stagingBufferMemory);

  void *data;
  vkMapMemory(app->logical_device, stagingBufferMemory, 0, imageSize, 0, &data);
  memcpy(data, pixels, imageSize);
  vkUnmapMemory(app->logical_device, stagingBufferMemory);

  stbi_image_free(pixels);

  createImage(app, texWidth, texHeight,
              VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &app->texture_image, &app->texture_image_memory);

  transitionImageLayout(app, app->texture_image,
                        VK_FORMAT_R8G8B8A8_SRGB,
                        VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  copyBufferToImage(app, stagingBuffer, app->texture_image, texWidth, texHeight);
  transitionImageLayout(app, app->texture_image,
                        VK_FORMAT_R8G8B8A8_SRGB,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  vkDestroyBuffer(app->logical_device, stagingBuffer, NULL);
  vkFreeMemory(app->logical_device, stagingBufferMemory, NULL);
}

void update_index_buffer(internal_strs_app *app) {
  if (app->index_buffer.bufferSize <= app->index_count) {
    destroy_buffer(app, &app->index_buffer);
    create_index_buffer(app);
  }
  if (app->index_buffer.contentsChanged) {
    memcpy(app->index_buffer.data, app->indices, app->index_buffer.bufferSize);
  }
  copy_buffer(app, app->index_buffer.stagingBuffer, app->index_buffer.buffer, app->index_buffer.bufferSize);

  vkDeviceWaitIdle(app->logical_device);

  vkFreeCommandBuffers(app->logical_device, app->command_pool, app->number_of_images, app->command_buffers);
  create_command_buffers(app);
}

void destroy_buffer(internal_strs_app *app, vulkan_buffer* buffer) {
  vkDeviceWaitIdle(app->logical_device);

  vkDestroyBuffer(app->logical_device, buffer->stagingBuffer, NULL);
  vkFreeMemory(app->logical_device, buffer->stagingBufferMemory, NULL);

  vkDestroyBuffer(app->logical_device, buffer->buffer, NULL);
  vkFreeMemory(app->logical_device, buffer->bufferMemory, NULL);
}

void strs_push_indices(strs_app *app, const uint16_t *indices, uint64_t count) {
  uint32_t a = 0;
  for (uint32_t i = app->index_count; i < app->index_count + count; i++) {
    app->indices[i] = indices[a];
    a++;
  }
  app->index_count += count;
  app->index_buffer.contentsChanged = true;
}

void strs_push_vertices(strs_app *app, const strs_vertex *vertices, uint64_t count) {
  internal_strs_app *intern_app = (internal_strs_app*)intern_app;
  uint32_t a = 0;
  for (uint32_t i = intern_app->vertex_count; i < intern_app->vertex_count + count; i++) {
    intern_app->vertices[i] = vertices[a];
    a++;
  }
  intern_app->vertex_count += count;
  intern_app->vertex_buffer.contentsChanged = true;
}

void update_vertex_buffer(internal_strs_app *app) {
  if (app->vertex_buffer.bufferSize <= app->vertex_count) {
    destroy_buffer(app, &app->vertex_buffer);
    create_index_buffer(app);
  }
  if (app->vertex_buffer.contentsChanged) {
    memcpy(app->vertex_buffer.data, app->indices, app->vertex_buffer.bufferSize);
  }
  copy_buffer(app, app->vertex_buffer.stagingBuffer, app->vertex_buffer.buffer, app->vertex_buffer.bufferSize);

  vkDeviceWaitIdle(app->logical_device);

  vkFreeCommandBuffers(app->logical_device, app->command_pool, app->number_of_images, app->command_buffers);
  create_command_buffers(app);
}

static void frameBufferResizeCallback(strs_window window, int width, int height) {
  internal_strs_app *app = strs_window_get_user_pointer(window);
  app->frame_buffer_resized = true;
}

STRS_LIB strs_app *strs_app_create(int width, int height, strs_string *title) {
  internal_strs_app *app = malloc(sizeof(internal_strs_app*));

  app->window = strs_window_create(width, height, title);
  strs_window_set_user_pointer(app->window, app);
  strs_window_

  create_instance(app);
  create_surface(app);
  pick_physical_device(app);
  create_logical_device(app);
  create_swap_chain(app);
  create_image_views(app);
  create_render_pass(app);
  create_shader_modules(app);
  create_descriptor_set_layout(app);
  fill_config_info(app);
  create_graphics_pipeline(app);
  create_frame_buffers(app);
  create_command_pool(app);
  create_vertex_buffer(app);
  create_index_buffer(app);
  create_uniform_buffers(app);
  create_descriptor_pool(app);
  create_descriptor_sets(app);
  create_command_buffers(app);
  create_sync_objects(app);

  return (strs_app*)app;
}

void *main_loop(void *arg) {
  internal_strs_app *app = (internal_strs_app*)arg;
  while (!strs_window_closing(app->window)) {
    strs_window_poll_events(app->window);
    draw_frame(app);
  }
  vkDeviceWaitIdle(app->logical_device);

  return NULL;
}

STRS_LIB void strs_app_run(strs_app *app) {
  internal_strs_app *intern_app = (internal_strs_app*)app;
  pthread_create(&intern_app->thread, NULL, main_loop, intern_app);
}

STRS_LIB void strs_app_add(strs_app *app, strs_widget *widget) {
  widget->create_widget(app, widget->pointer);
}

STRS_LIB void strs_app_free(strs_app *application) {
  internal_strs_app *app = (internal_strs_app*)application;
  pthread_join(app->thread, NULL);

  cleanup_swap_chain(app);

  //  vkDestroyImage(app->logical_device, app->texture_image, NULL);
  //  vkFreeMemory(app->logical_device, app->texture_image_memory, NULL);

  vkUnmapMemory(app->logical_device, app->vertex_buffer.stagingBufferMemory);
  vkUnmapMemory(app->logical_device, app->index_buffer.stagingBufferMemory);

  vkDestroyDescriptorSetLayout(app->logical_device, app->descriptor_set_layout, NULL);

  for (size_t i = 0; i < app->number_of_images; i++) {
    vkDestroyBuffer(app->logical_device, app->uniform_buffers[i], NULL);
    vkFreeMemory(app->logical_device, app->uniform_buffers_memory[i], NULL);
  }

  vkDestroyBuffer(app->logical_device, app->vertex_buffer.stagingBuffer, NULL);
  vkFreeMemory(app->logical_device, app->vertex_buffer.stagingBufferMemory, NULL);

  vkDestroyBuffer(app->logical_device, app->index_buffer.stagingBuffer, NULL);
  vkFreeMemory(app->logical_device, app->index_buffer.stagingBufferMemory, NULL);

  vkDestroyBuffer(app->logical_device, app->index_buffer.buffer, NULL);
  vkFreeMemory(app->logical_device, app->index_buffer.bufferMemory, NULL);

  vkDestroyBuffer(app->logical_device, app->vertex_buffer.buffer, NULL);
  vkFreeMemory(app->logical_device, app->vertex_buffer.bufferMemory, NULL);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(app->logical_device, app->render_finished_semaphores[i], NULL);
    vkDestroySemaphore(app->logical_device, app->image_available_semaphores[i], NULL);
    vkDestroyFence(app->logical_device, app->in_flight_fences[i], NULL);
  }
  vkDestroyCommandPool(app->logical_device, app->command_pool, NULL);
  vkDestroyShaderModule(app->logical_device, app->vert_shader_module, NULL);
  vkDestroyShaderModule(app->logical_device, app->frag_shader_module, NULL);
  vkDestroyDevice(app->logical_device, NULL);
  vkDestroySurfaceKHR(app->instance, app->surface, NULL);
  vkDestroyInstance(app->instance, NULL);
  strs_window_free(app->window);

  free(app->image_available_semaphores);
  free(app->render_finished_semaphores);
  free(app->in_flight_fences);
  free(app->images_in_flight);

  free(app->uniform_buffers);
  free(app->uniform_buffers_memory);

  free(app->descriptor_sets);

  free(app);
  app = NULL;
}
