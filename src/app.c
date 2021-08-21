#include "app.h"
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <la.h>

#ifndef NDEBUG
#define dbg_assert(x) assert(x)
#else
#define dbg_assert
#endif

typedef struct {
  VkVertexInputAttributeDescription array[2];
  uint32_t size;
} VkVertexInputAttributeDescriptionArray;

typedef struct {
  V2f pos;
  V3f color;
} Vertex;

typedef struct {
  VkSurfaceCapabilitiesKHR capabilities;
  VkSurfaceFormatKHR *formats;
  uint32_t formatsSize;
  VkPresentModeKHR *presentModes;
  uint32_t presentModesSize;
} SwapChainSupportDetails;

typedef struct {
  bool hasValue;
  uint32_t value;
} OptionUInt;

typedef struct {
  OptionUInt graphicsFamily;
  OptionUInt presentFamily;
} QueueFamilyIndices;

static bool init = false;
static bool enableValidationLayers = true;
static const char *validationLayers[] = {"VK_LAYER_KHRONOS_validation"};
static const char *deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
static Vertex vertices[] = {
  {{0.0f,  -0.5f}, {0.5f, 0.5f, 1.0f}},
  {{0.5f,  0.5f},  {0.0f, 1.0f, 0.5f}},
  {{-0.5f, 0.5f},  {3.0f, 7.0f, 1.0f}}
};
const int MAX_FRAMES_IN_FLIGHT = 2;

void drawFrame(StrApp *app);
void recreateSwapChain(StrApp *app);
void cleanupSwapChain(StrApp *app);
void createInstance(StrApp *app);
void createSurface(StrApp *app);
void pickPhysicalDevice(StrApp *app);
void createLogicalDevice(StrApp *app);
void createSwapChain(StrApp *app);
void createImageViews(StrApp *app);
void createRenderPass(StrApp *app);
void createShaderModules(StrApp *app);
void createGraphicsPipeline(StrApp *app);
void createFrameBuffers(StrApp *app);
void createCommandPool(StrApp *app);
void createVertexBuffer(StrApp *app);
void createCommandBuffers(StrApp *app);
void createSyncObjects(StrApp *app);

static VkVertexInputAttributeDescriptionArray getAttributeDescriptions() {
  VkVertexInputAttributeDescriptionArray attributeDescriptions = {
    .array = {
      {
        .binding = 0,
        .location = 0,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offsetof(Vertex, pos)
      },
      {
        .binding = 0,
        .location = 1,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Vertex, color)
      }
    },
    .size = 2
  };

  return attributeDescriptions;
}

static VkVertexInputBindingDescription getBindingDescription() {
  VkVertexInputBindingDescription bindingDescription = {
    .binding = 0,
    .stride = sizeof(Vertex),
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
  };

  return bindingDescription;
}

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  exit(-1);
}

uint32_t clamp(uint32_t d, uint32_t min, uint32_t max) {
  const uint32_t t = d < min ? min : d;
  return t > max ? max : t;
}

char *readShader(const char *filename, long *size) {
  FILE *fp;

  fp = fopen(filename, "rb");
  if (fp == NULL) {
    fprintf(stderr, "cannot open input file\n");
    return NULL;
  }

  struct stat sb;
  if (stat(filename, &sb) == -1) {
    perror("error");
    exit(-1);
  }

  char *fileContents = malloc(sb.st_size);
  fread(fileContents, sb.st_size, 1, fp);

  fclose(fp);
  *size = sb.st_size;
  return fileContents;
}

void swapChainSdFree(SwapChainSupportDetails *ptr) {
  free(ptr->presentModes);
  free(ptr->formats);
  ptr = NULL;
}

void option_u_int_set_value(OptionUInt *ptr, uint32_t value) {
  ptr->value = value;
  ptr->hasValue = true;
}

void option_u_int_create(OptionUInt *ptr) {
  ptr->hasValue = false;
  ptr->value = 0;
}

bool queueFamilyIndicesIsComplete(QueueFamilyIndices *indices) {
  return indices->graphicsFamily.hasValue && indices->presentFamily.hasValue;
}

QueueFamilyIndices findQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
  QueueFamilyIndices indices;
  option_u_int_create(&indices.graphicsFamily);

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);

  VkQueueFamilyProperties *queueFamilies = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies);

  for (int i = 0; i < queueFamilyCount; i++) {
    if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      option_u_int_set_value(&indices.graphicsFamily, i);
    }

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

    if (presentSupport) {
      option_u_int_set_value(&indices.presentFamily, i);
    }

    if (queueFamilyIndicesIsComplete(&indices)) {
      break;
    }
  }
  free(queueFamilies);
  return indices;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {// TODO: Finish this function
  /*uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

  VkExtensionProperties *availableExtensions = malloc(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);

  for(int i = 0; i < extensionCount; i++) {
      if (deviceExtensions[0] == availableExtensions[i].extensionName) {
          free(availableExtensions);
          return true;
      }
  }

  free(availableExtensions);
  return false;*/

  return true;
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
  SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL);

  if (formatCount != 0) {
    details.formatsSize = formatCount;
    details.formats = malloc(sizeof(uint32_t) * formatCount);
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

bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
  QueueFamilyIndices indices = findQueueFamilyIndices(physicalDevice, surface);

  bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);

  bool swapChainAdequate = false;
  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);
    swapChainAdequate = swapChainSupport.formats != NULL && swapChainSupport.presentModes != NULL;
    swapChainSdFree(&swapChainSupport);
  }

  return queueFamilyIndicesIsComplete(&indices) && swapChainAdequate;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkSurfaceFormatKHR *formats, uint32_t sizeOfArray) {
  for (int i = 0; i < sizeOfArray; i++) {
    if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return formats[i];
    }
  }

  return formats[0];
}

VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR *presentModes, uint32_t sizeOfArray) {
  for (int i = 0; i < sizeOfArray; i++) {
    if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
      return presentModes[i];
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR capabilities, GLFWwindow *window) {
  if (capabilities.currentExtent.width != UINT32_MAX) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actualExtent = {
      width,
      height
    };

    actualExtent.width =
      clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height =
      clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
  }
}

void createBuffer(StrApp *app, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                  VkBuffer* buffer, VkDeviceMemory* bufferMemory) {
  VkBufferCreateInfo bufferInfo = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = size,
    .usage = usage,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE
  };

  VkResult result = vkCreateBuffer(app->logicalDevice, &bufferInfo, NULL, buffer);
  dbg_assert(result == VK_SUCCESS);

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(app->logicalDevice, *buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize = memRequirements.size,
    .memoryTypeIndex = findMemoryType(app->physicalDevice, memRequirements.memoryTypeBits, properties)
  };

  result = vkAllocateMemory(app->logicalDevice, &allocInfo, NULL, bufferMemory);
  dbg_assert(result == VK_SUCCESS);

  vkBindBufferMemory(app->logicalDevice, *buffer, *bufferMemory, 0);
}

void copyBuffer(StrApp *app, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
  VkCommandBufferAllocateInfo allocInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandPool = app->commandPool,
    .commandBufferCount = 1
  };

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(app->logicalDevice, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
  };

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  VkBufferCopy copyRegion = {
    .size = size
  };

  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .commandBufferCount = 1,
    .pCommandBuffers = &commandBuffer
  };

  vkQueueSubmit(app->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(app->graphicsQueue);

  vkFreeCommandBuffers(app->logicalDevice, app->commandPool, 1, &commandBuffer);
}

void createSyncObjects(StrApp *app) {
  app->imageAvailableSemaphores = malloc(MAX_FRAMES_IN_FLIGHT);
  app->renderFinishedSemaphores = malloc(MAX_FRAMES_IN_FLIGHT);
  app->inFlightFences = malloc(MAX_FRAMES_IN_FLIGHT);
  app->imagesInFlight = malloc(app->numberOfImages);

  VkSemaphoreCreateInfo semaphoreInfo = {
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
  };

  VkFenceCreateInfo fenceInfo = {
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .flags = VK_FENCE_CREATE_SIGNALED_BIT
  };

  VkResult result;
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    result = vkCreateSemaphore(app->logicalDevice, &semaphoreInfo, NULL, &app->imageAvailableSemaphores[i]);
    dbg_assert(result == VK_SUCCESS);
    result = vkCreateSemaphore(app->logicalDevice, &semaphoreInfo, NULL, &app->renderFinishedSemaphores[i]);
    dbg_assert(result == VK_SUCCESS);
    result = vkCreateFence(app->logicalDevice, &fenceInfo, NULL, &app->inFlightFences[i]);
    dbg_assert(result == VK_SUCCESS);
  }
  for (int i = 0; i < app->numberOfImages; i++) {
    app->imagesInFlight[i] = 0;
  }
}

void createCommandBuffers(StrApp *app) {
  app->commandBuffers = malloc(app->numberOfImages);

  VkCommandBufferAllocateInfo allocInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = app->commandPool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = app->numberOfImages
  };

  VkResult result = vkAllocateCommandBuffers(app->logicalDevice, &allocInfo, app->commandBuffers);
  dbg_assert(result == VK_SUCCESS);

  for (size_t i = 0; i < app->numberOfImages; i++) {
    VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };

    result = vkBeginCommandBuffer(app->commandBuffers[i], &beginInfo);
    dbg_assert(result == VK_SUCCESS);

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

    VkRenderPassBeginInfo renderPassInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = app->renderPass,
      .framebuffer = app->swapChainFrameBuffers[i],
      .renderArea.offset = {0, 0},
      .renderArea.extent = app->swapChainExtent,
      .clearValueCount = 1,
      .pClearValues = &clearColor
    };

    vkCmdBeginRenderPass(app->commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(app->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, app->pipeline);

    VkBuffer vertexBuffers[] = {app->vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(app->commandBuffers[i], 0, 1, vertexBuffers, offsets);

    vkCmdDraw(app->commandBuffers[i], 3, 1, 0, 0);

    vkCmdEndRenderPass(app->commandBuffers[i]);

    result = vkEndCommandBuffer(app->commandBuffers[i]);
    dbg_assert(result == VK_SUCCESS);
  }
}

void createVertexBuffer(StrApp *app) {
  VkDeviceSize bufferSize = sizeof(vertices);

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  createBuffer(app, bufferSize,
               VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

  void *data;
  vkMapMemory(app->logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, vertices, (size_t) bufferSize);
  vkUnmapMemory(app->logicalDevice, stagingBufferMemory);

  createBuffer(app, bufferSize,
               VK_BUFFER_USAGE_TRANSFER_DST_BIT |
               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &app->vertexBuffer, &app->vertexBufferMemory);

  copyBuffer(app, stagingBuffer, app->vertexBuffer, bufferSize);

  vkDestroyBuffer(app->logicalDevice, stagingBuffer, NULL);
  vkFreeMemory(app->logicalDevice, stagingBufferMemory, NULL);
}

void createCommandPool(StrApp *app) {
  QueueFamilyIndices queueFamilyIndices = findQueueFamilyIndices(app->physicalDevice, app->surface);

  VkCommandPoolCreateInfo poolInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value
  };

  VkResult result = vkCreateCommandPool(app->logicalDevice, &poolInfo, NULL, &app->commandPool);
  dbg_assert(result == VK_SUCCESS);
}

void createFrameBuffers(StrApp *app) {
  app->swapChainFrameBuffers = malloc(app->numberOfImages);

  for (size_t i = 0; i < app->numberOfImages; i++) {
    VkImageView attachments[] = {
      app->swapChainImageViews[i]
    };

    VkFramebufferCreateInfo framebufferInfo = {
      .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass = app->renderPass,
      .attachmentCount = 1,
      .pAttachments = attachments,
      .width = app->swapChainExtent.width,
      .height = app->swapChainExtent.height,
      .layers = 1
    };

    VkResult result =
      vkCreateFramebuffer(app->logicalDevice, &framebufferInfo, NULL, &app->swapChainFrameBuffers[i]);
    dbg_assert(result == VK_SUCCESS);
  }
}

void createGraphicsPipeline(StrApp *app) {
  VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_VERTEX_BIT,
    .module = app->vertShaderModule,
    .pName = "main"
  };

  VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
    .module = app->fragShaderModule,
    .pName = "main"
  };

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

  VkVertexInputBindingDescription bindingDescription = getBindingDescription();
  VkVertexInputAttributeDescriptionArray attributeDescriptions = getAttributeDescriptions();

  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 1,
    .pVertexBindingDescriptions = &bindingDescription,
    .vertexAttributeDescriptionCount = attributeDescriptions.size,
    .pVertexAttributeDescriptions = attributeDescriptions.array
  };

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE
  };

  VkViewport viewport = {
    .x = 0.0f,
    .y = 0.0f,
    .width = (float) app->swapChainExtent.width,
    .height = (float) app->swapChainExtent.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f
  };

  VkRect2D scissor = {
    .offset = {0, 0},
    .extent = app->swapChainExtent
  };

  VkPipelineViewportStateCreateInfo viewportState = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .pViewports = &viewport,
    .scissorCount = 1,
    .pScissors = &scissor
  };

  VkPipelineRasterizationStateCreateInfo rasterizer = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .depthClampEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .lineWidth = 1.0f,
    .cullMode = VK_CULL_MODE_BACK_BIT,
    .frontFace = VK_FRONT_FACE_CLOCKWISE,
    .depthBiasEnable = VK_FALSE
  };

  VkPipelineMultisampleStateCreateInfo multisampling = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .sampleShadingEnable = VK_FALSE,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
  };

  VkPipelineColorBlendAttachmentState colorBlendAttachment = {
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                      VK_COLOR_COMPONENT_G_BIT |
                      VK_COLOR_COMPONENT_B_BIT |
                      VK_COLOR_COMPONENT_A_BIT,
    .blendEnable = VK_FALSE
  };

  VkPipelineColorBlendStateCreateInfo colorBlending = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .logicOpEnable = VK_FALSE,
    .logicOp = VK_LOGIC_OP_COPY,
    .attachmentCount = 1,
    .pAttachments = &colorBlendAttachment,
    .blendConstants[0] = 0.0f,
    .blendConstants[1] = 0.0f,
    .blendConstants[2] = 0.0f,
    .blendConstants[3] = 0.0f
  };

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = 0,
    .pushConstantRangeCount = 0
  };

  VkResult result =
    vkCreatePipelineLayout(app->logicalDevice, &pipelineLayoutInfo, NULL, &app->pipelineLayout);
  dbg_assert(result == VK_SUCCESS);

  VkGraphicsPipelineCreateInfo pipelineInfo = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .stageCount = 2,
    .pStages = shaderStages,
    .pVertexInputState = &vertexInputInfo,
    .pInputAssemblyState = &inputAssembly,
    .pViewportState = &viewportState,
    .pRasterizationState = &rasterizer,
    .pMultisampleState = &multisampling,
    .pColorBlendState = &colorBlending,
    .layout = app->pipelineLayout,
    .renderPass = app->renderPass,
    .subpass = 0,
    .basePipelineHandle = NULL
  };

  result =
    vkCreateGraphicsPipelines(
      app->logicalDevice,
      NULL,
      1,
      &pipelineInfo,
      NULL,
      &app->pipeline);
  dbg_assert(result == VK_SUCCESS);
}

void createShaderModules(StrApp *app) {
  long vertSize = 0;
  long fragSize = 0;
  char *vertShaderCode = readShader("shaders/shader.vert.spv", &vertSize);
  char *fragShaderCode = readShader("shaders/shader.frag.spv", &fragSize);

  VkShaderModuleCreateInfo vertModuleCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .codeSize = vertSize,
    .pCode = (const uint32_t *) vertShaderCode
  };
  VkShaderModuleCreateInfo fragModuleCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .codeSize = fragSize,
    .pCode = (const uint32_t *) fragShaderCode
  };

  VkResult result;
  result = vkCreateShaderModule(app->logicalDevice, &vertModuleCreateInfo, NULL, &app->vertShaderModule);
  dbg_assert(result == VK_SUCCESS);
  result = vkCreateShaderModule(app->logicalDevice, &fragModuleCreateInfo, NULL, &app->fragShaderModule);
  dbg_assert(result == VK_SUCCESS);

  free(vertShaderCode);
  free(fragShaderCode);
}

void createRenderPass(StrApp *app) {
  VkAttachmentDescription colorAttachment = {
    .format = app->swapChainImageFormat,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
  };

  VkAttachmentReference colorAttachmentRef = {
    .attachment = 0,
    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
  };

  VkSubpassDescription subpass = {
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .colorAttachmentCount = 1,
    .pColorAttachments = &colorAttachmentRef
  };

  VkSubpassDependency dependency = {
    .srcSubpass = VK_SUBPASS_EXTERNAL,
    .dstSubpass = 0,
    .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .srcAccessMask = 0,
    .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
  };

  VkRenderPassCreateInfo renderPassInfo = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .attachmentCount = 1,
    .pAttachments = &colorAttachment,
    .subpassCount = 1,
    .pSubpasses = &subpass,
    .dependencyCount = 1,
    .pDependencies = &dependency
  };

  VkResult result =
    vkCreateRenderPass(app->logicalDevice, &renderPassInfo, NULL, &app->renderPass);
  dbg_assert(result == VK_SUCCESS);
}

void createImageViews(StrApp *app) {
  app->swapChainImageViews = malloc(app->numberOfImages);

  for (size_t i = 0; i < app->numberOfImages; i++) {
    VkImageViewCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = app->swapChainImages[i],
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = app->swapChainImageFormat,
      .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
      .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
      .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
      .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
      .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .subresourceRange.baseMipLevel = 0,
      .subresourceRange.levelCount = 1,
      .subresourceRange.baseArrayLayer = 0,
      .subresourceRange.layerCount = 1
    };

    VkResult result =
      vkCreateImageView(app->logicalDevice, &createInfo, NULL, &app->swapChainImageViews[i]);
    dbg_assert(result == VK_SUCCESS);
  }
}

void createSwapChain(StrApp *app) {
  SwapChainSupportDetails swapChainSupport = querySwapChainSupport(app->physicalDevice, app->surface);

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
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
  };

  QueueFamilyIndices indices = findQueueFamilyIndices(app->physicalDevice, app->surface);
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value, indices.presentFamily.value};

  if (indices.graphicsFamily.value != indices.presentFamily.value) {
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

  createInfo.oldSwapchain = VK_NULL_HANDLE;

  VkResult result = vkCreateSwapchainKHR(app->logicalDevice, &createInfo, NULL, &app->swapChain);
  dbg_assert(result == VK_SUCCESS);

  vkGetSwapchainImagesKHR(app->logicalDevice, app->swapChain, &imageCount, NULL);
  app->swapChainImages = malloc(imageCount);
  vkGetSwapchainImagesKHR(app->logicalDevice, app->swapChain, &imageCount, app->swapChainImages);

  app->swapChainImageFormat = surfaceFormat.format;
  app->swapChainExtent = extent;
  app->numberOfImages = imageCount;

  swapChainSdFree(&swapChainSupport);
}

void createLogicalDevice(StrApp *app) {
  QueueFamilyIndices indices = findQueueFamilyIndices(app->physicalDevice, app->surface);

  VkDeviceQueueCreateInfo *queueCreateInfos;

  float queuePriority = 1.0f;
  int sizeOfCreateInfo;
  if (indices.graphicsFamily.value == indices.presentFamily.value) {
    VkDeviceQueueCreateInfo queueCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = indices.graphicsFamily.value,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority
    };
    sizeOfCreateInfo = 1;
    queueCreateInfos = malloc(sizeof(VkDeviceQueueCreateInfo) * sizeOfCreateInfo);
    queueCreateInfos[0] = queueCreateInfo;

  } else {
    VkDeviceQueueCreateInfo graphicsQueueCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = indices.presentFamily.value,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority
    };
    VkDeviceQueueCreateInfo presentQueueCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = indices.presentFamily.value,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority
    };
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
    .pEnabledFeatures = &deviceFeatures
  };

  createInfo.enabledExtensionCount = sizeof(deviceExtensions) / sizeof(const char *);
  createInfo.ppEnabledExtensionNames = deviceExtensions;

  if (enableValidationLayers) {
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = validationLayers;
  }

  VkResult result = vkCreateDevice(app->physicalDevice, &createInfo, NULL, &app->logicalDevice);
  dbg_assert(result == VK_SUCCESS);
  vkGetDeviceQueue(app->logicalDevice, indices.graphicsFamily.value, 0, &app->graphicsQueue);
  vkGetDeviceQueue(app->logicalDevice, indices.presentFamily.value, 0, &app->presentQueue);

  free(queueCreateInfos);
}

void pickPhysicalDevice(StrApp *app) {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(app->instance, &deviceCount, NULL);
  dbg_assert(deviceCount != 0);

  VkPhysicalDevice *physicalDevices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
  vkEnumeratePhysicalDevices(app->instance, &deviceCount, physicalDevices);

  for (int i = 0; i < deviceCount; i++) {
    if (isDeviceSuitable(physicalDevices[i], app->surface)) {
      app->physicalDevice = physicalDevices[i];
      free(physicalDevices);
      return;
    }
  }

  printf("No suitable GPU found, sorry\n");
  exit(-1);
}

void createSurface(StrApp *app) {
  glfwCreateWindowSurface(app->instance, app->window, NULL, &app->surface);
}

void createInstance(StrApp *app) {
  VkApplicationInfo appInfo = {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pNext = NULL,
    .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
    .pApplicationName = "Steros",
    .engineVersion = VK_MAKE_VERSION(1, 0, 0),
    .pEngineName = "Steros",
    .apiVersion = VK_API_VERSION_1_2
  };

  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  VkInstanceCreateInfo instanceInfo = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pNext = NULL,
    .flags = 0,
    .pApplicationInfo = &appInfo,
    .enabledExtensionCount = glfwExtensionCount,
    .ppEnabledExtensionNames = glfwExtensions,
  };

  if (enableValidationLayers) {
    instanceInfo.enabledLayerCount = 1;
    instanceInfo.ppEnabledLayerNames = validationLayers;
  }

  VkResult result = vkCreateInstance(&instanceInfo, NULL, &app->instance);
  dbg_assert(result == VK_SUCCESS);
}

void cleanupSwapChain(StrApp *app) {
  for (size_t i = 0; i < app->numberOfImages; i++) {
    vkDestroyFramebuffer(app->logicalDevice, app->swapChainFrameBuffers[i], NULL);
  }

  vkFreeCommandBuffers(app->logicalDevice, app->commandPool, app->numberOfImages, app->commandBuffers);

  vkDestroyPipeline(app->logicalDevice, app->pipeline, NULL);
  vkDestroyPipelineLayout(app->logicalDevice, app->pipelineLayout, NULL);
  vkDestroyRenderPass(app->logicalDevice, app->renderPass, NULL);

  for (size_t i = 0; i < app->numberOfImages; i++) {
    vkDestroyImageView(app->logicalDevice, app->swapChainImageViews[i], NULL);
  }

  vkDestroySwapchainKHR(app->logicalDevice, app->swapChain, NULL);

  free(app->swapChainFrameBuffers);
  free(app->swapChainImageViews);
  free(app->swapChainImages);
}

void recreateSwapChain(StrApp *app) {
  int width = 0, height = 0;
  glfwGetFramebufferSize(app->window, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(app->window, &width, &height);
    glfwWaitEvents();
  }
  vkDeviceWaitIdle(app->logicalDevice);

  cleanupSwapChain(app);

  createSwapChain(app);
  createImageViews(app);
  createRenderPass(app);
  createGraphicsPipeline(app);
  createFrameBuffers(app);
  createCommandBuffers(app);
}

void drawFrame(StrApp *app) {
  vkWaitForFences(app->logicalDevice, 1, &app->inFlightFences[app->currentFrame], VK_TRUE, UINT64_MAX);

  uint32_t imageIndex = 0;
  VkResult result = vkAcquireNextImageKHR(
    app->logicalDevice,
    app->swapChain,
    UINT64_MAX,
    app->imageAvailableSemaphores[app->currentFrame],
    VK_NULL_HANDLE,
    &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain(app);
    return;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    exit(-1);
  }

  if (app->imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
    vkWaitForFences(app->logicalDevice, 1, &app->imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
  }
  app->imagesInFlight[imageIndex] = app->inFlightFences[app->currentFrame];

  VkSemaphore waitSemaphores[] = {app->imageAvailableSemaphores[app->currentFrame]};
  VkSemaphore signalSemaphores[] = {app->renderFinishedSemaphores[app->currentFrame]};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

  VkSubmitInfo submitInfo = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = waitSemaphores,
    .pWaitDstStageMask = waitStages,
    .commandBufferCount = 1,
    .pCommandBuffers = &app->commandBuffers[imageIndex],
    .signalSemaphoreCount = 1,
    .pSignalSemaphores = signalSemaphores
  };

  vkResetFences(app->logicalDevice, 1, &app->inFlightFences[app->currentFrame]);

  result = vkQueueSubmit(app->graphicsQueue, 1, &submitInfo, app->inFlightFences[app->currentFrame]);
  dbg_assert(result == VK_SUCCESS);

  VkSwapchainKHR swapChains[] = {app->swapChain};

  VkPresentInfoKHR presentInfo = {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = signalSemaphores,
    .swapchainCount = 1,
    .pSwapchains = swapChains,
    .pImageIndices = &imageIndex
  };

  result = vkQueuePresentKHR(app->presentQueue, &presentInfo);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || app->frameBufferResized) {
    app->frameBufferResized = false;
    recreateSwapChain(app);
  } else if (result != VK_SUCCESS) {
    exit(-1);
  }

  app->currentFrame = (app->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

static void frameBufferResizeCallback(GLFWwindow *window, int width, int height) {
  StrApp *app = glfwGetWindowUserPointer(window);
  app->frameBufferResized = true;
}

StrApp *strAppCreate(int width, int height, const char *title) {
  if (!init) {
    return NULL;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);

  StrApp *app = malloc(sizeof(StrApp));
  app->window = window;
  app->currentFrame = 0;
  app->frameBufferResized = false;

  glfwSetWindowUserPointer(window, app);
  glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);

  createInstance(app);
  createSurface(app);
  pickPhysicalDevice(app);
  createLogicalDevice(app);
  createSwapChain(app);
  createImageViews(app);
  createRenderPass(app);
  createShaderModules(app);
  createGraphicsPipeline(app);
  createFrameBuffers(app);
  createCommandPool(app);
  createVertexBuffer(app);
  createCommandBuffers(app);
  createSyncObjects(app);

  return app;
}

void strAppRun(StrApp *app) {
  while (!glfwWindowShouldClose(app->window)) {
    glfwPollEvents();
    drawFrame(app);
  }
  vkDeviceWaitIdle(app->logicalDevice);
}

int strInit() {
  init = true;
  return glfwInit();
}

void strTerminate() {
  glfwTerminate();
}

void strAppFree(StrApp *app) {
  cleanupSwapChain(app);

  vkDestroyBuffer(app->logicalDevice, app->vertexBuffer, NULL);
  vkFreeMemory(app->logicalDevice, app->vertexBufferMemory, NULL);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(app->logicalDevice, app->renderFinishedSemaphores[i], NULL);
    vkDestroySemaphore(app->logicalDevice, app->imageAvailableSemaphores[i], NULL);
    vkDestroyFence(app->logicalDevice, app->inFlightFences[i], NULL);
  }
  vkDestroyCommandPool(app->logicalDevice, app->commandPool, NULL);
  vkDestroyShaderModule(app->logicalDevice, app->vertShaderModule, NULL);
  vkDestroyShaderModule(app->logicalDevice, app->fragShaderModule, NULL);
  vkDestroyDevice(app->logicalDevice, NULL);
  vkDestroySurfaceKHR(app->instance, app->surface, NULL);
  vkDestroyInstance(app->instance, NULL);
  glfwDestroyWindow(app->window);

  free(app->imageAvailableSemaphores);
  free(app->renderFinishedSemaphores);
  free(app->inFlightFences);
  free(app->imagesInFlight);

  free(app);
  app = NULL;
}


