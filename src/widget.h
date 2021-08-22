#ifndef STEROS_WIDGET_H
#define STEROS_WIDGET_H

#include <stdint.h>

typedef void (*PFN_strsCreateWidget)();
typedef void (*PFN_strsUpdateWidget)();
typedef void (*PFN_strsWhileSelected)();
typedef void (*PFN_strsOnAction)();

typedef struct {
  uint32_t width, height;
  uint32_t maxWidth, maxHeight;
  uint32_t minWidth, minHeight;
  uint32_t x, y;
  PFN_strsCreateWidget createWidget;
  PFN_strsUpdateWidget updateWidget;
  PFN_strsWhileSelected whileSelected;
  PFN_strsOnAction onAction;
} Widget;

#endif //STEROS_WIDGET_H
