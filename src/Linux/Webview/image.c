#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <Linux/Mowitz/MwUtils.h>
#include <Linux/Mowitz/http.h>
#include <Linux/Mowitz/image.h>

static int lastc;

static image *img_stack = NULL;

static pixel bg = {255, 255, 255};	/* white background */
static pixel fg = {0, 0, 0};		/* black foreground */

static void debug(char *fmt, ...)
{
#ifdef USE_DEBUG
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
#endif
}

static void img_warn(char *fmt, ...)
{
	va_list ap;
	fprintf(stderr, "Warning: ");
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	return;
}

static image *img_new(int npixels)
{
	image *i1 = MwMalloc(sizeof *i1);
	if (npixels) {
		i1->npixels = npixels;
		/* the extra 7 are for P4; think about it */
		i1->pixels = MwMalloc((npixels+7)*sizeof *i1->pixels);
	} else {
		i1->npixels = 0;
		i1->pixels = NULL;
	}
	i1->next = NULL;
	return i1;
}

/* Implement a reference counted cache so we don't have to reload the
   image every time.
   Could also cache the generated XImage, but since we don't want
   to get X involved with this code...
*/
struct img_cache {
	url_info *ui;
	image *img;
	int ref;
	struct img_cache *next;
} *ic;

/* free an image */
void img_free(image *i1)
{
	struct img_cache *i;

	if (i1 == NULL) return;

	for (i = ic; i->img != i1; i = i->next);
	if (i) {
		i->ref--;
		return;	/* we don't actually free anything */
	}

	/* this is the old non-cached thing */
	MwFree(i1->pixels);
	MwFree(i1);
}

image *img_load(char *url)
{
	url_info *ui;
	struct img_cache *i, *i1;
	for (i = ic; i; i = i->next) {
		if (!strcmp(url, i->ui->url)) {
			i->ref++;
			return i->img;
		}
	}
	ui = load_url(url);
	if (ui == NULL) return NULL;
	if (img_read(ui->local) == -1) return NULL;
	i1 = MwMalloc(sizeof *i1);
	i1->ui = ui;
	i1->img = img_pop();
	i1->ref = 1;
	i1->next = ic;
	ic = i1;
	return i1->img;
}

int img_eq_pixel(pixel p, pixel q)
{
	return (p.r == q.r) && (p.g == q.g) && (p.b == q.b);
}

static void readchar(FILE *fpi)
{
	lastc = getc(fpi);
	if (lastc == EOF) img_warn("unexpected end of file");
}

/* Skip past a stretch of white space and/or comments */
static void skip_space(FILE *fpi)
{
	int state = 0;	/* 1 for comment */
	for (;;) {
		if (state) {
			if (lastc == '\n') state = 0;
		} else {
			if (lastc == '#') state = 1;
			else if (!isspace(lastc)) return;
		}
		readchar(fpi);
	}
}

static int read_number(FILE *fpi)
{
	char b[100];
	int n, i = 0;
	while (isdigit(lastc)) {
		b[i++] = lastc;
		readchar(fpi);
	}
	b[i] = '\0';
	n = atoi(b);
	return n;
}

static image *alloc_pixels(FILE *fpi)
{
	image *i1;
	int w, h;

	readchar(fpi);
	skip_space(fpi);
	w = read_number(fpi);
	skip_space(fpi);
	h = read_number(fpi);
	i1 = img_new(w*h);
	i1->width = w;
	i1->height = h;
	return i1;
}

static int hexto255(char *p)
{
	int r, n = strtol(p, NULL, 16)*255;
	static int t[] = {1, 15, 255, 4095, 65535};
	p[4] = '\0';
	r = n/t[strlen(p)];
	return r;
}

static struct {
	char *n;
	pixel p;
} known_colors[] = {
	{"black", {0, 0, 0}},
	{"red", {255, 0, 0}},
	{"green", {0, 255, 0}},
	{"blue", {0, 0, 255}},
	{"cyan", {0, 255, 255}},
	{"magenta", {255, 0, 255}},
	{"yellow", {255, 255, 0}},
	{"white", {255, 255, 255}},
	{NULL}
};

static pixel scan_pixel(char *p)
{
	pixel p1, black = {0, 0, 0};
	char rp[10], gp[10], bp[10];
	char q[10], *r;
	float rf, gf, bf;
	int i;

	for (r = p; *r; r++) *r = tolower(*r);

	if (!strcmp(p, "none")) return bg;
	for (i = 0; known_colors[i].n; i++) {
		if (!strcmp(p, known_colors[i].n)) return known_colors[i].p;
	}
	if (p[0] == '#') {
		switch (strlen(p+1)) {
		case 3:
			q[0] = p[1];
			q[1] = '\0';
			p1.r = hexto255(q);
			q[0] = p[2];
			p1.g = hexto255(q);
			q[0] = p[3];
			p1.b = hexto255(q);
			break;
		case 6:
			strncpy(q, p+1, 2);
			q[2] = '\0';
			p1.r = hexto255(q);
			strncpy(q, p+3, 2);
			p1.g = hexto255(q);
			strncpy(q, p+5, 2);
			p1.b = hexto255(q);
			break;
		case 9:
			strncpy(q, p+1, 3);
			q[3] = '\0';
			p1.r = hexto255(q);
			strncpy(q, p+4, 3);
			p1.g = hexto255(q);
			strncpy(q, p+7, 3);
			p1.b = hexto255(q);
			break;
		case 12:
			strncpy(q, p+1, 4);
			q[4] = '\0';
			p1.r = hexto255(q);
			strncpy(q, p+5, 4);
			p1.g = hexto255(q);
			strncpy(q, p+9, 4);
			p1.b = hexto255(q);
			break;
		default:
			img_warn("'%s' is not a colour", p);
			return black;
		}
	} else if (!strncmp("rgb:", p, 4)) {
		if (sscanf(p+4, "%9[^/]/%9[^/]/%9s", rp, gp, bp) != 3) {
			img_warn("'%s' is not a colour", p);
			return black;
		}
		p1.r = hexto255(rp);
		p1.g = hexto255(gp);
		p1.b = hexto255(bp);
	} else if (!strncmp("rgbi:", p, 5)) {
		if (sscanf(p+5, "%f/%f/%f", &rf, &gf, &bf) != 3) {
			img_warn("'%s' is not a colour", p);
			return black;
		}
		p1.r = 255*rf;
		p1.g = 255*gf;
		p1.b = 255*bf;
	} else {
		img_warn("'%s' is not a colour", p);
		return black;
	}
	return p1;
}

pixel img_average_pixel(int x, int y, int w, int h)
{
	pixel p;
	int i, j;
	int w1 = img_stack->width, h1 = img_stack->height;
	long r, g, b;
	if (x < 0) x = 0;
	if (x > w1-1) x = w1-1;
	if (y < 0) y = 0;
	if (y > h1-1) y = h1-1;
	if (w > w1-x) w = w1-x;
	if (w < 1) w = 1;
	if (h > h1-y) h = h1-y;
	if (h < 1) h = 1;
	r = g = b = 0;
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			p = GET_PIXEL(img_stack, x+j, y+i);
			r += p.r;
			g += p.g;
			b += p.b;
		}
	}
	p.r = r/(w*h);
	p.g = g/(w*h);
	p.b = b/(w*h);
	return p;
}

static int pixel_cmp(const void *p, const void *q)
{
	pixel *p2 = (pixel *)p, *q2 = (pixel *)q;
	return ((long)p2->r+p2->g+p2->b) - ((long)q2->r+q2->g+q2->b);
}

pixel img_median_pixel(int x, int y, int w, int h)
{
	pixel *p, r;
	int i, j;
	int w1 = img_stack->width, h1 = img_stack->height;
	if (x < 0) x = 0;
	if (x > w1-1) x = w1-1;
	if (y < 0) y = 0;
	if (y > h1-1) y = h1-1;
	if (w > w1-x) w = w1-x;
	if (w < 1) w = 1;
	if (h > h1-y) h = h1-y;
	if (h < 1) h = 1;
	p = MwMalloc(w*h*sizeof *p);
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			p[i*w+j] = GET_PIXEL(img_stack, x+j, y+i);
		}
	}
	qsort(p, w*h, sizeof *p, pixel_cmp);
	r = p[w*h/2];
	MwFree(p);
	return r;
}

typedef struct palette {
	char *n;
	pixel p;
	struct palette *next;
} palette;

static palette *palette_lookup(palette *p, char *n)
{
	palette *q;
	for (q = p; q; q = q->next) {
		if (!strcmp(n, q->n)) break;
	}
	return q;
}

static palette *palette_add(palette *pa, char *n, pixel p)
{
	palette *q = palette_lookup(pa, n);
	if (q) return pa;
	q = MwMalloc(sizeof *q);
	q->n = MwMalloc(strlen(n)+1);
	strcpy(q->n, n);
	q->p = p;
	q->next = pa;
	return q;
}

static void palette_free(palette *pa)
{
	if (pa == NULL) return
	palette_free(pa->next);
	MwFree(pa);
}

static int palette_size(palette *pa)
{
	if (pa == NULL) return 0;
	return palette_size(pa->next)+1;
}

/* ascii pbm */
static image *read_p1(FILE *fpi)
{
	image *i1;
	int i, n;
	pixel *pixels;
	i1 = alloc_pixels(fpi);
	pixels = i1->pixels;
	for (i = 0; i < i1->npixels; i++) {
		skip_space(fpi);
		n = read_number(fpi);
		if (n) pixels[i] = fg;
		else pixels[i] = bg;
	}
	return i1;
}

/* image output */
static int write_p1(image *i1, FILE *fpo)
{
	int i, w = i1->width, h = i1->height, m, n = w*h;
	pixel *pixels = i1->pixels;
	fprintf(fpo, "P1\n%d\n%d\n", w, h);
	for (i = 0; i < n; i++) {
		m = (int)pixels[i].r+pixels[i].g+pixels[i].b;
		fprintf(fpo, "%d\n", (m>381)?1:0);
	}
	return 0;
}

/* ascii pgm */
static image *read_p2(FILE *fpi)
{
	image *i1;
	int i, n, m;
	pixel *pixels;
	i1 = alloc_pixels(fpi);
	pixels = i1->pixels;
	skip_space(fpi);
	m = read_number(fpi);	/* max grey */
	for (i = 0; i < i1->npixels; i++) {
		skip_space(fpi);
		n = read_number(fpi)*255/m;
		pixels[i].r = pixels[i].g = pixels[i].b = n;
	}
	return i1;
}

static int write_p2(image *i1, FILE *fpo)
{
	int i;
	pixel *pixels = i1->pixels;
	int n = i1->width*i1->height;
	int m;
	fprintf(fpo, "P2\n%d\n%d\n255\n", i1->width, i1->height);
	for (i = 0; i < n; i++) {
		m = (int)pixels[i].r+pixels[i].g+pixels[i].b;
		fprintf(fpo, "%d\n", m/3);
	}
	return 0;
}

/* ascii ppm */
static image *read_p3(FILE *fpi)
{
	image *i1;
	int i, m;
	pixel *pixels;
	i1 = alloc_pixels(fpi);
	pixels = i1->pixels;
	skip_space(fpi);
	m = read_number(fpi);	/* max colour component */
	for (i = 0; i < i1->npixels; i++) {
		skip_space(fpi);
		pixels[i].r = read_number(fpi)*255/m;
		skip_space(fpi);
		pixels[i].g = read_number(fpi)*255/m;
		skip_space(fpi);
		pixels[i].b = read_number(fpi)*255/m;
	}
	return i1;
}

static int write_p3(image *i1, FILE *fpo)
{
	int i;
	pixel *pixels = i1->pixels;
	int n = i1->width*i1->height;
	fprintf(fpo, "P3\n%d\n%d\n255\n", i1->width, i1->height);
	for (i = 0; i < n; i++) {
		fprintf(fpo, "%d %d %d\n",
			(int)pixels[i].r, (int)pixels[i].g, (int)pixels[i].b);
	}
	return 0;
}

/* raw pbm */
static image *read_p4(FILE *fpi)
{
	image *i1;
	int i, j;
	pixel *pixels;
	i1 = alloc_pixels(fpi);
	pixels = i1->pixels;
	for (i = 0; i < i1->npixels; i += i1->width) {
		for (j = 0; j < i1->width; j += 8) {
			readchar(fpi);
			pixels[i+j] = (lastc & 128) ? fg : bg;
			pixels[i+j+1] = (lastc & 64) ? fg : bg;
			pixels[i+j+2] = (lastc & 32) ? fg : bg;
			pixels[i+j+3] = (lastc & 16) ? fg : bg;
			pixels[i+j+4] = (lastc & 8) ? fg : bg;
			pixels[i+j+5] = (lastc & 4) ? fg : bg;
			pixels[i+j+6] = (lastc & 2) ? fg : bg;
			pixels[i+j+7] = (lastc & 1) ? fg : bg;
		}
	}
	return i1;
}

/* raw pgm */
static image *read_p5(FILE *fpi)
{
	image *i1;
	int i, m;
	pixel *pixels;
	i1 = alloc_pixels(fpi);
	pixels = i1->pixels;
	skip_space(fpi);
	m = read_number(fpi);	/* max grey */
	for (i = 0; i < i1->npixels; i++) {
		readchar(fpi);
		pixels[i].r = pixels[i].g = pixels[i].b = (lastc & 255)*255/m;
	}
	return i1;
}

/* raw ppm */
static image *read_p6(FILE *fpi)
{
	image *i1;
	int i, m;
	pixel *pixels;
	i1 = alloc_pixels(fpi);
	pixels = i1->pixels;
	skip_space(fpi);
	m = read_number(fpi);	/* max colour component */
	for (i = 0; i < i1->npixels; i++) {
		readchar(fpi);
		pixels[i].r = (lastc & 255)*255/m;
		readchar(fpi);
		pixels[i].g = (lastc & 255)*255/m;
		readchar(fpi);
		pixels[i].b = (lastc & 255)*255/m;
	}
	return i1;
}

static image *read_pnm(FILE *fpi)
{
	readchar(fpi);
	if (lastc == 'P') {	/* possible P?M */
		readchar(fpi);
		switch (lastc) {
		case '1':
			return read_p1(fpi);
		case '2':
			return read_p2(fpi);
		case '3':
			return read_p3(fpi);
		case '4':
			return read_p4(fpi);
		case '5':
			return read_p5(fpi);
		case '6':
			return read_p6(fpi);
		default:
			img_warn("not pnm (bad magic in PNM file)");
			return NULL;
		}
	}
	img_warn("unknown PNM format");
	return NULL;
}

static int write_gba(image *i1, FILE *fpo)
{
	int i;
	int cm[256];	/* color map */
	int rcm[32768];	/* reverse color map */
	int r, g, b, bgr;
	pixel *pixels = i1->pixels;
	int n = i1->width*i1->height;
	int index, index0 = 10;
	FILE *fp;
	for (i = 0; i < 256; i++) cm[i] = -1;
	for (i = 0; i < 32768; i++) rcm[i] = -1;
	fprintf(fpo, "/* Image data size %dx%d */\n",
		i1->width, i1->height);
	fprintf(fpo, "/* First colormap index is %d */\n", 10);
	fprintf(fpo, "/* Colormap in gba.colormap */\n");
	for (i = 0; i < n; i++) {
		r = pixels[i].r*31/255;
		g = pixels[i].g*31/255;
		b = pixels[i].b*31/255;
		bgr = (b<<10)+(g<<5)+r;
		index = rcm[bgr];
		if (index == -1) {
			index = index0;
			rcm[bgr] = index0;
			cm[index] = bgr;
			index0++;
		}
		if (index > 255) {
			img_warn("Too many colors");
			return -1;
		}
		fprintf(fpo, "%d,\n", index);
	}
	fp = fopen("gba.colormap", "w");
	if (fp == NULL) {
		img_warn("Can't write colormap");
		return -1;
	}
	fprintf(fp, "/* Image colormap */\n");
	fprintf(fp, "/* First index is %d, first unused %d */\n",
		10, index0);
	for (i = 10; i < index0; i++) {
		fprintf(fp, "%d,\n", cm[i]);
	}
	fclose(fp);
	return 0;
}

static void read_string(char *b, int n, FILE *fpi)
{
	int i;

	readchar(fpi);
	for (i = 0; lastc != '"' && i < n-1; i++) {
		b[i] = lastc;
		readchar(fpi);
	}
	b[i] = '\0';
}

/* used to sort the array in alphabetic order by name */
static int xpm_comp(const void *p, const void *q)
{
	palette *p1 = (palette *)p;
	palette *q1 = (palette *)q;
	return strcmp(p1->n, q1->n);
}

static pixel xpm_find(palette *colors, int ncolors, char *name)
{
	int lower, upper, i, d;

#if 1	/* binary search */
	lower = 0;
	upper = ncolors-1;
	while (lower <= upper) {
		i = (lower+upper)/2;
		d = strcmp(name, colors[i].n);
		if (d == 0) {
			return colors[i].p;
		} else if (d > 0) {
			lower = i+1;
		} else {
			upper = i-1;
		}
	}
#else	/* linear search */
	for (i = 0; i < ncolors; i++) {
		if (!strcmp(name, colors[i].n)) return colors[i].p;
	}
#endif
	return bg; /* no match found, return background */
}

static image *read_external(FILE *fpi, char *cmd)
{
	image *i1;
	int c;
	char b[4096];
	FILE *fp = fopen("/tmp/fnord", "w");
	if (fp == NULL) {
		img_warn("can't write temp file");
		return NULL;
	}
	while ((c = getc(fpi)) != EOF) putc(c, fp);
	fclose(fp);
	snprintf(b, sizeof b, cmd, "/tmp/fnord");
	fp = popen(b, "r");
	if (fp == NULL) {
		img_warn("can't read temp file");
		return NULL;
	}
	i1 = read_pnm(fp);
	pclose(fp);
	return i1;
}

/* this format is a bloody mess; someone ought to be shot */
static image *read_xpm(FILE *fpi)
{
	image *i1;
	pixel p;
	char b[1024], c[10];
	char *q;
	int i, j, w, h, n, cpp;
	palette *pa = NULL, *pb;
	palette *colors;

	/* anything outside a string is crap */
	while (lastc != '"') readchar(fpi);

	/* first string is geometry */
	read_string(b, sizeof b, fpi);
	if (sscanf(b, "%d %d %d %d", &w, &h, &n, &cpp) < 4) {
		img_warn("Bogus geometry");
		return NULL;
	}
	if (cpp > 8) {
		img_warn("yeah, right...");
		return NULL;
	}
	i1 = img_new(w*h);
	i1->width = w;
	i1->height = h;
	/* then we start reading colours... */
	for (i = 0; i < n; i++) {
		int a = 0, state;
		char *cname, na[100];
debug("Color %d of %d\n", i, n);
		readchar(fpi);
		while (lastc != '"') readchar(fpi);
		read_string(b, sizeof b, fpi);
		strncpy(na, b, cpp);
		na[cpp] = '\0';
		q = b+cpp;
		state = 0;
		cname = NULL;
		for (q = b+cpp; *q; q++) {
			switch (state) {
			case 0:	/* initial space */
				if (!isspace(*q)) {
					a = *q;
					state = 1;
				}
				break;
			case 1: /* required space */
				if (!isspace(*q))
					img_warn("'%c' is no space", *q);
				state = 2;
				break;
			case 2:	/* optional space */
				if (!isspace(*q)) {
					if (a == 'c') cname = q;
					state = 3;
				}
				break;
			case 3:	/* colour name */
				if (isspace(*q)) state = 0;
				break;
			default:
				img_warn("you can't be here");
				return NULL;
			}
		}
		if (cname == NULL) p = bg;
		else p = scan_pixel(cname);
		pa = palette_add(pa, na, p);
	}
	/* we now need to store the colours differently, or die waiting... */
	colors = MwMalloc(n*sizeof *colors);
	pb = pa;
	for (i = 0; i < n && pb; i++) {
		colors[i] = *pb;
		pb = pb->next;
	}
	if (i != n) {
		img_warn("Wrong number of colors");
		return NULL;
	}
	qsort(colors, n, sizeof *colors, xpm_comp);
for (i = 0; i < n; i++)
debug("%s ", colors[i].n);
	for (i = 0; i < h; i++) {
debug("Scanline %d of %d\n", i, h);
		readchar(fpi);
		while (lastc != '"') readchar(fpi);
		read_string(b, sizeof b, fpi);
		for (j = 0; j < w; j++) {
			strncpy(c, b+cpp*j, cpp);
			c[cpp] = '\0';
			p = xpm_find(colors, n, c);
			PUT_PIXEL(i1, j, i, p);
		}
	}
	MwFree(colors);
	palette_free(pa);
	return i1;
}

static int write_xpm(image *i1, FILE *fpo)
{
	palette *pa = NULL, *pb;
	int i, j, k, m, n, cpp, w = i1->width, h = i1->height;
	pixel p, *pixels;
	char b[100];
	char pn[] =	" .-"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"0123456789";

	for (i = 0; i < h; i++) {
debug("%d\n", i);
		for (j = 0; j < w; j++) {
			p = GET_PIXEL(i1, j, i);
			sprintf(b, "%02x%02x%02x",
				(int)p.r, (int)p.g, (int)p.b);
			pa = palette_add(pa, b, p);
		}
	}
	n = palette_size(pa);
	cpp = 0;
	m = n;
	do {
		cpp++;
		m /= 64;
	} while (m);
	fprintf(fpo,
		"/* XPM */\n"
		"static char * fnord_xpm[] = {\n"
		"\"%d %d %d %d\",\n",
		w, h, n, cpp);
	pixels = MwMalloc(n*sizeof *pixels);
	i = 0;
	pb = pa;
	while (pb) {
		pixels[i++] = pb->p;
		pb = pb->next;
	}
	palette_free(pa);
	for (i = 0; i < n; i++) {
		m = i;
		fprintf(fpo, "\"");
		for (j = 0; j < cpp; j++) {
			fprintf(fpo, "%c", pn[m&63]);
			m /= 64;
		}
		fprintf(fpo, " c #%02x%02x%02x\",\n",
			(int)pixels[i].r, (int)pixels[i].g, (int)pixels[i].b);
	}
	for (i = 0; i < h; i++) {
debug("%d\n", i);
		fprintf(fpo, "\"");
		for (j = 0; j < w; j++) {
			p = GET_PIXEL(i1, j, i);
			for (m = 0; m < n; m++) {
				if (img_eq_pixel(p, pixels[m])) break;
			}
			if (m == n) m = 0;
			for (k = 0; k < cpp; k++) {
				fprintf(fpo, "%c", pn[m&63]);
				m /= 64;
			}
		}
		if (i < h-1) fprintf(fpo, "\",\n");
		else fprintf(fpo, "\"};\n");
	}
	return 0;
}

static image *read_xbm(FILE *fpi)
{
	img_warn("Can't read XBM");
	return NULL;
}

static int write_xbm(image *i1, FILE *fpo)
{
	int i, j, w = i1->width, h = i1->height, m, n;
	pixel p;
	fprintf(fpo,
		"#define fnord_width %d\n"
		"#define fnord_height %d\n"
		"static unsigned char fnord_bits[] = {\n",
		w, h);
	for (i = 0; i < h; i++) {
		n = 0;
		for (j = 0; j < w; j++) {
			p = GET_PIXEL(i1, j, i);
			m = (int)p.r+p.g+p.b;
			n = 2*n+(m>381);
			if ((j & 7) == 7 || (j+1) == w) {
				fprintf(fpo, "0x%02x", n);
				n = 0;
				if ((i+1) == h && (j+1) == w) {
					fprintf(fpo, "};\n");
				} else {
					fprintf(fpo, ",\n");
				}
			}
		}
	}
	return 0;
}

static image *read_jpeg(FILE *fpi)
{
	return read_external(fpi, "djpeg %s");
}

static image *read_gif(FILE *fpi)
{
	return read_external(fpi, "giftopnm %s");
}

static image *read_tif(FILE *fpi)
{
	return read_external(fpi, "tifftopnm %s");
}

static image *read_png(FILE *fpi)
{
	return read_external(fpi, "pngtopnm %s");
}

static image *read_bmp(FILE *fpi)
{
	return read_external(fpi, "bmptoppm %s");
}

static image *read_unknown(FILE *fpi)
{
	return read_external(fpi, "anytopnm %s");
}

struct {
	char *name;
	image *(*load)(FILE *);
	int (*save)(image *, FILE *);
} img_io[] = {
	{"P1", read_pnm, write_p1},
	{"PBM", read_pnm, write_p1},
	{"P2", read_pnm, write_p2},
	{"PGM", read_pnm, write_p2},
	{"P3", read_pnm, write_p3},
	{"PPM", read_pnm, write_p3},
	{"GBA", NULL, write_gba},
	{"XPM", read_xpm, write_xpm},
	{"XBM", read_xbm, write_xbm},
	{"JPG", read_jpeg, NULL},
	{"JPEG", read_jpeg, NULL},
	{"GIF", read_gif, NULL},
	{"TIF", read_tif, NULL},
	{"TIFF", read_tif, NULL},
	{"PNG", read_png, NULL},
	{"BMP", read_bmp, NULL},
	{NULL, NULL, NULL}
};

int img_alias(void)
{
	pixel p, p1, p2, p3, p4, p5, p6, p7, p8;
	image *i2;
	int i, j, w = img_stack->width, h = img_stack->height;

	i2 = img_new(w*h);
	i2->width = w;
	i2->height = h;
	for (j = 0; j < w; j++) {
		p = GET_PIXEL(img_stack, j, 0);
		PUT_PIXEL(i2, j, 0, p);
		p = GET_PIXEL(img_stack, j, h-1);
		PUT_PIXEL(i2, j, h-1, p);
	}
	for (i = 1; i < h-1; i++) {
		p = GET_PIXEL(img_stack, 0, i);
		PUT_PIXEL(i2, 0, i, p);
		p = GET_PIXEL(img_stack, w-1, i);
		PUT_PIXEL(i2, w-1, i, p);
		for (j = 1; j < w-1; j++) {
			p = GET_PIXEL(img_stack, j, i);
			if (img_eq_pixel(p, bg)) {
				p1 = GET_PIXEL(img_stack, j, i-1);
				p2 = GET_PIXEL(img_stack, j-1, i);
				p3 = GET_PIXEL(img_stack, j+1, i);
				p4 = GET_PIXEL(img_stack, j, i+1);
				if ((img_eq_pixel(p1, fg) || img_eq_pixel(p4, fg)) &&
				     (img_eq_pixel(p2, fg) || img_eq_pixel(p3, fg))) {
					p5 = GET_PIXEL(img_stack, j-1, i-1);
					p6 = GET_PIXEL(img_stack, j+1, i-1);
					p7 = GET_PIXEL(img_stack, j-1, i+1);
					p8 = GET_PIXEL(img_stack, j+1, i+1);
					p.r = p.r/2+((int)p1.r+p2.r+p3.r+p4.r+
						p5.r+p6.r+p7.r+p8.r)/16;
					p.g = p.g/2+((int)p1.g+p2.g+p3.g+p4.g+
						p5.g+p6.g+p7.g+p8.g)/16;
					p.b = p.b/2+((int)p1.b+p2.b+p3.b+p4.b+
						p5.b+p6.b+p7.b+p8.b)/16;
				}
			}
			PUT_PIXEL(i2, j, i, p);
		}
	}
	i2->next = img_stack->next;
	img_free(img_stack);
	img_stack = i2;
	return 0;
}

int img_bg(char *p)
{
	bg = scan_pixel(p);
	return 0;
}

int img_cd(char *p)
{
	return chdir(p);
}

int img_crop(void)
{
	image *i1 = img_stack;
	pixel p1, p2;
	int lc, rc, tr, br, i, w, h, n;

	p1 = i1->pixels[0];
	w = i1->width;
	h = i1->height;
	for (tr = 0; tr < h; tr++) {
		for (i = 0; i < w; i++) {
			n = tr*h+i;
			p2 = i1->pixels[n];
			if (p2.r != p1.r || p2.g != p1.g || p2.b != p1.b) {
				goto L1;	/* double break */
			}
		}
	}
L1:	/* tr now contains the topmost line we want to keep */

	for (br = h-1; br > 0; br--) {
		for (i = 0; i < w; i++) {
			n = br*h+i;
			p2 = i1->pixels[n];
			if (p2.r != p1.r || p2.g != p1.g || p2.b != p1.b) {
				goto L2;
			}
		}
	}
L2:	/* br now contains the bottommost line we want to keep */

	for (lc = 0; lc < w; lc++) {
		for (i = tr; i <= br; i++) {
			n = i*h+lc;
			p2 = i1->pixels[n];
			if (p2.r != p1.r || p2.g != p1.g || p2.b != p1.b) {
				goto L3;
			}
		}
	}
L3:	/* lc now contains the leftmost column we want to keep */

	for (rc = w-1; rc > 0; rc--) {
		for (i = tr; i <= br; i++) {
			n = i*h+rc;
			p2 = i1->pixels[n];
			if (p2.r != p1.r || p2.g != p1.g || p2.b != p1.b) {
				goto L4;
			}
		}
	}
L4:	/* rc now contains the rightmost column we want to keep */
	return img_cut(lc, tr, rc-lc+1, br-tr+1);
}

int img_cut(int x, int y, int w, int h)
{
	image i2 = *img_stack;
	pixel p;
	int i, j;

	if (w < 0) w = 0;
	if (h < 0) h = 0;
	img_stack->width = w;
	img_stack->height = h;
	img_stack->npixels = w*h;

	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			p = GET_PIXEL(&i2, x+j, y+i);
			PUT_PIXEL(img_stack, j, i, p);
		}
	}
	return 0;
}

int img_depth(int n)
{
	int i;
	pixel p;

	for (i = 0; i < img_stack->npixels; i++) {
		p = img_stack->pixels[i];
		p.r = p.r*n/255;
		p.r = p.r*255/n;
		p.g = p.g*n/255;
		p.g = p.g*255/n;
		p.b = p.b*n/255;
		p.b = p.b*255/n;
		img_stack->pixels[i] = p;
	}
	return 0;
}

int img_despeckle(int w, int h)
{
	int w1, h1;
	int i, j;
	image *i2;
	pixel p;

	if (img_stack == NULL) return -1;
	w1 = img_stack->width, h1 = img_stack->height;
	i2 = img_new(img_stack->npixels);
	if (!(w & h & 1)) {
		img_warn("even arg (%d,%d) to smooth", w, h);
		return -1;
	}

	i2->width = w1;
	i2->height = h1;

	for (i = 0; i < h1; i++) {
		for (j = 0; j < w1; j++) {
			p = img_median_pixel(j-(w/2), i-(h/2), w, h);
			PUT_PIXEL(i2, j, i, p);
		}
	}
	i2->next = img_stack->next;
	img_free(img_stack);
	img_stack = i2;
	return 0;
}

int img_drop(void)
{
	image *i1 = img_stack;
	if (i1 == NULL) return -1;
	img_stack = img_stack->next;
	img_free(i1);
	return 0;
}

int img_dup(void)
{
	image *i1 = img_new(img_stack->npixels);
	if (img_stack == NULL) return -1;
	i1->width = img_stack->width;
	i1->height = img_stack->height;
	memcpy(i1->pixels, img_stack->pixels, img_stack->npixels);
	i1->next = img_stack;
	img_stack = i1;
	return 0;
}

int img_enlarge(int n)
{
	return img_size(n*img_stack->width, n*img_stack->height);
}

int img_fg(char *p)
{
	fg = scan_pixel(p);
	return 0;
}

int img_fit(int w, int h)
{
	float w1, h1, c1;
	if (img_stack == NULL) return -1;
	w1 = img_stack->width;
	h1 = img_stack->height;
	if (w1 == 0 || h1 == 0) return -1;
	c1 = w1/h1;
	if (w1 > w) {
		w1 = w;
		h1 = w1/c1;
	}
	if (h1 > h) {
		h1 = h;
		w1 = h1/c1;
	}
	return img_size(w1, h1);
}

int img_gamma(float r, float g, float b)
{
	pixel p;
	int i, m;

	if (img_stack == NULL) return -1;
	for (i = 0; i < img_stack->npixels; i++) {
		p = img_stack->pixels[i];
		m = r*p.r;
		if (m > 255) m = 255;
		p.r = m;
		m = g*p.g;
		if (m > 255) m = 255;
		p.g = m;
		m = b*p.b;
		if (m > 255) m = 255;
		p.b = m;
		img_stack->pixels[i] = p;
	}
	return 0;
}

int img_invert(void)
{
	pixel p;
	int i;
	if (img_stack == NULL) return -1;
	for (i = 0; i < img_stack->npixels; i++) {
		p = img_stack->pixels[i];
		p.r = 255-p.r;
		p.g = 255-p.g;
		p.b = 255-p.b;
		img_stack->pixels[i] = p;
	}
	return 0;
}

int img_lr(void)
{
	pixel p1, p2;
	int i, j;
	if (img_stack == NULL) return -1;
	for (i = 0; i < img_stack->height; i++) {
		for (j = 0; j < img_stack->width/2; j++) {
			p1 = GET_PIXEL(img_stack, j, i);
			p2 = GET_PIXEL(img_stack, img_stack->width-j-1, i);
			PUT_PIXEL(img_stack, j, i, p2);
			PUT_PIXEL(img_stack, img_stack->width-j-1, i, p1);
		}
	}
	return 0;
}

int img_makeicons(int w, int h, char *tndir)
{
	image *i1;
	DIR *dp = opendir(".");
	struct dirent *de;
	char *q, *fn, b[4096];
	FILE *fpi;
	image *(*load)(FILE *);
	int i;
	if (dp == NULL) {
		img_warn("Can't open current directory");
		return -1;
	}
	while ((de = readdir(dp)) != NULL) {
		fn = de->d_name;
		load = NULL;
		if ((q = strrchr(fn, '.'))) {
        	        strncpy(b, q+1, sizeof b);
	                b[sizeof b-1] = '\0';
	                for (q = b; *q; q++) *q = toupper(*q);
	                for (i = 0; img_io[i].name; i++) {
	                        if (!strcmp(b, img_io[i].name)) {
	                                load = img_io[i].load;
	                                break;
	                        }
	                }
		}
		if (load == NULL) continue;
		fpi = fopen(fn, "r");
		if (fpi == NULL) continue;
		i1 = (*load)(fpi);
		fclose(fpi);
		if (i1 == NULL) continue;
		i1->next = img_stack;
		img_stack = i1;
		img_fit(w, h);
		img_depth(6);
		snprintf(b, sizeof b, "%s/%s.xpm", tndir, fn);
		img_write(b);
	}
	closedir(dp);
	return 0;
}

int img_margin(char *p)
{
	return -1;
}

int img_noop(char *p)
{
	fprintf(stderr,
		"No operation; "
		"this is probably because '%s' is unimplemented\n", p);
	return 0;
}

int img_pixels(int n)
{
	return -1;
}

image *img_pop(void)
{
	image *i;
	if (img_stack == NULL) return NULL;
	i = img_stack;
	img_stack = img_stack->next;
	return i;
}

int img_push(image *i)
{
	i->next = img_stack;
	img_stack = i;
	return 0;
}

int img_r90(void)
{
	int i, j;
	int w1, h1, w2, h2;
	pixel p1;
	image *i2;
	if (img_stack == NULL) return -1;
	w1 = img_stack->width, h1 = img_stack->height;
	i2 = img_new(w1*h1);
	w2 = i2->width = h1;
	h2 = i2->height = w1;
	for (i = 0; i < h1; i++) {
		for (j = 0; j < w1; j++) {
			p1 = GET_PIXEL(img_stack, j, i);
			PUT_PIXEL(i2, i, h2-j-1, p1);
		}
	}
	i2->next = img_stack->next;
	img_free(img_stack);
	img_stack = i2;
	return 0;
}

int img_r180(void)
{
	int i, j;
	int w1 = img_stack->width, h1 = img_stack->height, w2, h2;
	pixel p1;
	image *i2 = img_new(w1*h1);
	w2 = i2->width = w1;
	h2 = i2->height = h1;
	for (i = 0; i < h1; i++) {
		for (j = 0; j < w1; j++) {
			p1 = GET_PIXEL(img_stack, j, i);
			PUT_PIXEL(i2, w2-j-1, h2-i-1, p1);
		}
	}
	i2->next = img_stack->next;
	img_free(img_stack);
	img_stack = i2;
	return 0;
}

int img_r270(void)
{
	int i, j;
	int w1 = img_stack->width, h1 = img_stack->height, w2, h2;
	pixel p1;
	image *i2 = img_new(w1*h1);
	w2 = i2->width = h1;
	h2 = i2->height = w1;
	for (i = 0; i < h1; i++) {
		for (j = 0; j < w1; j++) {
			p1 = GET_PIXEL(img_stack, j, i);
			PUT_PIXEL(i2, w2-i-1, j, p1);
		}
	}
	i2->next = img_stack->next;
	img_free(img_stack);
	img_stack = i2;
	return 0;
}

/* If format is specified as FMT:filename, trust the FMT. Otherwise guess. */
int img_read(char *fn)
{
	char *q, b[100];
	image *i1 = NULL;
	FILE *fpi;
	int i, n;
	image *(*load)(FILE *) = NULL;

	q = strchr(fn, ':');
	if (q) {
		n = q-fn;
		if (n > sizeof b-1) n = sizeof b-1;
		strncpy(b, fn, n);
		fn = q+1;
		b[n] = '\0';
		for (q = b; *q; q++) *q = toupper(*q);
		for (i = 0; img_io[i].name; i++) {
			if (!strcmp(b, img_io[i].name)) {
				load = img_io[i].load;
				break;
			}
		}
		if (load == NULL) {
			img_warn("Unknown format: '%s'\n", b);
			return -1;
		}
	} else if ((q = strrchr(fn, '.'))) {
		strncpy(b, q+1, sizeof b);
		b[sizeof b-1] = '\0';
		for (q = b; *q; q++) *q = toupper(*q);
		for (i = 0; img_io[i].name; i++) {
			if (!strcmp(b, img_io[i].name)) {
				load = img_io[i].load;
				break;
			}
		}
	}
	if (load == NULL) {
		img_warn("File '%s' has unknown format", fn);
		load = read_unknown;
	}
	if (!strcmp(fn, "-")) fpi = stdin;
	else fpi = fopen(fn, "r");
	if (fpi == NULL) {
		img_warn("Can't read image '%s'", fn);
		return -1;
	}

	i1 = (*load)(fpi);
	if (fpi != stdin) fclose(fpi);
	if (i1 == NULL) return -1;

	i1->next = img_stack;
	img_stack = i1;
	return 0;
}

int img_rotate(int n)
{
	debug("Cheating because this is unimplemented\n");
	n %= 360;
	if (n < 45) return 0;
	else if (n < 135) return img_r90();
	else if (n < 225) return img_r180();
	else if (n < 315) return img_r270();
	return 0;
}

int img_scale(float w, float h)
{
	float w1 = w*img_stack->width;
	float h1 = h*img_stack->height;
	if (w1 == 0 || h1 == 0) return 0;
	return img_size(w1, h1);
}

int img_scroll(int x, int y)
{
	int i, j;
	int w1 = img_stack->width, h1 = img_stack->height, w2, h2;
	pixel p1;
	image *i2 = img_new(w1*h1);
	if (i2 == NULL) return -1;
	w2 = i2->width = w1;
	h2 = i2->height = h1;
	for (i = 0; i < h1; i++) {
		for (j = 0; j < w1; j++) {
			p1 = GET_PIXEL(img_stack, j, i);
			PUT_PIXEL(i2, (j+x)%w2, (i+y)%h2, p1);
		}
	}
	i2->next = img_stack->next;
	img_free(img_stack);
	img_stack = i2;
	return 0;
}

int img_sh(char *cmd)
{
	return system(cmd);
}

int img_shear(int a)
{
	return -1;
}

int img_size(int w, int h)
{
	int w1 = img_stack->width, h1 = img_stack->height;
	int w2, h2;
	int i, j;
	pixel p1;
	image *i2;

	if (w) w2 = w;
	else w2 = w1;
	if (h) h2 = h;
	else h2 = h1;

	i2 = img_new(w2*h2);
	if (i2 == NULL) return -1;
	i2->width = w2;
	i2->height = h2;
	for (i = 0; i < h2; i++) {
		for (j = 0; j < w2; j++) {
			p1 = GET_PIXEL(img_stack, (j*w1)/w2, (i*h1)/h2);
			PUT_PIXEL(i2, j, i, p1);
		}
	}
	i2->next = img_stack->next;
	img_free(img_stack);
	img_stack = i2;
	return 0;
}

int img_smooth(int w, int h)
{
	int w1 = img_stack->width, h1 = img_stack->height;
	int i, j;
	image *i2 = img_new(img_stack->npixels);
	pixel p;

	if (!(w & h & 1)) {
		img_warn("even arg (%d,%d) to smooth", w, h);
		return -1;
	}

	i2->width = w1;
	i2->height = h1;

	for (i = 0; i < h1; i++) {
		for (j = 0; j < w1; j++) {
			p = img_average_pixel(j-(w/2), i-(h/2), w, h);
			PUT_PIXEL(i2, j, i, p);
		}
	}
	i2->next = img_stack->next;
	img_free(img_stack);
	img_stack = i2;
	return 0;
}

int img_swap(void)
{
	image *i1 = img_stack;
	if (i1 == NULL) return -1;
	img_stack = i1->next;
	if (img_stack == NULL) return -1;
	i1->next = img_stack->next;
	img_stack->next = i1;
	return 0;
}

int img_tb(void)
{
	pixel p1, p2;
	int i, j;
	if (img_stack == NULL) return -1;
	for (i = 0; i < img_stack->height/2; i++) {
		for (j = 0; j < img_stack->width; j++) {
			p1 = GET_PIXEL(img_stack, j, i);
			p2 = GET_PIXEL(img_stack, j, img_stack->height-i-1);
			PUT_PIXEL(img_stack, j, i, p2);
			PUT_PIXEL(img_stack, j, img_stack->height-i-1, p1);
		}
	}
	return 0;
}

int img_tile(int w, int h)
{
	image *i2 = img_new(w*h);
	int i, j;
	int w1, h1;
	pixel p;

	if (img_stack == NULL) return -1;
	w1 = img_stack->width, h1 = img_stack->height;
	i2->width = w;
	i2->height = h;
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			p = GET_PIXEL(img_stack, j%w1, i%h1);
			PUT_PIXEL(i2, j, i, p);
		}
	}
	i2->next = img_stack->next;
	img_free(img_stack);
	img_stack = i2;
	return 0;
}

image *img_top(void)
{
	return img_stack;
}

/* Format can be specified as FMT:filename.foo or filename.fmt */
int img_write(char *p)
{
	FILE *fpo;
	char b[1024];
	char *q = strrchr(p, '.');
	int (*save)(image *, FILE *) = NULL;
	int i, n;

	q = strchr(p, ':');
	if (q) {
		n = q-p;
		if (n > sizeof b - 1) n = sizeof b - 1;
		strncpy(b, p, n);
		p = q+1;
		b[n] = '\0';
		for (q = b; *q; q++) *q = toupper(*q);
		for (i = 0; img_io[i].name; i++) {
			if (!strcmp(b, img_io[i].name)) {
				save = img_io[i].save;
				break;
			}
		}
		if (save == NULL) {
			img_warn("Unknown format: '%s'\n", b);
			return -1;
		}
	} else if ((q = strrchr(p, '.'))) {
		strncpy(b, q+1, sizeof b);
		b[sizeof b - 1] = '\0';
		for (q = b; *q; q++) *q = toupper(*q);
		for (i = 0; img_io[i].name; i++) {
			if (!strcmp(b, img_io[i].name)) {
				save = img_io[i].save;
				break;
			}
		}
	}
	if (save == NULL) {
		save = write_p3;	/* default to ppm */
	}
	if (!strcmp(p, "-")) fpo = stdout;
	else fpo = fopen(p, "w");
	if (fpo == NULL) {
		img_warn("Can't write image '%s'", p);
		return -1;
	}

	(*save)(img_stack, fpo);
	if (fpo != stdout) fclose(fpo);
	return -1;
}

int img_main(int argc, char **argv)
{
	int i;

	i = 1;
	while (i < argc) {
debug("argv[%d] = '%s'\n", i, argv[i]);
		if (!strcmp(argv[i], "-o") && i+1 < argc) {
			i++;
			img_write(argv[i]);
		} else if (!strcmp(argv[i], "-alias")) {
			img_alias();
		} else if (!strcmp(argv[i], "-bg") && i+1 < argc) {
			i++;
			img_bg(argv[i]);
		} else if (!strcmp(argv[i], "-cd") && i+1 < argc) {
			i++;
			img_cd(argv[i]);
		} else if (!strcmp(argv[i], "-crop")) {
			img_crop();
		} else if (!strcmp(argv[i], "-cut") && i+4 < argc) {
			img_cut(atoi(argv[i+1]), atoi(argv[i+2]),
				atoi(argv[i+3]), atoi(argv[i+4]));
			i += 4;
		} else if (!strcmp(argv[i], "-depth") && i+1 < argc) {
			i++;
			img_depth(atoi(argv[i]));
		} else if (!strcmp(argv[i], "-despeckle") && i+2 < argc) {
			img_despeckle(atoi(argv[i+1]), atoi(argv[i+2]));
			i += 2;
		} else if (!strcmp(argv[i], "-drop")) {
			img_drop();
		} else if (!strcmp(argv[i], "-dup")) {
			img_dup();
		} else if (!strcmp(argv[i], "-enlarge") && i+1 < argc) {
			i++;
			img_enlarge(atoi(argv[i]));
		} else if (!strcmp(argv[i], "-fg") && i+1 < argc) {
			i++;
			img_fg(argv[i]);
		} else if (!strcmp(argv[i], "-fit") && i+2 < argc) {
			img_fit(atoi(argv[i+1]), atoi(argv[i+2]));
			i += 2;
		} else if (!strcmp(argv[i], "-gamma") && i+3 < argc) {
			img_gamma(strtod(argv[i+1], NULL),
				strtod(argv[i+2], NULL),
				strtod(argv[i+3], NULL));
			i += 3;
		} else if (!strcmp(argv[i], "-i") && i+1 < argc) {
			i++;
			img_read(argv[i]);
		} else if (!strcmp(argv[i], "-invert")) {
			img_invert();
		} else if (!strcmp(argv[i], "-lr")) {
			img_lr();
		} else if (!strcmp(argv[i], "-makeicons") && i+3 < argc) {
			img_makeicons( strtol(argv[i+1], NULL, 10),
					strtol(argv[i+2], NULL, 10),
					argv[i+3]);
			i += 3;
		} else if (!strcmp(argv[i], "-margin") && i+1 < argc) {
			i++;
			img_margin(argv[i]);
		} else if (!strcmp(argv[i], "-noop")) {
			img_noop(argv[i]);
		} else if (!strcmp(argv[i], "-pixels") && i+1 < argc) {
			i++;
			img_pixels(atoi(argv[i]));
		} else if (!strcmp(argv[i], "-r90")) {
			img_r90();
		} else if (!strcmp(argv[i], "-r180")) {
			img_r180();
		} else if (!strcmp(argv[i], "-r270")) {
			img_r270();
		} else if (!strcmp(argv[i], "-rotate") && i+1 < argc) {
			i++;
			img_rotate(atoi(argv[i]));
		} else if (!strcmp(argv[i], "-scale") && i+2 < argc) {
			img_scale(strtod(argv[i+1], NULL),
				strtod(argv[i+2], NULL));
			i += 2;
		} else if (!strcmp(argv[i], "-scroll") && i+2 < argc) {
			img_scroll(atoi(argv[i+1]), atoi(argv[i+2]));
			i += 2;
		} else if (!strcmp(argv[i], "-sh") && i+1 < argc) {
			i++;
			img_sh(argv[i]);
		} else if (!strcmp(argv[i], "-shear") && i+1 < argc) {
			i++;
			img_shear(atoi(argv[i]));
		} else if (!strcmp(argv[i], "-size") && i+2 < argc) {
			img_size(atoi(argv[i+1]), atoi(argv[i+2]));
			i += 2;
		} else if (!strcmp(argv[i], "-smooth") && i+2 < argc) {
			img_smooth(atoi(argv[i+1]), atoi(argv[i+2]));
			i += 2;
		} else if (!strcmp(argv[i], "-swap")) {
			img_swap();
		} else if (!strcmp(argv[i], "-tb")) {
			img_tb();
		} else if (!strcmp(argv[i], "-tile")) {
			img_tile(atoi(argv[i+1]), atoi(argv[i+2]));
			i += 2;
		} else {
			img_read(argv[i]);
		}
		i++;
	}
	return 0;
}
