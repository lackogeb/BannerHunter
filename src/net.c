#include "bannerhunter.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>

static int set_nonblocking(int fd, int nb) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    if (nb) flags |= O_NONBLOCK;
    else flags &= ~O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags);
}

static void set_timeouts(int fd, int timeout_sec) {
    struct timeval tv;
    tv.tv_sec = timeout_sec;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));
}

int bh_tcp_connect(const char *host, int port, int timeout_sec, char *err, size_t err_sz) {
    if (err && err_sz) err[0] = '\0';

    char portstr[16];
    snprintf(portstr, sizeof(portstr), "%d", port);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;

    struct addrinfo *res = NULL;
    int rc = getaddrinfo(host, portstr, &hints, &res);
    if (rc != 0) {
        if (err) snprintf(err, err_sz, "getaddrinfo: %s", gai_strerror(rc));
        return -1;
    }

    int fd = -1;
    for (struct addrinfo *ai = res; ai; ai = ai->ai_next) {
        fd = (int)socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (fd < 0) continue;

        set_timeouts(fd, timeout_sec);
        set_nonblocking(fd, 1);

        int c = connect(fd, ai->ai_addr, (socklen_t)ai->ai_addrlen);
        if (c == 0) {
            set_nonblocking(fd, 0);
            freeaddrinfo(res);
            return fd;
        }

        if (errno != EINPROGRESS) {
            close(fd);
            fd = -1;
            continue;
        }

        fd_set wfds;
        FD_ZERO(&wfds);
        FD_SET(fd, &wfds);

        struct timeval tv;
        tv.tv_sec = timeout_sec;
        tv.tv_usec = 0;

        int s = select(fd + 1, NULL, &wfds, NULL, &tv);
        if (s <= 0) {
            if (err) snprintf(err, err_sz, "connect timeout");
            close(fd);
            fd = -1;
            continue;
        }

        int so_error = 0;
        socklen_t slen = sizeof(so_error);
        getsockopt(fd, SOL_SOCKET, SO_ERROR, &so_error, &slen);
        if (so_error != 0) {
            if (err) snprintf(err, err_sz, "connect: %s", strerror(so_error));
            close(fd);
            fd = -1;
            continue;
        }

        set_nonblocking(fd, 0);
        freeaddrinfo(res);
        return fd;
    }

    freeaddrinfo(res);
    if (err && err_sz && err[0] == '\0') snprintf(err, err_sz, "unable to connect");
    return -1;
}
