// STD
#include <stdio.h>
#include <stdlib.h>

// LIB
#include "window.h"
#define NTD_IMPL_STRING
#include "ntd/string.h"

void null_function(strs_window window, uint32_t width, uint32_t height) {}

typedef struct {
  VkExtensionProperties surface;
  VkExtensionProperties surface_xcb;
} RequiredInstanceExtensions;

typedef void (*PFN_resize_function_pointer)(strs_window window, uint32_t width, uint32_t height);

typedef struct {
  uint64_t width;
  uint64_t height;
  xcb_connection_t *connection;
  xcb_window_t window;
  xcb_generic_event_t *event;
  xcb_intern_atom_reply_t *delete_reply;
  PFN_resize_function_pointer resize_function_pointer;
  const char **array_of_instance_extensions;
  void *user_pointer;
  bool closing;
} internal_strs_window;

strs_window strs_window_create(uint64_t width, uint64_t height, strs_string *title) {
  internal_strs_window *result = malloc(sizeof(internal_strs_window));
  result->closing = false;
  result->resize_function_pointer = null_function;
  result->array_of_instance_extensions = malloc(sizeof(const char*) * 2);
  result->array_of_instance_extensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;
  result->array_of_instance_extensions[1] = VK_KHR_XCB_SURFACE_EXTENSION_NAME;

  result->connection = xcb_connect(NULL, NULL);

  const xcb_setup_t *setup = xcb_get_setup(result->connection);
  xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
  xcb_screen_t *screen = iter.data;

  result->window = xcb_generate_id(result->connection);

  uint32_t mask = XCB_CW_EVENT_MASK;

  uint32_t value_list[] = {
    XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS |
    XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_RESIZE_REDIRECT,
  };

  xcb_create_window(
    result->connection,
    XCB_COPY_FROM_PARENT,
    result->window,
    screen->root,
    0, 0, width, height,
    0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
    screen->root_visual,
    mask,
    value_list);

  xcb_intern_atom_cookie_t protocols_cookie =
    xcb_intern_atom( result->connection, 1, 12, "WM_PROTOCOLS" );
  xcb_intern_atom_reply_t *protocols_reply =
    xcb_intern_atom_reply( result->connection, protocols_cookie, 0 );
  xcb_intern_atom_cookie_t delete_cookie =
    xcb_intern_atom( result->connection, 0, 16, "WM_DELETE_WINDOW" );
  result->delete_reply = xcb_intern_atom_reply( result->connection, delete_cookie, 0 );
  xcb_change_property(
    result->connection,
    XCB_PROP_MODE_REPLACE,
    result->window,
    (*protocols_reply).atom,
    4, 32, 1,
    &result->delete_reply->atom);

  free(protocols_reply);

  xcb_change_property(
    result->connection,
    XCB_PROP_MODE_REPLACE,
    result->window,
    XCB_ATOM_WM_NAME,
    XCB_ATOM_STRING,
    8,
    title->length,
    strs_string_to_cstr(title));

  xcb_map_window(result->connection, result->window);

  xcb_flush(result->connection);

  return (strs_window)result;
}

STRS_INTERN void event_switch_statement(strs_window window) {
  internal_strs_window *intern_window = (internal_strs_window*)window;
  switch(intern_window->event->response_type & ~0x80) {
    case XCB_EXPOSE: {
      xcb_flush(intern_window->connection);
      break;
    }
    case XCB_CLIENT_MESSAGE: {
      if( ((xcb_client_message_event_t*)intern_window->event)->data.data32[0] == intern_window->delete_reply->atom ) {
        intern_window->closing = true;
        free(intern_window->delete_reply);
      }
      break;
    }
    case XCB_RESIZE_REQUEST: {
      xcb_resize_request_event_t *resize_event = (xcb_resize_request_event_t*)intern_window->event;
      intern_window->resize_function_pointer(window, resize_event->width, resize_event->height);
      break;
    }
  }
}

void strs_window_poll_events(strs_window window) {
  internal_strs_window *intern_window = (internal_strs_window*)window;
  intern_window->event = xcb_poll_for_event(intern_window->connection);

  if(intern_window->event) {
    event_switch_statement(window);
  }
  free(intern_window->event);
}

void strs_window_wait_events(strs_window window) {
  internal_strs_window *intern_window = (internal_strs_window*)window;
  intern_window->event = xcb_wait_for_event(intern_window->connection);

  switch(intern_window->event->response_type & ~0x80) {
    case XCB_EXPOSE: {
      xcb_flush(intern_window->connection);
      break;
    }
    case XCB_CLIENT_MESSAGE: {
      if( ((xcb_client_message_event_t*)intern_window->event)->data.data32[0] == intern_window->delete_reply->atom ) {
        intern_window->closing = true;
        free(intern_window->delete_reply);
      }
      break;
    }
    case XCB_RESIZE_REQUEST: {
      xcb_resize_request_event_t *resize_event = (xcb_resize_request_event_t*)intern_window->event;
      intern_window->resize_function_pointer(window, resize_event->width, resize_event->height);
      break;
    }
  }
  free(intern_window->event);
}

void strs_window_free(strs_window window) {
  internal_strs_window *intern_window = (internal_strs_window*)window;
  xcb_destroy_window(intern_window->connection, intern_window->window);
  xcb_disconnect(intern_window->connection);

  free(intern_window->array_of_instance_extensions);
}

bool strs_window_closing(strs_window window) {
  internal_strs_window *intern_window = (internal_strs_window*)window;
  return intern_window->closing;
}

void strs_window_get_size(strs_window window, uint64_t *width, uint64_t *height) {
  internal_strs_window *intern_window = (internal_strs_window*)window;
  xcb_get_geometry_cookie_t cookie = xcb_get_geometry(intern_window->connection, intern_window->window);
  xcb_get_geometry_reply_t *reply = xcb_get_geometry_reply(intern_window->connection, cookie, NULL);
  *width = reply->width;
  *height = reply->height;
  free(reply);
}

VkResult strs_window_create_vulkan_surface(
  strs_window window, VkInstance instance,
  VkSurfaceKHR *pSurface, const VkAllocationCallbacks *pAllocator) {

  internal_strs_window *intern_window = (internal_strs_window*)window;
  VkXcbSurfaceCreateInfoKHR create_info = {
    .sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
    .connection = intern_window->connection,
    .window = intern_window->window
  };

  return vkCreateXcbSurfaceKHR(instance, &create_info, pAllocator, pSurface);
}

const char **strs_window_get_required_instance_extensions(strs_window window, uint32_t *count) {
  internal_strs_window *intern_window = (internal_strs_window*)window;
  *count = 2;
  return intern_window->array_of_instance_extensions;
}

void strs_window_set_user_pointer(strs_window window, void *pointer) {
  internal_strs_window *intern_window = (internal_strs_window*)window;
  intern_window->user_pointer = pointer;
}

void *strs_window_get_user_pointer(strs_window window) {
  internal_strs_window *intern_window = (internal_strs_window*)window;
  return intern_window->user_pointer;
}

void strs_window_set_resize_callback(strs_window window, PFN_resize_function_pointer resize_function_pointer) {
  internal_strs_window *intern_window = (internal_strs_window*)window;
  intern_window->resize_function_pointer = resize_function_pointer;
}
