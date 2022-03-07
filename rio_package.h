
/* All the communication between the server and the client is supposed to be reading and writing a string line. 
   The server uses rio package which is a buffered version of UNIX IO. It only uses rio_readlineb() 
   and rio_writen(). */

#include "constant.h"

typedef struct {
    int rio_fd; 
    int rio_cnt; 
    char *rio_bufptr; 
    char rio_buf[BUF_SIZE]; 
} rio_t;

void rio_readinitb(rio_t *rp, int fd);
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n);
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);
ssize_t rio_writen(int fd, char *usrbuf, size_t n);
