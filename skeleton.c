

#include <gui.h>

#define W 400
#define H 400

static void fenster_rect(gui_info *f, int x, int y, int w, int h,
	uint32_t c) {
	int row, col;
	for (row = 0; row < h; row++) {
		for (col = 0; col < w; col++) {
			gui_pixel(f, x + col, y + row) = c;
		}
	}
}

// clang-format off
static uint16_t font5x3[] = {0x0000,0x2092,0x002d,0x5f7d,0x279e,0x52a5,0x7ad6,0x0012,0x4494,0x1491,0x017a,0x05d0,0x1400,0x01c0,0x0400,0x12a4,0x2b6a,0x749a,0x752a,0x38a3,0x4f4a,0x38cf,0x3bce,0x12a7,0x3aae,0x49ae,0x0410,0x1410,0x4454,0x0e38,0x1511,0x10e3,0x73ee,0x5f7a,0x3beb,0x624e,0x3b6b,0x73cf,0x13cf,0x6b4e,0x5bed,0x7497,0x2b27,0x5add,0x7249,0x5b7d,0x5b6b,0x3b6e,0x12eb,0x4f6b,0x5aeb,0x388e,0x2497,0x6b6d,0x256d,0x5f6d,0x5aad,0x24ad,0x72a7,0x6496,0x4889,0x3493,0x002a,0xf000,0x0011,0x6b98,0x3b79,0x7270,0x7b74,0x6750,0x95d6,0xb9ee,0x5b59,0x6410,0xb482,0x56e8,0x6492,0x5be8,0x5b58,0x3b70,0x976a,0xcd6a,0x1370,0x38f0,0x64ba,0x3b68,0x2568,0x5f68,0x54a8,0xb9ad,0x73b8,0x64d6,0x2492,0x3593,0x03e0};
// clang-format on
static void fenster_text(gui_info *f, int x, int y, char *s, int scale,
	uint32_t c) {
	while (*s) {
		int dy, dx;
		char chr = *s++;
		if (chr > 32) {
			uint16_t bmp = font5x3[chr - 32];
			for (dy = 0; dy < 5; dy++) {
				for (dx = 0; dx < 3; dx++) {
					if (bmp >> (dy * 3 + dx) & 1) {
						fenster_rect(f, x + dx * scale, y + dy * scale, scale, scale, c);
					}
				}
			}
		}
		x = x + 4 * scale;
	}
}

/* ============================================================
 * A small example demonstrating keymaps/keycodes:
 * - On all platforms keys usually correspond to upper-case ASCII
 * - Enter code is 10, Tab is 9, Backspace is 8, Escape is 27
 * - Delete is 127, Space is 32
 * - Modifiers are: Ctrl=1, Shift=2, Ctrl+Shift=3
 *
 * This demo prints currently pressed keys with modifiers.
 * ============================================================ */
void key_box(__GUI_MENU__) {
	gui_info f = {0};
	gui_window(&f, "Press any key...", W, H, true);
	int64_t now = gui_time();
	int i;
	while (gui_loop(&f) == 0) {
		int has_keys = 0;
		char s[32];
		char *p = s;
		for (i = 0; i < 128; i++) {
			if (f.keys[i]) {
				has_keys = 1;
				*p++ = i;
			}
		}
		*p = '\0';
		fenster_rect(&f, 0, 0, W, H, 0);
		/* draw mouse "pointer" */
		if (f.x > 5 && f.y > 5 && f.x < f.width - 5 && f.y < f.height - 5) {
			fenster_rect(&f, f.x - 3, f.y - 3, 6, 6, f.mouse ? 0xffffff : 0xff0000);
		}
		fenster_text(&f, 8, 8, s, 4, 0xffffff);
		if (has_keys) {
			if (f.mod & 1) {
				fenster_text(&f, 8, 40, "Ctrl", 4, 0xffffff);
			}
			if (f.mod & 2) {
				fenster_text(&f, 8, 80, "Shift", 4, 0xffffff);
			}
		}
		if (f.keys[27]) {
			break;
		}
		int64_t time = gui_time();
		if (time - now < 1000 / 60) {
			gui_sleep(time - now);
		}
		now = time;
	}
	gui_close(&f);
}

/* ============================================================
 * A very minimal example of a Fenster app:
 * - Opens a window
 * - Starts a loop
 * - Changes pixel colours based on some "shader" formula
 * - Sleeps if needed to maintain a frame rate of 60 FPS
 * - Closes a window
 * ============================================================ */
void color_box(__GUI_MENU__) {
	int i, j;
	gui_info f = {0};
	gui_window(&f, "hello", W, H, true);
	uint32_t t = 0;
	int64_t now = gui_time();
	while (gui_loop(&f) == 0) {
		t++;
		for (i = 0; i < W; i++) {
			for (j = 0; j < H; j++) {
			  /* White noise: */
				//gui_pixel(&f, i, j) = (rand() << 16) ^ (rand() << 8) ^ rand();

			  /* Colorful and moving: */
				gui_pixel(&f, i, j) = i * j * t;

			  /* Munching squares: */
				//gui_pixel(&f, i, j) = i ^ j ^ t;
			}
		}

		int64_t time = gui_time();
		if (time - now < 1000 / 60) {
			gui_sleep(time - now);
		}

		now = time;
	}

	gui_close(&f);
}

#define IDC_FIELD1	10
#define IDC_FIELD2 	20
#define IDC_FIELD3	30
#define IDC_FIELD4	40

void form_prompt(__GUI_MENU__) {
	gui_info ui = {0};
	ui_field form[] = {
			{IDC_FIELD1, field_text, "Name", "Free alternative to the Motif XmTextField", 290, 40, 1},
			{IDC_FIELD2, field_secret, "Password", "Fixed Length", 130, 0, 8},
			{IDC_FIELD3, field_text, NULL, "No Echo", 90, 6, 4},
			{IDC_FIELD4, field_text, NULL, "No Pending Delete", 160, 16, 10},
	};

	gui_form(&ui, "Form Fill", form, 4, (ui_form_cb)data);
	gui_active(ui);
	gui_destroy(ui);
}

void message_box(__GUI_MENU__) {
	ui_button buttons = {0};
	char lang_bt_eng[] = "English";

	buttons[0].label = lang_bt_eng;
	int res = gui_message_box(self, "Language",
		"Please choose a language.", buttons, 1);
	printf("messageBox return %d\n", res);

	if (res == 1) {
		buttons[0].label = "No";
		buttons[1].label = "Yes";
		buttons[2].label = "Maybe";
		res = gui_message_box(self, "Answer this question",
			"Do you like to program in C language?", buttons, 3);
		printf("messageBox return %d\n", res);
		if (res == 1) {
			buttons[0].label = "Accept";
			res = gui_message_box(self, "Oops",
				"Unfortunately, you are a bad person.\nThere is nothing I can do for you.", buttons, 1);
			printf("messageBox return %d\n", res);
		}
	}
}

void web_box(__GUI_MENU__) {
	gui_info ui = {0};
	gui_webview(&ui, "Webview", "http://en.wikipedia.org/wiki/WebView", 800, 400);
	gui_webactive(ui);
	gui_webdestroy(ui);
}

#define ID_FILE_OPEN	1
#define ID_FILE_FORM 	2
#define ID_MODE_ALERT	3
#define ID_MODE_ARCADE	4
#define ID_MODE_KEY 	5
#define ID_FILE_SAVE 	6
#define ID_WEB_BOX		7

int main(int argc, char **argv) {
	int error = -1;
	gui_info ui = {0};
	if (gui_window(&ui, "Skeleton", 600, 600, false)
		&& gui_menubar(&ui, 2)) {
		menuitem_t items[] = {
			{ID_FILE_OPEN, "Open", gui_open_dialog, "O", NULL},
			{ID_FILE_SAVE, "Save", gui_save_dialog, "S", NULL},
			{__GUI_SEPERATOR__},
			{ID_FILE_FORM, "Form", form_prompt, "F", NULL},
		};

		menuitem_t items_two[] = {
			{ID_MODE_ALERT, "Alert Box", message_box, "A", NULL},
			{ID_MODE_ARCADE, "Arcade Box", color_box, "B", NULL},
			{ID_MODE_KEY, "Key Box", key_box, "K", NULL},
			{__GUI_SEPERATOR__},
			{ID_WEB_BOX, "Webview Box", web_box, "W", NULL},
		};

		if (!gui_menufont(&ui, lucida)
			|| !gui_menu(&ui, 0, items, 4, 1, "File")
			|| !gui_menu(&ui, 1, items_two, 5, 2, "Mode")) {
			error = -2;
		}

		if (error == -1)
			error = gui_handler(&ui);

		gui_close(&ui);
	}

	return error;
}