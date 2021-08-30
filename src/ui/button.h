#ifndef STEROS_BUTTON_H
#define STEROS_BUTTON_H

#include "steros.h"
#include "app.h"

typedef struct strs_button_tag strs_button;

struct strs_button_tag {
  strs_string title;
  float x;
  float y;
  float width;
  float height;
  strs_widget widget;
};

STRS_LIB strs_button strs_button_create(float x, float y, float width, float height);
STRS_LIB void strs_button_on_action(strs_button *button, PFN_strs_on_action onAction);

#endif //STEROS_BUTTON_H
