#ifndef STEROS_APP_H
#define STEROS_APP_H

#include "steros.h"

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stdbool.h>

#define STRS_IMPL_STR

#include "ntdstring.h"

typedef struct {
  void *data;
  VkDeviceSize bufferSize;
  VkBuffer buffer;
  VkDeviceMemory bufferMemory;
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  bool contentsChanged;
} VulkanBuffer;

typedef struct {
  vec2 pos;
  vec3 color;
} Vertex;

typedef struct {
  VkVertexInputAttributeDescription array[2];
  uint32_t size;
} VkVertexInputAttributeDescriptionArray;

typedef struct {
  VkPipelineShaderStageCreateInfo shaderStages[2];
  VkVertexInputBindingDescription bindingDescription;
  VkVertexInputAttributeDescriptionArray attributeDescriptions;
  VkPipelineVertexInputStateCreateInfo vertexInputInfo;
  VkPipelineInputAssemblyStateCreateInfo inputAssembly;
  VkPipelineViewportStateCreateInfo viewportState;
  VkPipelineRasterizationStateCreateInfo rasterizer;
  VkPipelineMultisampleStateCreateInfo multisampling;
  VkPipelineColorBlendAttachmentState colorBlendAttachment;
  VkPipelineColorBlendStateCreateInfo colorBlending;
  VkDynamicState dynamicStateEnables[2];
  uint8_t dynamicStateCount;
  VkPipelineDynamicStateCreateInfo dynamicStateInfo;
} PipelineConfigInfo;


typedef struct {
  Vertex *vertices;
  uint64_t vertexCount;
  uint16_t *indices;
  uint64_t indexCount;

  GLFWwindow *window;

  VkInstance instance;
  VkSurfaceKHR surface;
  VkPhysicalDevice physicalDevice;

  VkDevice logicalDevice;
  VkQueue presentQueue;
  VkQueue graphicsQueue;
  VkSwapchainKHR swapChain;
  VkImage *swapChainImages;

  uint32_t numberOfImages;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;
  VkImageView *swapChainImageViews;
  VkRenderPass renderPass;

  VkShaderModule vertShaderModule;
  VkShaderModule fragShaderModule;

  VkDescriptorSetLayout descriptorSetLayout;

  PipelineConfigInfo pipelineConfig;
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;
  VkFramebuffer *swapChainFrameBuffers;
  VkCommandPool commandPool;

  VkDescriptorPool descriptorPool;
  VkDescriptorSet *descriptorSets;

  VkCommandBuffer *commandBuffers;

  VkSemaphore *imageAvailableSemaphores;
  VkSemaphore *renderFinishedSemaphores;
  VkFence *inFlightFences;
  VkFence *imagesInFlight;

  VulkanBuffer vertexBuffer;
  VulkanBuffer indexBuffer;

  VkBuffer *uniformBuffers;
  VkDeviceMemory *uniformBuffersMemory;

  VkImage textureImage;
  VkDeviceMemory textureImageMemory;

  size_t currentFrame;
  bool frameBufferResized;
  pthread_t thread;
} StrsApp;

typedef void (*PFN_strsCreateWidget)(StrsApp *app, void *pointer);
typedef void (*PFN_strsUpdateWidget)(StrsApp *app, void *pointer);
typedef void (*PFN_strsWhileSelected)(StrsApp *app, void *pointer);
typedef void (*PFN_strsOnAction)(StrsApp *app, void *pointer);

typedef struct {
  void *pointer;
  PFN_strsCreateWidget createWidget;
  PFN_strsUpdateWidget updateWidget;
  PFN_strsWhileSelected whileSelected;
  PFN_strsOnAction onAction;
} Widget;

STRS_LIB int strsInit();
STRS_LIB StrsApp *strsAppCreate(int width, int height, StrsStr *title);
#ifndef STRS_NOT_MULTI_THREADED
STRS_LIB void strsAppRun(StrsApp *app);
#else
typedef void (*PFN_strsExecAsync)();
STRS_LIB void strsAppRun(StrsApp *app, PFN_strsExecAsync strsExecAsync);
#endif
STRS_LIB void strsAppAdd(StrsApp *app, Widget *widget);
STRS_LIB void strsAppFree(StrsApp *app);
STRS_LIB void strsTerminate();

#endif //STEROS_APP_H
