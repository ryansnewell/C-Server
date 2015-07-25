//
//  sendfile.c
//  server
//
//  Created by TheKingDoof on 7/2/15
//

#ifdef __APPLE__
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/uio.h>
#elif __linux__
	#include <sys/sendfile.h>
#else
	#include <stdio.h>
#endif

int sendfd(int out_fd, int in_fd, off_t offset, off_t *len)
{
	#ifdef __APPLE__
		int rc = sendfile(in_fd, out_fd, offset, len, NULL, 0);
		return rc;
	#elif __linux__
		off_t *os = offset;
		ssize_t rc = sendfile(out_fd, in_fd, os, (size_t) *len);
		int code = (int)rc;
		return code;
	#else
		printf("Only linux and apple systems supported");
		return -1;
	#endif
}
