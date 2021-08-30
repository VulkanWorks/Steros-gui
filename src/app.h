#ifndef STEROS_APP_H
#define STEROS_APP_H

// LIB
#include "steros.h"
#include "windowing/window.h"

// Vulkan
#include <vulkan/vulkan.h>

#include <cglm/cglm.h>

// STD
#include <stdbool.h>

// NTD
#define STRS_IMPL_STR
#include "ntd/string.h"

typedef struct strs_widget strs_widget;
typedef struct strs_app_tag strs_app;
typedef struct pipeline_config_info_tag pipeline_config_info;
typedef struct vulkan_buffer_tag vulkan_buffer;
typedef struct vk_vertex_input_attribute_description_array_tag vk_vertex_input_attribute_description_array;
typedef struct strs_vertex_tag strs_vertex;

struct vulkan_buffer_tag {
  void *data;
  VkDeviceSize bufferSize;
  VkBuffer buffer;
  VkDeviceMemory bufferMemory;
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  bool contentsChanged;
};

struct strs_vertex_tag {
  vec2 pos;
  vec3 color;
};

struct vk_vertex_input_attribute_description_array_tag {
  VkVertexInputAttributeDescription array[2];
  uint32_t size;
};

struct pipeline_config_info_tag {
  VkPipelineShaderStageCreateInfo shader_stages[2];
  VkVertexInputBindingDescription binding_description;
  vk_vertex_input_attribute_description_array attribute_descriptions;
  VkPipelineVertexInputStateCreateInfo vertex_input_info;
  VkPipelineInputAssemblyStateCreateInfo input_assembly;
  VkPipelineViewportStateCreateInfo viewport_state;
  VkPipelineRasterizationStateCreateInfo rasterizer;
  VkPipelineMultisampleStateCreateInfo multisampling;
  VkPipelineColorBlendAttachmentState color_blend_attachment;
  VkPipelineColorBlendStateCreateInfo color_blending;
  VkDynamicState dynamic_state_enables[2];
  uint8_t dynamic_state_count;
  VkPipelineDynamicStateCreateInfo dynamic_state_info;
};



typedef void (*PFN_strs_create_widget)(strs_app *app, void *pointer);
typedef void (*PFN_strs_update_widget)(strs_app *app, void *pointer);
typedef void (*PFN_strs_while_selected)(strs_app *app, void *pointer);
typedef void (*PFN_strs_on_action)(strs_app *app, void *pointer);

struct strs_widget{
  void *pointer;
  PFN_strs_create_widget create_widget;
  PFN_strs_update_widget update_widget;
  PFN_strs_while_selected while_selected;
  PFN_strs_on_action on_action;
};

STRS_LIB int strs_init();
STRS_LIB strs_app *strs_app_create(int width, int height, strs_string *title);
#ifndef STRS_NOT_MULTI_THREADED
STRS_LIB void strs_app_run(strs_app *app);
#else
typedef void (*PFN_strsExecAsync)();
STRS_LIB void strsAppRun(strs_app *app, PFN_strsExecAsync strsExecAsync);
#endif
STRS_LIB void strs_app_add(strs_app *app, strs_widget *widget);
STRS_LIB void strs_app_free(strs_app *app);
STRS_LIB void strs_terminate();

STRS_LIB void strs_push_vertices(strs_app *app, const strs_vertex *vertices, uint64_t count);
STRS_LIB void strs_pop_back_vertices(strs_app *app, uint64_t count);
STRS_LIB void strs_pop_front_vertices(strs_app *app, uint64_t count);
STRS_LIB void strs_erase_vertices(strs_app *app, uint64_t index);

STRS_LIB void strs_push_indices(strs_app *app, const uint16_t *indices, uint64_t count);
STRS_LIB void strs_pop_back_indices(strs_app *app, uint64_t count);
STRS_LIB void strs_pop_front_indices(strs_app *app, uint64_t count);
STRS_LIB void strs_erase_indices(strs_app *app, uint64_t index);

#endif //STEROS_APP_H
