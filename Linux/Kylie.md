# It's small, fast and cute: it's Kylie

*From:* <https://siag.nu/kylie/>

## Parsing

The HTML parser in `MwHtmlParser.c` creates a linked list of all the
nodes: wordparts, whitespace and special tags.

Newline immediately before end tag is ignored. Newline immediately after
start tag is ignored.

Composite objects (currently limited to TABLE, ROW and CELL, but
frames belong there too) branch the list so it actually becomes a tree.

## Displaying

When the widget gets the list back, it lays out the boxes by assigning
width and height and placing them in a line from left to right. If
there is too little space left for another box, start over on a new
line.

In a second pass the objects are drawn which are located in the
currently visible part of the window.

Two callbacks in the widget, begin_a and
end_a, handle <A> and </A> tags. The coordinates are put in the
`a_href` or `a_name` tables depending on their `HREF` or `NAME` values.

Composite objects are displayed by recursively drawing the list
of children.

## Navigation

There are two linked lists for backwards and forwards navigation.
Each time we visit a link, the current url is added to the backwards
lists and the forwards list is cleared, *except* when we visit the
link by clicking the back button. In that case the current url is
added to the forwards list instead.

### Type 1 fonts

If `Mowitz` was compiled with t1lib support, Kylie can take advantage
of it out of the box. The standard fonts.txt file is used for font
management. Antialias is not supported.

## IO

The `IO` is split into different modules with narrow interfaces. The
modules are:

`io.c:` Register protocol drivers, normalize urls, cache documents.
 External linkage: load_url.

`http.c:` Fetch documents over http. The only symbol with external
 linkage is load_http, which should *only* be called from io.c.

`file.c:` Fetch local file or predefined "documents".
 External linkage: load_file and load_about.

FTP is currently not implemented.

None of this belongs in the widget code. The widget should only
have the bare minimum required to render stuff in a window.
The widget will probably eventually move over to `Mowitz`.
It should suffice to have url resource that is a string to give
to a loader function.
This function can also be set through a resource and defaults to
a function which loads the file from the local file system using
the url as filename.
Everything else belongs in the application.

## Images

The file `image.c` contains a small, stack-oriented library of functions
which can perform a variety of operations on images. The only thing
we're using it for at the moment is to load images, usually with
the help of external applications such as the `netpbm` kit.

An advantage to this approach is that we can display many types of
images. In the future we will also be able to manipulate images in
more ways than we are using now.

The disadvantage is that it is relatively slow. This is mainly due
to the fact that we have to convert between the library's internal
format, image, and the format X understands, `XImage`. It should be
possible to help the situation somewhat by caching the `XImage`; the
current cache implementation only caches the image.

## Cache

We want all "difficult" data structures to be cached, including:

- Images. Loading an image is CPU-intensive and often requires
   launching an external application. The key is the normalized URL,
   the value is an image structure.
   See image.c, functions img_load and img_free.

- URLs. What we want to cache isn't just the URL, but the downloaded
   data, the header and other relevant information. The data is
   stored in a file rather than memory. The key is the normalized URL,
   the value is a structure with cached document info including
   local file name. The files are stored in
   $HOME/.kylie/cache/server/port/path/file
   See io.c, functions load_url and free_url.

- Object box lists. The key is the normalized URL, the value is
   the object box list.
   See MwHtmlParser.c, functions MwHtmlParse and MwHtmlFree.

From the widget's point of view the cache has two levels, with a lower
level which fetches documents and stores them in local files and an
upper level which caches images and object box lists. The widget only
calls the upper level, i.e. MwHtmlParse and img_load.

load_url lives in io.c. It is called by MwHtmlParse and img_load.

## Displaying HTML, images and plain text

The widget only understands how to display lists of object boxes,
so that is what `MwHtmlParse` produces. The parser creates different
box lists depending on file type.

In the plaintext case the text is split by newlines into a list
of wordpart and newline boxes. Only one text format is used,
and there is no reason to separate white space from other text.

In the image case the list consists of a single image box followed
by a single newline box. The newline is to make sure that the
widget gets the geometry right; that has been a problem before.

So now the parser must be able to figure out the format of the
data it is parsing. The parser can then use the header to
figure out the format.

There is also unknown data which the parser doesn't know how to
parse and the widget doesn't know how to render. The best action
to take is to allow the user to save the data, i.e. present a
file selection box.

## Widget internals

The total height of the document is in the resource `XtNtotalHeight`.
It is recalculated when the tree is dumped (such as after a url change
or a window resize) and used for the vertical scrollbar.

There is also a resource `XtNtotalWidth` which is currently implemented
as the window width or the largest x that has been used in the layout,
whichever is greater.

## Tables

This document:

A document
<table><tr><td>A<td><table>
<tr><td>B<td>C
<tr><td>D<td>E</table>
<tr><td>F<td>G</table>
The end

is parsed into (approximately) this tree:

<CELL>
 |-<WORDPART,"A">
 |-<SPACE>
 |-<WORDPART,"document">
 |-<TABLE>
 |  |-<ROW>
 |  |  |-<CELL>
 |  |  |  `-<WORDPART,"A">
 |  |  `-<CELL>
 |  |     `-<TABLE>
 |  |        |-<ROW>
 |  |        |  |-<CELL>
 |  |        |  |  `-<WORDPART,"B">
 |  |        |  `-<CELL>
 |  |        |     `-<WORDPART,"C">
 |  |        `-<ROW>
 |  |           |-<CELL>
 |  |           |  `-<WORDPART,"D">
 |  |           `-<CELL>
 |  |              `-<WORDPART,"E">
 |  `-<ROW>
 |     |-<CELL>
 |     |  `-<WORDPART,"F">
 |     `-<CELL>
 |        `-<WORDPART,"G">
 |-<WORDPART,"The">
 |-<SPACE>
 `-<WORDPART,"end">

- a TABLE is a list of ROWs and CAPTIONs. No other objects are possible.
- a ROW is a list of CELLs.
- a CELL is a rectangle which contains simple or composite objects.

Note that the main document is a CELL.

In the parser, we keep a pointer to the "current parent". When we
add an object, it is always added as the last child to the current parent.

We ignore </tr>, </td> and </th> tags. They are not required, so we
need to detect end conditions by other means anyway.

A <tr> tag can only appear as the child of a table. When we find
one, move up the tree until we find a table, skipping unfinished cells
and rows as necessary. Create the new row as the last child of the
table object.

A cell (i.e. <td> and <th> tags) can only appear as children of row.
Same principle as above, move up the tree until we find a row.
Create the new cell as the last child of the row object.

The </table> tag is necessary. When we find it, close all unfinished
cells and rows and make the parent of the table the new parent.

The widget's first action is to traverse the tree and assign minimum
and maximum widths to everything. Simple objects do this
by looking at the contents, composite ones do it by combining its children.

The next step is to assign actual width, height and position. The
standard algorithm is:

- if the sum of maximums is less than the total width, use it
- if the sum of minimums is larger than the total width, use it
- split the space "fairly"

Finally the objects are drawn. Like everything else, this is done
by a recursive function.

## TODO

Image inputs are not implemented at all, and button inputs only in part.
File input is not implemented.
The POST method for forms.
Calculate form control sizes more carefully.

---

HTTP 1.1, RFC2616.
HTML tables, RFC1942.
Method of approximating file names: RFC2045.
Information about file uploads: RFC2388.
FTP

---

Printing:

Create a list of all leaf objects that are not size 0x0. That removes
a lot of the meta-objects. In the list, store absolute positions.

When printing, go through the list and print those that will fit
within the current page. Then do the next page until all objects
have been printed. Simple!

---

Nonblocking networking.

---

Image transparency, or at least masks. Add an alpha channel but only
use the values 0 (fully transparent) and 255 (fully opaque).

---

Maybe the parser can be simplified by looking at more than one
character at a time. For example:

static void flush_space(void)
{
 if (spacen) create a space box
}

static void flush_word(void)
{
 if (wordn) create a word box
}

static void flush(void)
{
 flush_space();
 flush_word();
}

static void emit_space(int c)
{
 flush_word();
 growb(spacen+1);
 buffer[spacen++] = c;
}

static void emit_wordpart(int c)
{
 flush_space();
 growb(wordn+1);
 buffer[wordn++] = c;
}

static void emit_starttag(char *p)
{
 flush();
 whatever
}

static void emit_endtag(char *p)
{
 flush();
 whatever
}

while (*b) {
 switch (state) {
 case START:
  if (*b == '\r') b++;
  else if (!strncmp(b, "<!--", 4)) b +=4, state = COMMENT;
		else if (!strncmp(b, "\n</", 3)) b++;
		else if (!strncmp(b "</", 2)) b += 2, state = ENDTAG;
		else if (*b == '<') b++, state = STARTTAG;
		else if (isspace(*b)) emit_space(*b++);
		else emit_wordpart(*b++);
		break;
	case COMMENT:
		p = strstr(b, "-->");
  if (p) b = p+3, state = START;
  else state = ERROR;
  break;
 case ENDTAG:
  p = strchr(b, '>');
  if (p) {
   *p = '\0';
   emit_endtag(b);
   b = p+1;
   state = START;
  } else {
   state = ERROR;
  }
  break;
 case STARTTAG:
  p = strchr(b, '>');
  if (p) {
   *p = '\0';
   emit_starttag(b);
   b = p+1;
   if (*b == '\r') b++;
   if (*b == '\n') b++;
   state = START;
  } else {
   state = ERROR;
  }
  break;
 case ERROR:
  b += strlen(b);
 }
}
flush();

This has the added benefit that it is easy to add new stuff, if the
need should arise.
