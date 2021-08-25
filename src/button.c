#include "button.h"

STRS_INTERN void buttonCreateWidget(StrsApp *app, void *pointer) {
  
}

STRS_INTERN void buttonUpdateWidget(StrsApp *app, void *pointer) {

}

STRS_INTERN void buttonWhileSelected(StrsApp *app, void *pointer) {

}

Button strsButtonCreate(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
  Button button = {
    .widget = (Widget){
      .createWidget = buttonCreateWidget,
      .updateWidget = buttonUpdateWidget,
      .whileSelected = buttonWhileSelected
    }
  };

  button.widget.pointer = &button;

  return button;
}