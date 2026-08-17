
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <Linux/Mowitz/MwUtils.h>
#include <Linux/Mowitz/MwHtmlParser.h>

struct loader {
	char *prot;
	url_info *(*load)(char *);
};

static struct loader loaders[] = {
	{"http", load_http},
	{"file", load_file},
	{"about", load_about},
	{NULL, NULL}};

/* Reference counted cache */
struct url_cache {
	url_info *ui;
	int ref;
	struct url_cache *next;
} *uc;

void free_url(url_info *u1)
{
	struct url_cache *u;

	if (u1 == NULL) return;

	for (u = uc; u->ui != u1; u = u->next);
	if (u) {
		u->ref--;
		return;		/* don't free right now */
	}
}

/* Input state: ui->base = url of parent doc or NULL,
   ui->url = requested url, ui->state = current state.
   Valid states are -1 = error, 0 = done, 1 = beginning (as above),
   2 = ui->nurl contains normalized url,
   3 = ui->local contains local file name,
   4 = has been redirected once, 5 = has been redirected twice.
*/
url_info *load_url(char *url)
{
	int i;
        char *p;
	struct url_cache *u, *u1;
	url_info *ui = NULL;

	for (u = uc; u; u = u->next) {
		if (!strcmp(url, u->ui->url)) {
			u->ref++;
			return u->ui;
		}
	}
	/* and now it's all up to the protocol handlers */
	p = strchr(url, ':');
	if (p) {
		for (i = 0; loaders[i].prot; i++) {
			char pr[10];
			int n = p-url;
			if (n > 9) n = 9;
			strncpy(pr, url, n);
			pr[n] = '\0';
			if (!MwStrcasecmp(loaders[i].prot, pr)) {
				break;
			}
		}
	}
	if (p && loaders[i].prot) {
		ui = (*loaders[i].load)(url);
	}
	if (ui == NULL) return NULL;
	u1 = MwMalloc(sizeof *u1);
	u1->ui = ui;
	u1->ref = 1;
	u1->next = uc;
	uc = u1;
	return ui;
}
