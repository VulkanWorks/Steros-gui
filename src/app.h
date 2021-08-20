#ifndef STEROS_APP_H
#define STEROS_APP_H

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
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    VkFramebuffer *swapChainFrameBuffers;
    VkCommandPool commandPool;
    VkCommandBuffer *commandBuffers;

    VkSemaphore* imageAvailableSemaphores;
    VkSemaphore* renderFinishedSemaphores;
    VkFence* inFlightFences;
    VkFence* imagesInFlight;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    size_t currentFrame;
    bool frameBufferResized;
} StrApp;

int strInit();
StrApp* strAppCreate(int width, int height, const char* title);
void strAppRun(StrApp *app);
void strAppFree(StrApp* app);
void strTerminate();

#endif //STEROS_APP_H
