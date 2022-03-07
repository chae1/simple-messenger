#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "rio_package.h"

void rio_readinitb(rio_t *rp, int fd) {
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

/* If there is more than or equals to n bytes in the buffer, rio_read() copies n bytes from the buffer to the 
   user buffer. If there is less than n bytes in the buffer, rio_read() copies what is in the buffer to the 
   user buffer. */

/* If there is more than or equals to n bytes to read from the file, rio_read() copies n bytes from the file to 
   the rio buffer. If there is less than n bytes to read from the file, rio_read() copies what is available to
   read in the buffer to the rio buffer. */

static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n) {
    int cnt;

    while (rp->rio_cnt <= 0) /* Refill if buf is empty */
    {
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));

        if (rp->rio_cnt < 0)
        {
            if (errno != EINTR)
                return -1;
        }
        else if (rp->rio_cnt == 0) /* EOF */
            return 0;
        else   
            rp->rio_bufptr = rp->rio_buf; /* Reset buffer ptr */
    }

    cnt = n;
    if (rp->rio_cnt < n)
        cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}


/* If successful, rio_readlineb() copies a line to the user buffer and returns the size of a line which is read.
   If it encounters EOF before a new line character, rio_readlineb() returns 0.
   If the size of a line is equal to or above the maximum length, rio_readlineb() copies first maxlen - 1 bytes 
   to the user buffer and makes the last byte be 0 for the user buffer to be a string */

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) {
    int n, rc;
    char c, *bufp = usrbuf;

    for (n = 1; n < maxlen; n++)
    {
        if ((rc = rio_read(rp, &c, 1)) == 1)
        {
            *bufp++ = c;
            if (c == '\n')
            {
                n++;
                break;
            }
        }
        else if (rc == 0)
            return 0; /* EOF */
        else {
            printf("readlineb error");
            return -1; /* Error */    
        }
    }
    *bufp = 0;
    return n-1; 
}



ssize_t rio_writen(int fd, char *usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;

    while (nleft > 0)
    {
        if ((nwritten = write(fd, bufp, nleft)) < 0)
        {
            if (errno == EINTR)            
                nwritten = 0;
            else {
                printf("rio_writen error\n");
                return -1;                
            }
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}
