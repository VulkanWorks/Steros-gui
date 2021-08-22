#ifndef STEROS_BUTTON_H
#define STEROS_BUTTON_H

#include "steros.h"
#include "widget.h"

typedef Widget Button;

STRS_LIB Button strsButtonCreate(uint32_t width, uint32_t height);
STRS_LIB void strsButtonOnAction(Button button, PFN_strsOnAction onAction);

#endif //STEROS_BUTTON_H
