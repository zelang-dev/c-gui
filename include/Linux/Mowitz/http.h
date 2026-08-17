/* libstocks - Library to get current stock quotes from Yahoo Finance
 *
 * Copyright (C) 2000 Eric Laeuffer
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __HTTP_H__
#define __HTTP_H__

#ifndef __HTTP_C__
#define PUBEXT_HTTP extern
#else
#define PUBEXT_HTTP
#endif

/* <scheme>://<net_loc>/<path>;<params>?<query>#<fragment> */

typedef struct object_box {
	int type;
	void *data;
	int min_w, max_w;
	int x, y, w, h;
	struct object_box *parent;
	struct object_box *child;	/* first child */
	struct object_box *last;	/* last child */
	struct object_box *next;
} object_box;

typedef struct url_components {
	char *scheme;
	char *net_loc;
	char *path;
	char *params;
	char *query;
	char *fragment;
} url_components;

typedef struct url_info {
	char *url;	/* normalized url */
	char *header;
	char *local;	/* cached file */
	int size;	/* excluding header */
	int ref;	/* for reference counting */
	struct url_info *next;
} url_info;

PUBEXT_HTTP void free_url(url_info *);	/* io.c */
PUBEXT_HTTP url_info *load_url(char *);	/* io.c */
PUBEXT_HTTP url_info *load_http(char *);	/* http.c */
PUBEXT_HTTP url_info *load_file(char *);	/* file.c */
PUBEXT_HTTP url_info *load_about(char *);	/* file.c */
PUBEXT_HTTP url_info *load_ftp(char *);	/* to be implemented */

#endif /* __HTTP_H__ */
