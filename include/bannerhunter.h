#ifndef BANNERHUNTER_H
#define BANNERHUNTER_H

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BH_MAX_PORTS 256
#define BH_BANNER_MAX 4096
#define BH_HOST_MAX 256

typedef enum {
    BH_MODE_AUTO = 0,   // try TLS then plain fallback
    BH_MODE_TLS_ONLY,
    BH_MODE_PLAIN_ONLY
} bh_mode_t;

typedef enum {
    BH_OUT_TEXT = 0,
    BH_OUT_JSONL
} bh_output_t;

typedef struct {
    char host[BH_HOST_MAX];
    char sni[BH_HOST_MAX];
    int ports[BH_MAX_PORTS];
    int port_count;

    int timeout_sec;          // connect/read/write timeout
    int delay_ms;             // delay between ports
    int probe_http;           // send HTTP HEAD request after connect (plain or TLS)
    int send_newline;         // send \r\n right after connect (helps some services)
    int show_cert;            // TLS: print subject/issuer
    int quiet;

    bh_mode_t mode;
    bh_output_t output;

    FILE *out;                // optional file to append results
} bh_config_t;

typedef struct {
    int port;
    int tcp_ok;
    int tls_ok;
    int used_tls;
    char banner[BH_BANNER_MAX];
    char cert_subject[512];
    char cert_issuer[512];
    char error[256];
} bh_result_t;

// TLS helpers (OpenSSL)
typedef struct ssl_st SSL;
SSL *bh_tls_connect(int fd, const char *sni, int timeout_sec, char *err, size_t err_sz,
                    char *subj, size_t subj_sz, char *iss, size_t iss_sz, int show_cert);
void bh_tls_close(SSL *ssl);

int bh_probe_banner_plain(int fd, int send_newline, int probe_http, const char *host, char *banner, size_t banner_sz);
int bh_probe_banner_tls(SSL *ssl, int send_newline, int probe_http, const char *host, char *banner, size_t banner_sz);

int bh_parse_ports(const char *spec, int *out_ports, int max_ports);
int bh_tcp_connect(const char *host, int port, int timeout_sec, char *err, size_t err_sz);
int bh_scan_port(const bh_config_t *cfg, int port, bh_result_t *res);

#ifdef __cplusplus
}
#endif

#endif
