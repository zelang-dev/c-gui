#ifndef IMAGE_H
#define IMAGE_H

#define GET_PIXEL(i,x,y) \
        ((i)->pixels[(y)*(i)->width+(x)])
#define PUT_PIXEL(i,x,y,p) \
        ((i)->pixels[(y)*(i)->width+(x)]=(p))

typedef struct pixel {
        unsigned char r, g, b;
} pixel;

typedef struct image {
        int width, height, npixels;
        pixel *pixels;
        struct image *next;
} image;

void img_free(image *);
image *img_load(char *);
int img_eq_pixel(pixel p, pixel q);
pixel img_average_pixel(int x, int y, int w, int h);
pixel img_median_pixel(int x, int y, int w, int h);
int img_alias(void);
int img_bg(char *p);
int img_cd(char *);
int img_crop(void);
int img_cut(int x, int y, int w, int h);
int img_depth(int n);
int img_despeckle(int w, int h);
int img_drop(void);
int img_dup(void);
int img_enlarge(int n);
int img_fg(char *p);
int img_fit(int w, int h);
int img_gamma(float r, float g, float b);
int img_invert(void);
int img_lr(void);
int img_makeicons(int w, int h, char *tndir);
int img_margin(char *p);
int img_noop(char *p);
int img_pixels(int n);
image *img_pop(void);
int img_push(image *);
int img_r90(void);
int img_r180(void);
int img_r270(void);
int img_read(char *fn);
int img_rotate(int n);
int img_scale(float w, float h);
int img_scroll(int x, int y);
int img_sh(char *cmd);
int img_shear(int a);
int img_size(int w, int h);
int img_smooth(int w, int h);
int img_swap(void);
int img_tb(void);
int img_tile(int w, int h);
image *img_top(void);
int img_write(char *p);
int img_main(int, char **);

#endif	/* IMAGE_H */
