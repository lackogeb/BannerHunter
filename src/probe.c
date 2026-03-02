#include "bannerhunter.h"
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <ctype.h>
#include <openssl/ssl.h>

static int write_all_plain(int fd, const char *buf, size_t n) {
    size_t off = 0;
    while (off < n) {
        ssize_t w = send(fd, buf + off, n - off, 0);
        if (w <= 0) return -1;
        off += (size_t)w;
    }
    return 0;
}

static int write_all_tls(SSL *ssl, const char *buf, size_t n) {
    size_t off = 0;
    while (off < n) {
        int w = SSL_write(ssl, buf + off, (int)(n - off));
        if (w <= 0) return -1;
        off += (size_t)w;
    }
    return 0;
}

static int read_some_plain(int fd, char *out, size_t out_sz) {
    if (out_sz == 0) return 0;
    ssize_t r = recv(fd, out, out_sz - 1, 0);
    if (r <= 0) return 0;
    out[r] = '\0';
    return (int)r;
}

static int read_some_tls(SSL *ssl, char *out, size_t out_sz) {
    if (out_sz == 0) return 0;
    int r = SSL_read(ssl, out, (int)(out_sz - 1));
    if (r <= 0) return 0;
    out[r] = '\0';
    return r;
}

static void sanitize(char *s) {
    // Keep printable chars; replace others with dots; trim very long whitespace runs.
    for (size_t i = 0; s[i]; i++) {
        unsigned char c = (unsigned char)s[i];
        if (c == '\r' || c == '\n' || c == '\t') continue;
        if (c < 32 || c == 127) s[i] = '.';
    }
}

int bh_probe_banner_plain(int fd, int send_newline, int probe_http, const char *host, char *banner, size_t banner_sz) {
    if (banner && banner_sz) banner[0] = '\0';
    if (send_newline) write_all_plain(fd, "\r\n", 2);

    int total = 0;
    total += read_some_plain(fd, banner, banner_sz);

    if (probe_http) {
        const char *tmpl = "HEAD / HTTP/1.1\r\nHost: %s\r\nUser-Agent: BannerHunter\r\nConnection: close\r\n\r\n";
        char req[512];
        snprintf(req, sizeof(req), tmpl, host && *host ? host : "localhost");
        write_all_plain(fd, req, strlen(req));
        if (total == 0) total += read_some_plain(fd, banner, banner_sz);
    }

    if (banner && *banner) sanitize(banner);
    return total;
}

int bh_probe_banner_tls(SSL *ssl, int send_newline, int probe_http, const char *host, char *banner, size_t banner_sz) {
    if (banner && banner_sz) banner[0] = '\0';
    if (send_newline) write_all_tls(ssl, "\r\n", 2);

    int total = 0;
    total += read_some_tls(ssl, banner, banner_sz);

    if (probe_http) {
        const char *tmpl = "HEAD / HTTP/1.1\r\nHost: %s\r\nUser-Agent: BannerHunter\r\nConnection: close\r\n\r\n";
        char req[512];
        snprintf(req, sizeof(req), tmpl, host && *host ? host : "localhost");
        write_all_tls(ssl, req, strlen(req));
        if (total == 0) total += read_some_tls(ssl, banner, banner_sz);
    }

    if (banner && *banner) sanitize(banner);
    return total;
}
