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

typedef struct {
  vec2 pos;
  vec3 color;
} strs_vertex;

typedef struct {
	uint32_t not_used;
} *strs_app;

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
STRS_LIB strs_app strs_app_create(int width, int height, strs_string *title);
#ifndef STRS_NOT_MULTI_THREADED
STRS_LIB void strs_app_run(strs_app app);
#else
typedef void (*PFN_strsExecAsync)();
STRS_LIB void strsAppRun(strs_app *app, PFN_strsExecAsync strsExecAsync);
#endif
STRS_LIB void strs_app_add(strs_app app, strs_widget *widget);
STRS_LIB void strs_app_free(strs_app app);
STRS_LIB void strs_terminate();

STRS_LIB void strs_push_vertices(strs_app app, const strs_vertex *vertices, uint64_t count);
STRS_LIB void strs_pop_back_vertices(strs_app app, uint64_t count);
STRS_LIB void strs_pop_front_vertices(strs_app app, uint64_t count);
STRS_LIB void strs_erase_vertices(strs_app app, uint64_t index);

STRS_LIB void strs_push_indices(strs_app app, const uint16_t *indices, uint64_t count);
STRS_LIB void strs_pop_back_indices(strs_app app, uint64_t count);
STRS_LIB void strs_pop_front_indices(strs_app app, uint64_t count);
STRS_LIB void strs_erase_indices(strs_app app, uint64_t index);

#endif //STEROS_APP_H
