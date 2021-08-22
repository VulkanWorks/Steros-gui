#ifndef STEROS_APP_H
#define STEROS_APP_H

#include "steros.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdbool.h>

typedef struct {
    GLFWwindow* window;

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

    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    VkFramebuffer *swapChainFrameBuffers;
    VkCommandPool commandPool;

    VkDescriptorPool descriptorPool;
    VkDescriptorSet* descriptorSets;

    VkCommandBuffer *commandBuffers;

    VkSemaphore* imageAvailableSemaphores;
    VkSemaphore* renderFinishedSemaphores;
    VkFence* inFlightFences;
    VkFence* imagesInFlight;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    VkBuffer* uniformBuffers;
    VkDeviceMemory* uniformBuffersMemory;

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;

    size_t currentFrame;
    bool frameBufferResized;
} StrApp;

STRS_LIB int strsInit();
STRS_LIB StrApp* strsAppCreate(int width, int height, const char* title);
STRS_LIB void strsAppRun(StrApp *app);
STRS_LIB void strsAppAdd(StrApp *app, Widget *widget);
STRS_LIB void strsAppFree(StrApp* app);
STRS_LIB void strsTerminate();

#endif //STEROS_APP_H
