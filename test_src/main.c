#include <app.h>
#include <ui/button.h>

int main(void) {
	strs_string title = strs_string_create_from_cstr("title", 6);
	strs_app app = strs_app_create(800, 600, &title);
	strs_app_run(app);
	strs_app_free(app);
}
