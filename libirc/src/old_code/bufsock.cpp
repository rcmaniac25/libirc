#include <stdlib.h>
#include <assert.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <unistd.h>
#endif 

#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include "bufsock.h"
#include "tcp.h"

static buf_sock_status_t _buf_sock_examine_buffer(buf_sock_t sock, buf_lines_t* lines);

struct _buf_sock {
     SOCKET	s;
     char*	buf;
     int	buf_size;
     int	cur_ptr;
};

buf_sock_t	buf_sock_connect(const char* host, short port)
{
     buf_sock_t	sock;
     SOCKET		s;
     
     s = tcp_connect(host, port);
     if(INVALID_SOCKET == s)
	  return NULL;

#ifdef _WIN32
	// if(ioctlsocket(s, FIONBIO, (u_long*)1) == SOCKET_ERROR) {
	//	 int err = WSAGetLastError();
	//	 return NULL;
	// }
#else
     if(fcntl(s, F_SETFL, O_NONBLOCK) < 0) {
	  perror("fcntl");
	  return NULL;
     }
#endif
     
     sock = (buf_sock_t)malloc(sizeof(struct _buf_sock));
     assert(sock != NULL);
     if(NULL == sock) {
	  tcp_close(s);
	  return NULL;
     }
     sock->s = s;
     sock->buf = (char*)malloc(1024);
     sock->buf_size = 1024;
     sock->cur_ptr = 0;
     
     assert(sock->buf != NULL);
     if(NULL == sock->buf) {
	  tcp_close(s);
	  free(sock->buf);
	  free(sock);
	  return NULL;
     }

     return sock;
}

void buf_sock_close(buf_sock_t sock)
{
     assert(sock != NULL);
     
     tcp_close(sock->s);
     free(sock->buf);
     free(sock);
}

buf_sock_status_t buf_sock_read_lines(buf_sock_t sock, buf_lines_t* lines)
{
     int	max_bytes;
     int	rbytes;

     assert(sock != NULL);
     assert(lines != NULL);

     max_bytes = sock->buf_size - sock->cur_ptr;
     while(max_bytes < 10) {
	  char* new_buf = (char*)realloc(sock->buf, sock->buf_size * 2);
	  if(NULL == new_buf)
	       return BS_NO_MEM;
	  
	  sock->buf = new_buf;
	  sock->buf_size *= 2;
	  max_bytes = sock->buf_size - sock->cur_ptr;
     }

     rbytes = tcp_recv(sock->s, sock->buf + sock->cur_ptr, max_bytes);
     if(rbytes < 0) {
	  if(errno == EAGAIN)
	       return BS_INCOMPLETE_READ;
	  else
	       return BS_ERR;
     } else if(rbytes == 0)
	  return BS_EOF;
     else
	  sock->cur_ptr += rbytes;

     return _buf_sock_examine_buffer(sock, lines);
}

buf_sock_status_t buf_sock_write(buf_sock_t sock, const char* text)
{ 
     size_t text_len;
     int written_total;
     int written;

     assert(sock != NULL);
     assert(text != NULL);

     text_len = strlen(text);
     written_total = 0;
     do {
	  written = tcp_send(sock->s, text + written_total, (int)(text_len - written_total));
	  if(written < 0) {
	       if(errno == EAGAIN) {
		    /* TCP subsystem can't send anymore.. lets sleep a while and let it stabilize */
#ifdef _WIN32
		    Sleep(1);
#else
		    usleep(1000);
#endif
	       } else
		    return BS_ERR;
	  }

	  written_total += written;
     } while(written_total < (int)text_len);

     return BS_OK;
}

buf_sock_status_t buf_sock_write_line(buf_sock_t sock, const char* line)
{
     buf_sock_status_t stat;

     stat = buf_sock_write(sock, line);
     if(stat != BS_OK)
	  return stat;

     return buf_sock_write(sock, "\r\n");
} 

SOCKET buf_sock_get_raw_socket(buf_sock_t sock)
{
     assert(sock != NULL);
     return sock->s;
}


static buf_sock_status_t _buf_sock_examine_buffer(buf_sock_t sock, buf_lines_t* lines)
{
     int	i;
     int	num_lines;
     int	begin_string, end_string;
     int	cur_line;

     for(num_lines = 0, i = 0; i < sock->buf_size; ++i) {
	  if(sock->buf[i] == '\n')
	       ++num_lines;
     }
     
     if(0 == num_lines)
	  return BS_INCOMPLETE_READ;

     lines->num = num_lines;
     lines->lines = (char**)malloc(num_lines * sizeof(char*));

     for(cur_line = 0, begin_string = 0, i = 0; cur_line < num_lines; ++i) {
	  if(sock->buf[i] == '\n') {
	       if(i > 0 && sock->buf[i - 1] == '\r')
		    end_string = i - 1;
	       else
		    end_string = i;
	       sock->buf[end_string] = '\0';

	       lines->lines[cur_line] = strdup(&sock->buf[begin_string]);
	       begin_string = i + 1;
	       cur_line++;
	  }
     }

     // Copy the rest of the data to the beginning of the buffer
     if(begin_string < sock->cur_ptr) {
	  memmove(sock->buf, sock->buf + begin_string, sock->cur_ptr - begin_string);
	  sock->cur_ptr = 0;
     }

     return BS_OK;
}

void buf_sock_free_lines(buf_lines_t* lines)
{
     int i = 0;
     for(i = 0; i < lines->num; ++i)
	  free(lines->lines[i]);
     free(lines->lines);
}
