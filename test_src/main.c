#include <stdio.h>
#include <app.h>

int main(void) {
  strsInit();
  StrApp *app = strsAppCreate(800, 600, "Hello");
  printf("Hello\n");
  strsAppRun(app);
  printf("2hello\n");
  strsAppFree(app);
  strsTerminate();
}

