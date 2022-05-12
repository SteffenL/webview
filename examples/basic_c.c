#define WEBVIEW_DEFINE_MAIN
#include "webview/webview.h"

#include <stdio.h>
#include <stdlib.h>

void myFunc(const char *seq, const char *req, void *arg) {
	printf("Params: %s\n", req);
}

int webview_app_main(int argc, char *argv[]) {
	webview_t w = webview_create(0, NULL);
	webview_set_title(w, "Webview Example");
	webview_set_size(w, 480, 320, WEBVIEW_HINT_NONE);
	webview_bind(w, "myFunc", myFunc, NULL);
	webview_navigate(w, "data:text/html,%3Cbutton%20onclick%3D%27myFunc%28%22Foo%20bar%22%29%27%3EClick%20Me%3C%2Fbutton%3E");
	webview_run(w);
	webview_destroy(w);
	return 0;
}
