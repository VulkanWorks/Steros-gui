#include <stdio.h>
#include <app.h>

int main(void) {
  strInit();
  StrApp *app = strAppCreate(800, 600, "Hello");
  printf("Hello\n");
  strAppRun(app);
  printf("2hello\n");
  strAppFree(app);
  strTerminate();
}

