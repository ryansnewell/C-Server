//
//  sendfile.h
//  server
//
//  Created by TheKingDoof on 7/2/15.
//

#ifndef __server__sendfile__
#define __server__sendfile__

int sendfd(int out_fd, int in_fd, off_t offset, off_t *len);

#endif /* defined(__server__sendfile__) */
