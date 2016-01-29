/*
 * Copyright (c) 2007, 2009 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * created on 2,Feb,2012 srinivasa attili
 */

#include <string.h>
#include <stdio.h>

#include "mfs_config.h"
#include "webserver.h"
#include "board_test_app.h"

void
extract_file_name(char *filename, char *req, int rlen, int maxlen)
{
    char *fstart, *fend;

    /* first locate the file name in the request */
    /* requests are of the form GET /path/to/filename HTTP... */

    req += strlen("GET ");

    if (*req == '/')
        req++;

    fstart = req;               /* start marker */

    while (*req != ' ')         /* file name finally ends in a space */
        req++;

    fend = req - 1;             /* end marker */

    if (fend < fstart) {
        strcpy(filename, "index.html");
        return;
    }

    /* make sure filename is of reasonable size */
    if (fend - fstart > maxlen) {
        *fend = 0;
        strcpy(filename, "404.html");
        xil_printf
            ("Request filename is too long, length = %d, file = %s (truncated), max = %d\n\r",
             (unsigned)(fend - fstart), fstart, maxlen);
        return;
    }

    /* copy over the filename */
    strncpy(filename, fstart, fend - fstart + 1);
    filename[fend - fstart + 1] = 0;

    /* if last character is a '/', append index.html */
    if (*fend == '/')
        strcat(filename, "index.html");
}

char *
get_file_extension(char *fname)
{
    char *fext = fname + strlen(fname) - 1;

    while (fext > fname) {
        if (*fext == '.')
            return fext + 1;
        fext--;
    }

    return NULL;
}

int
generate_http_header(char *buf, char *fext, int fsize)
{
    char lbuf[40];

    strcpy(buf, "HTTP/1.0 200 OK\r\nContent-Type: ");

    if (fext == NULL)
        strcat(buf, "text/html");       /* for unknown types */
    else if (!strncmp(fext, "htm", 3))
        strcat(buf, "text/html");       /* html */
    else if (!strncmp(fext, "jpg", 3))
        strcat(buf, "image/jpeg");
    else if (!strncmp(fext, "gif", 3))
        strcat(buf, "image/gif");
    else if (!strncmp(fext, "js", 2))
        strcat(buf, "text/javascript");
    else if (!strncmp(fext, "pdf", 3))
        strcat(buf, "application/pdf");
    else if (!strncmp(fext, "css", 3))
        strcat(buf, "text/css");
    else if (!strncmp(fext, "svg", 3))
        strcat(buf, "image/svg+xml");
    else
        strcat(buf, "text/plain");      /* for unknown types */
    strcat(buf, "\r\n");

    sprintf(lbuf, "Content-length: %d", fsize);
    strcat(buf, lbuf);
    strcat(buf, "\r\n");

    strcat(buf, "Connection: close\r\n");
    strcat(buf, "\r\n");

    return strlen(buf);
}
