#include "button.h"

const uint8_t VERTEX_COUNT = 4;
const uint8_t INDEX_COUNT = 6;

STRS_INTERN void buttonFillVertices(float x, float y, float width, float height, strs_vertex *vertices) {
  vertices[0] = (strs_vertex){
    .pos[0] = x,
    .pos[1] = y
  };
  vertices[0] = (strs_vertex){
    .pos[0] = x + width,
    .pos[1] = y
  };
  vertices[0] = (strs_vertex){
    .pos[0] = x,
    .pos[1] = y + height
  };
  vertices[0] = (strs_vertex){
    .pos[0] = x + width,
    .pos[1] = y + height
  };
}

STRS_INTERN void buttonFillIndices(uint16_t *indices) {
  indices[0] = 0;
  indices[1] = 1;
  indices[2] = 2;
  indices[3] = 2;
  indices[4] = 3;
  indices[5] = 0;
}

STRS_INTERN void buttonCreateWidget(strs_app *app, void *pointer) {
  strs_vertex vertices[VERTEX_COUNT];
  uint16_t indices[INDEX_COUNT];
  strs_button *button;

  button = (strs_button*)pointer;

  buttonFillVertices(button->x, button->y, button->width, button->height, vertices);
  buttonFillIndices(indices);

  strs_push_vertices(app, vertices, VERTEX_COUNT);
  strs_push_indices(app, indices, INDEX_COUNT);
}

STRS_INTERN void buttonUpdateWidget(strs_app *app, void *pointer) {

}

STRS_INTERN void buttonWhileSelected(strs_app *app, void *pointer) {

}

strs_button strs_button_create(float x, float y, float width, float height) {
  strs_button button = {
    .x = x,
    .y = y,
    .width = width,
    .height = height,
    .widget = (strs_widget){
      .create_widget = buttonCreateWidget,
      .update_widget = buttonUpdateWidget,
      .while_selected = buttonWhileSelected
    }
  };

  button.widget.pointer = &button;

  return button;
}