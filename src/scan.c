#include "bannerhunter.h"
#include <string.h>
#include <unistd.h>
#include <time.h>

static void msleep(int ms) {
    if (ms <= 0) return;
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

static void set_err(bh_result_t *r, const char *msg) {
    if (!r) return;
    if (msg) {
        strncpy(r->error, msg, sizeof(r->error)-1);
        r->error[sizeof(r->error)-1] = '\0';
    }
}

int bh_scan_port(const bh_config_t *cfg, int port, bh_result_t *res) {
    memset(res, 0, sizeof(*res));
    res->port = port;

    char err[256];
    int fd = bh_tcp_connect(cfg->host, port, cfg->timeout_sec, err, sizeof(err));
    if (fd < 0) {
        set_err(res, err);
        return -1;
    }
    res->tcp_ok = 1;

    // Decide TLS/plain attempts
    int try_tls = (cfg->mode == BH_MODE_TLS_ONLY) || (cfg->mode == BH_MODE_AUTO);
    int try_plain = (cfg->mode == BH_MODE_PLAIN_ONLY) || (cfg->mode == BH_MODE_AUTO);

    if (try_tls) {
        SSL *ssl = bh_tls_connect(fd, (cfg->sni[0] ? cfg->sni : cfg->host),
                                  cfg->timeout_sec, err, sizeof(err),
                                  res->cert_subject, sizeof(res->cert_subject),
                                  res->cert_issuer, sizeof(res->cert_issuer),
                                  cfg->show_cert);
        if (ssl) {
            res->tls_ok = 1;
            res->used_tls = 1;
            bh_probe_banner_tls(ssl, cfg->send_newline, cfg->probe_http, cfg->host, res->banner, sizeof(res->banner));
            bh_tls_close(ssl);
            close(fd);
            msleep(cfg->delay_ms);
            return 0;
        } else if (cfg->mode == BH_MODE_TLS_ONLY) {
            set_err(res, err);
            close(fd);
            msleep(cfg->delay_ms);
            return -1;
        }
        // fall through to plain if auto
    }

    if (try_plain) {
        res->used_tls = 0;
        bh_probe_banner_plain(fd, cfg->send_newline, cfg->probe_http, cfg->host, res->banner, sizeof(res->banner));
        close(fd);
        msleep(cfg->delay_ms);
        return 0;
    }

    close(fd);
    msleep(cfg->delay_ms);
    set_err(res, "no mode selected");
    return -1;
}
