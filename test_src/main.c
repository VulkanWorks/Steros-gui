#include <app.h>

int main(void) {
  strsInit();
  StrsStr title = strsStrCreateFromCString("Hello", 6);
  StrsApp *app = strsAppCreate(800, 600, &title);
  strsAppRun(app);
  strsAppFree(app);
  strsTerminate();
}

