#ifndef __BUFSOCK_H__
#define __BUFSOCK_H__

#include "defs.h"

#ifdef __cplusplus
extern "C" {
#endif 


struct _buf_sock;
typedef struct _buf_sock* buf_sock_t;

typedef struct _buf_lines {
    int		num;
    char**	lines;
} buf_lines_t;

typedef enum _buf_sock_status { BS_OK, BS_INCOMPLETE_READ, BS_EOF, BS_ERR, BS_NO_MEM } buf_sock_status_t;

buf_sock_t	buf_sock_connect(const char* host, short port);
void		buf_sock_close(buf_sock_t sock);

buf_sock_status_t buf_sock_read_lines(buf_sock_t sock, buf_lines_t* lines);
buf_sock_status_t buf_sock_write(buf_sock_t sock, const char* text);
buf_sock_status_t buf_sock_write_line(buf_sock_t sock, const char* line);

SOCKET buf_sock_get_raw_socket(buf_sock_t sock);

void buf_sock_free_lines(buf_lines_t* lines);

#ifdef __cplusplus
}
#endif 

#endif /* __BUFSOCK_H__ */
