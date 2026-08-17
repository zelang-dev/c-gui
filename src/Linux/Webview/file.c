
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <Linux/Mowitz/MwUtils.h>
#include <Linux/Mowitz/http.h>

#ifdef USE_DEBUG
#	undef MOWITZ_DATA
#	undef DEFAULT_PIXPATH
#	undef DEFAULT_DATAPATH
#	define MOWITZ_DATA "./share"
#	define DEFAULT_PIXPATH "./share/pixmaps"
#	define DEFAULT_DATAPATH MOWITZ_DATA
#else
#	ifndef MOWITZ_DATA
#		define MOWITZ_DATA "/usr/local/share"
#	endif
#	ifndef DEFAULT_PIXPATH
#		define DEFAULT_PIXPATH "/usr/local/share/pixmaps"
#	endif
#	ifndef DEFAULT_DATAPATH
#		define DEFAULT_DATAPATH MOWITZ_DATA
#	endif
#endif

/* mapping from extension to mime type */
static struct {
	char *ext, *type;
} mt[] = {
	{"txt", "text/plain"},
	{"html", "text/html"},
	{"shtml", "text/html"},
	{"jpg", "image/jpeg"},
	{"jpeg", "image/jpeg"},
	{"png", "image/png"},
	{"gif", "image/gif"},
	{NULL, NULL}};

static char *type(char *ext)
{
	int i;

	for (i = 0; mt[i].ext; i++) {
		if (!MwStrcasecmp(mt[i].ext, ext)) return mt[i].type;
	}
	return "text/plain";
}

url_info *local_file(char *fn, char *url)
{
	char b[1024];
	int n;
	char *p, *t;
	url_info *ui;
	FILE *fp = fopen(fn, "r");
        if (fp == NULL) {
		return NULL;
	}
	p = strrchr(fn, '.');
	if (p) t = type(p+1);
	else t = "text/plain";
        fseek(fp, 0, SEEK_END);
        n = ftell(fp);
        fseek(fp, 0, SEEK_SET);
	ui = MwMalloc(sizeof *ui);
	ui->local = MwStrdup(fn);
	ui->url = MwStrdup(url);
        ui->size = n;
	sprintf(b, "Content-Length: %d\r\nContent-Type: %s\r\n", n, t);
	ui->header = MwStrdup(b);
	return ui;
}

/* The url looks like file:///whatever */
url_info *load_file(char *url)
{
	char *fn;
	fn = strstr(url, "://");
	if (fn) fn += 3;
	else fn = url;	/* worth a shot */
	return local_file(fn, url);
}

url_info *load_about(char *url)
{
        char *p = strrchr(url, ':');
        char fn[1024];
	if (p) p++;
	else p = "error";
        sprintf(fn, "%s/about_%s.html", DEFAULT_DATAPATH, p);
	return local_file(fn, url);
}
