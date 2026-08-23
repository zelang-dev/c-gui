
#include <gui.h>

int main(int argc, char *argv[]) {
	gui_info ui = {0};
	int r = -1;
	if (gui_window(&ui, "wikipedia", 800, 600, false)) {
		/* Open wikipedia in a 800x400 resizable window */
		r = gui_webview(&ui, "Minimal webview example", "https://en.m.wikipedia.org/wiki/Main_Page", 800, 400);
		gui_webactive(ui);
		gui_webdestroy(ui);
		gui_close(&ui);
		r = (r == 1) ? 0 : -2;
	}

	return r;
}
