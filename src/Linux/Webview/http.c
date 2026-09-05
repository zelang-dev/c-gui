#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#ifndef __USE_MISC
#	define __USE_MISC
#endif
#include <netdb.h>
#include <unistd.h>

#include <features.h>
#include <Linux/Mowitz/MwUtils.h>
#include <Linux/Mowitz/http.h>

#define BUF_SIZE 1024 * 16
#define HEADER_MAXBUF 512 * 4
#define TIMEOUT		30	/* default timeout */

static int timeout = TIMEOUT;

/* pointer to proxy server name or NULL */
static char *http_proxy_server = NULL;
/* proxy server port number or 0 */
static int http_proxy_port = 0;

static int split_url(char *url, char *proto, char *host, int *port, char *uri) {
	char b[1024], *p;
	int n;

	proto[0] = b[0] = uri[0] = '\0';
	n = sscanf(url, "%[^:]://%s", proto, b);

	p = strchr(b, '/');
	if (p) {
		strcpy(uri, p);
		*p = '\0';
	} else {
		strcpy(uri, "/");
	}
	strcpy(host, b);

	n = sscanf(b, "%[^:]:%d", host, port);
	if (n != 2)
		*port = 443;

	return 0;
}


/******************************************************************************/
/* Set the proxy server to use                                                */
/******************************************************************************/
static int set_proxy(void) {
	char *ptr, *proxy;
	char c;

	webview_debug("*set_proxy\n");
	proxy = getenv("http_proxy");
	if (proxy == NULL) return 0;

	/* Parse the proxy URL - It must start with http://  */
	if (MwStrncasecmp("http://", proxy, 7))
		return -1;

	proxy += 7;

	/* find ":" in the proxy url */
	ptr = proxy;
	for (c = *ptr; (c && c != ':');)
		c = *ptr++;

	/* ptr points just after the ":" or at the end of proxy if : not founded */
	*(ptr - 1) = 0;		/* clear the ":"  */

	http_proxy_server = MwStrdup(proxy);
	webview_debug("http_proxy_server: %s\n", http_proxy_server);

	/* get the port number of the url */
	if (sscanf(ptr, "%d", &http_proxy_port) != 1)
		return -1;

	webview_debug("http_proxy_port: %d\n", http_proxy_port);
	return 0;
}

static void alarm_handler(int dummy) {
	webview_debug("alarm_handler()\n");
}

/*****************************************************************************/
/* Gets the file from http://http_server/http_file                           */
/* It allocates memory for the file and defines *pdata (the pointer of datas)*/
/*****************************************************************************/
url_info *load_http(char *url) {
	FILE *fp;
	char header[HEADER_MAXBUF];	/* request header */
	int  hlg;		/* header length */
	char buf[BUF_SIZE + 1];	/* tempory buffer from socket read */
	int r;			/* number of bytes read by read function */
	char *data = NULL;	/* http server response */
	int data_lgr;		/* http server response length */
	char *temp;		/* pointer used to split header and csv */
	int error_code;		/* error code returned by http server */

	int po;
	char pr[1024];
	char http_file[1024], http_server[1024];
	char *p, b[1024], port[64];
	int c;
	char reurl[1024];
	url_info *ui;
	struct tls *tls;
	struct tls_config *tls_config;
	int n;

	if ((tls = tls_client()) == NULL) {
		webview_debug(" tls_client: NOK\n");
		return NULL;
	}

	if ((tls_config = tls_config_new()) == NULL) {
		webview_debug(" tls_config_new: NOK\n");
		tls_free(tls);
		return NULL;
	}

	if (tls_config_set_ciphers(tls_config, "compat") != 0) {
		fprintf(stderr, " Error: %s\n", tls_config_error(tls_config));
		tls_config_free(tls_config);
		tls_free(tls);
		return NULL;
	}

/* Insecure connections automatically allowed in `DEBUG` builds*/
#ifdef USE_DEBUG
	tls_config_insecure_noverifycert(tls_config);
	tls_config_insecure_noverifyname(tls_config);
#endif

	if (tls_configure(tls, tls_config) != 0) {
		fprintf(stderr, " Error: %s\n", tls_config_error(tls_config));
		tls_config_free(tls_config);
		tls_free(tls);
		return NULL;
	}

Redirect:
	split_url(url, pr, http_server, &po, http_file);
	webview_debug("*http_get\n");
	set_proxy();

	/* create socket */
	/* connect to server */
	signal(SIGALRM, alarm_handler);
	alarm(timeout);
	snprintf(port, sizeof(port), "%d", po);
	if (tls_connect(tls, http_server, port) != 0) {
		alarm(0);	/* cancel scheduled alarm */
		fprintf(stderr, " Error: %s\n", tls_error(tls));
		tls_config_free(tls_config);
		tls_free(tls);
		return NULL;
	}
	alarm(0);	/* cancel scheduled alarm */

	/* create header */
	if (http_proxy_server) {
		sprintf(header,
			"GET http://%.128s:%d%.256s HTTP/1.0\r\n"
			"User-Agent: Kylie\r\n\r\n",
			http_server, po, http_file);
	} else {
		sprintf(header,
			"GET %s HTTP/1.0\r\n"
			"Host: %s\r\n"
			"User-Agent: Kylie\r\n\r\n",
			http_file, http_server);
	}

	hlg = strlen(header);

	/* send header */
	if ((int)tls_write(tls, header, hlg) != hlg) {
		fprintf(stderr, " Error: %s\n", tls_error(tls));
		tls_config_free(tls_config);
		tls_free(tls);
		return NULL;
	}

	data_lgr = 0;
	r = 1;
	while (r) {
		/* Clear Buffer */
		memset(buf, 0, BUF_SIZE + 1);

		r = (int)tls_read(tls, buf, BUF_SIZE);
		if (r) {
			if (!data_lgr) {
				data = MwMalloc(r + 1);
				memcpy(data, buf, r);
				data_lgr = r;
				data[r] = 0;
			} else {
				temp = MwMalloc(r + data_lgr + 1);
				memcpy(temp, data, data_lgr);
				memcpy(temp + data_lgr, buf, r);
				temp[r + data_lgr] = 0;
				data_lgr += r;
				MwFree(data);
				data = temp;
			}
		}
	}

	/* close socket */
	tls_close(tls);
	webview_debug("%s\n", data);

	/* At this point, data points to the header followed by
	   \r\n\r\n followed by the data. Split at \r\n\r\n,
	   allocate new chunks of memory and free the original. */

/* Example header:
HTTP/1.1 200 OK
Date: Wed, 13 Feb 2002 22:22:27 GMT
Server: Apache/1.3.22 (Unix)
Last-Modified: Fri, 04 May 2001 00:00:38 GMT
ETag: "190be-5b0-3af1f126"
Accept-Ranges: bytes
Content-Length: 1456
Connection: close
Content-Type: text/html
Content-Language: en
*/

	temp = strstr(data, "\r\n\r\n");
	if (temp == NULL) {	/* bogus */
		tls_config_free(tls_config);
		tls_free(tls);
		return NULL;
	}

	temp += 2;	/* past first \r\n */
	*temp = '\0';
	temp += 2;	/* past second \r\n */

	sscanf(data, "HTTP/1.%*d %03d", &error_code);

	if (error_code != 200) {
	webview_debug(" HTTP error code: %d\n", error_code);

#if 1				/* Ulric was here */
		if (error_code == 301) {
			char *p = strstr(data, "\nLocation:");
			char loc[1024];
			MwFree(data);
			if (p == NULL || p > temp) {
				printf("No Location found\n");
				tls_config_free(tls_config);
				tls_free(tls);
				return NULL;
			} else {
				sscanf(p + 1, "Location: %s", loc);
				printf("Found Location header '%s'\n", loc);
				if (!strcmp(url, loc)) {
					printf("Redirect to self\n");
					tls_config_free(tls_config);
					tls_free(tls);
					return NULL;
				}
				strcpy(reurl, loc);
				url = reurl;
				goto Redirect;
			}
		}
#endif				/* Ulric was here */

		tls_config_free(tls_config);
		tls_free(tls);
		return NULL;
	}

	tls_config_free(tls_config);
	tls_free(tls);
	p = getenv("HOME");
	if (!p) p = "/tmp";
	sprintf(b, "%s/.kylie/cache/%s/%d%s", p, http_server, po, http_file);
	p = strrchr(b, '/');
	if (p[1] == '\0') strcat(p, "index.html");
	p = b;
	while ((p = strchr(p + 1, '/'))) {
		c = *p;
		*p = '\0';
		mkdir(b, 0700);
		*p = c;
	}
	fp = fopen(b, "w");
	if (fp == NULL) return NULL;
	fwrite(temp, 1, data_lgr, fp);
	fclose(fp);
	ui = MwMalloc(sizeof * ui);
	ui->local = MwStrdup(b);
	ui->url = MwStrdup(url);
	ui->header = MwStrdup(data);
	MwFree(data);
	ui->size = data_lgr;
	return ui;
}
