
/* This module implements url parsing according to rfc1808 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <Linux/Mowitz/MwUtils.h>
#include <Linux/Mowitz/http.h>

/* Parse url into components. Returns allocated memory which must be freed */
char *parse_url(char *url, struct url_components *c)
{
	char *p = MwStrdup(url);
	char *q = strchr(p, '#');
	char *s;

	/* 2.4.1. Parsing the Fragment Identifier */
	if (q) {
		*q++ = '\0';
		c->fragment = q;
	} else {
		c->fragment = NULL;
	}
	/* p points to the part before the # */

	/* 2.4.2. Parsing the Scheme */
	for (s = p; *s; s++) {
		if (!isalnum(*s) && *s != '+' && *s != '.' && *s != '-')
			break;
	}
	if (*s == ':') {
		*s++ = '\0';
		c->scheme = p;
	} else {
		s = p;
		c->scheme = NULL;
	}
	/* s points to the part after the : */

	/* 2.4.3. Parsing the Network Location/Login */
	if (s[0] == '/' && s[1] == '/') {
		s += 2;
		c->net_loc = s-1;
		for (q = s; *q && *q != '/'; q++);
		memmove(s-1, s, q-s);
		*(q-1) = '\0';
	} else {
		c->net_loc = NULL;
		q = s;
	}
	/* q points to the part after the // */

	/* 2.4.4. Parsing the Query Information */
	s = strchr(q, '?');
	if (s) {
		*s++ = '\0';
		c->query = s;
	} else {
		c->query = NULL;
	}
	/* q points to the part before the ? */

	/* 2.4.5. Parsing the Parameters */
	s = strchr(q, ';');
	if (s) {
		*s++ = '\0';
		c->params = s;
	} else {
		c->params = NULL;
	}
	/* q point to the part before the ; */

	/* 2.4.6. Parsing the Path */
	c->path = q;

	if (c->scheme == NULL) c->scheme = "";
	if (c->net_loc == NULL) c->net_loc = "";
	if (c->path == NULL) c->path = "";
	if (c->params == NULL) c->params = "";
	if (c->query == NULL) c->query = "";
	if (c->fragment == NULL) c->fragment = "";
	return p;
}

/* Given base and embedded url, return resolved url. Caller must free */
char *x_resolve_url(char *base, char *url)
{
	char *p = NULL, *q = NULL, *r, *s, *t, *r2;
	int n;
	struct url_components b, u;

	if (!base) base = "";
	p = parse_url(base, &b);
	if (!url) url = "";
	q = parse_url(url, &u);

	/* Step 1: If the base URL is the empty string, the embedded URL
	   is interpreted as an absolute URL and we are done */
	if (base[0] == '\0') {
		r = MwStrdup(url);
		goto Done;
	}

	/* Step 2a: If the embedded URL is entirely empty, it inherits
	   the entire base URL and we are done */
	if (url[0] == '\0') {
		r = MwStrdup(base);
		goto Done;
	}

	/* Step 2b: If the embedded URL starts with a scheme name, it is
	   interpreted as an absolute URL and we are done */
	if (u.scheme[0]) {
		r = MwStrdup(url);
		goto Done;
	}

	/* Step 2c: Otherwise, the embedded URL inherits the scheme of
	   the base URL */
	u.scheme = b.scheme;

	/* Step 3: If the embedded URL's <net_loc> is non-empty, we skip to
	   Step 7.  Otherwise, the embedded URL inherits the <net_loc>
	   (if any) of the base URL. */
	if (u.net_loc[0] != '\0') goto Step7;
	u.net_loc = b.net_loc;

	/* Step 4: If the embedded URL path is preceded by a slash "/",
	   the path is not relative and we skip to Step 7. */
	if (u.path[0] == '/') goto Step7;

	/* Step 5: If the embedded URL path is empty (and not preceded by
	   a slash), then the embedded URL inherits the base URL path */
	if (u.path[0] == '\0') {
		u.path = b.path;

		/* Step 5a: if the embedded URL's <params> is non-empty,
		   we skip to step 7; otherwise, it inherits the <params>
		   of the base URL */
		if (u.params[0]) goto Step7;
		u.params = b.params;

		/* Step 5b: if the embedded URL's <query> is non-empty,
		   we skip to step 7; otherwise, it inherits the <query>
		   of the base URL */
		if (u.query[0]) goto Step7;
		u.query = b.query;

		goto Step7;
	}

	/* Step 6: The last segment of the base URL's path (anything
	   following the rightmost slash "/", or the entire path if no
	   slash is present) is removed and the embedded URL's path is
	   appended in its place. The following operations are then
	   applied, in order, to the new path:
	   a) All occurences of "./", where "." is a complete path
	   segment, are removed. */
	s = MwMalloc(strlen(b.path)+strlen(u.path)+1);
	strcpy(s, b.path);
	t = strrchr(s, '/');
	if (t) t[1] = '\0';
	else s[0] = '\0';
	strcat(s, u.path);
	while ((t = strstr(s, "/./"))) {
		memcpy(t, t+2, strlen(t+2)+1);
	}
	/* b) If the path ends with "." as a complete path segment,
	   that "." is removed. */
	t = strrchr(s, '/');
	if (!strcmp(t, "/.")) t[1] = '\0';
	/* c) All occurrences of "<segment>/../", where <segment> is a
	   complete path segment not equal to "..", are removed.
	   Removal of these path segments is performed iteratively,
	   removing the leftmost matching pattern on each iteration,
	   until no matching pattern remains. */
	while ((t = strstr(s, "/../"))) {
		char *r2 = t-1;
		while (r2 >= s && *r2 != '/') r2--;
		if (*r2 != '/') break;
		memmove(r2, t+3, strlen(t+3)+1);
	}
	/* d) If the path ends with "<segment>/..", where <segment> is a
	   complete path segment not equal to "..", that
	   "<segment>/.." is removed. */
	t = strrchr(s, '/');
	if (t) {
		if (!strcmp(t, "/..") && t > s) {
			*t = '\0';
			r2 = strrchr(s, '/');
			if (r2) r2++;
			else r2 = s;
			if (!strcmp(r2, "..")) *t = '/';
			else *r2 = '\0';
		}
	}
	u.path = s;

Step7:
	/* Step 7: The resulting URL components, including any inherited
	   from the base URL, are recombined to give the absolute form of
	   the embedded URL. */
	if (!u.scheme[0]) u.scheme = b.scheme;
	if (!u.net_loc[0]) u.net_loc = b.net_loc;
	if (!u.path[0]) u.path = b.path;
	if (!u.params[0]) u.params = b.params;
	if (!u.query[0]) u.query = b.query;
	if (!u.fragment[0]) u.fragment = b.fragment;
	n = strlen(u.scheme)+strlen(u.net_loc)+strlen(u.path)+
	    strlen(u.params)+strlen(u.query)+strlen(u.fragment);
	r = MwMalloc(n+10);
	sprintf(r, "%s://%s%s%s%s%s%s%s%s", u.scheme, u.net_loc, u.path,
		u.params[0]?";":"", u.params,
		u.query[0]?"?":"", u.query,
		u.fragment[0]?"#":"", u.fragment);

Done:
	if (p) MwFree(p);
	if (q) MwFree(q);
	return r;
}
