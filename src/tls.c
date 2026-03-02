#include "bannerhunter.h"
#include <string.h>
#include <stdio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>

static int g_ssl_init = 0;

static void bh_ssl_init_once(void) {
    if (g_ssl_init) return;
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    g_ssl_init = 1;
}

SSL *bh_tls_connect(int fd, const char *sni, int timeout_sec, char *err, size_t err_sz, char *subj, size_t subj_sz, char *iss, size_t iss_sz, int show_cert) {
    if (err && err_sz) err[0] = '\0';
    if (subj && subj_sz) subj[0] = '\0';
    if (iss && iss_sz) iss[0] = '\0';

    bh_ssl_init_once();
    (void)timeout_sec;

    const SSL_METHOD *method = TLS_client_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (!ctx) {
        if (err) snprintf(err, err_sz, "SSL_CTX_new failed");
        return NULL;
    }

    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);

    SSL *ssl = SSL_new(ctx);
    if (!ssl) {
        SSL_CTX_free(ctx);
        if (err) snprintf(err, err_sz, "SSL_new failed");
        return NULL;
    }

    // SSL owns ctx? No. Keep ctx attached to ssl via ex_data? simplest: use SSL_set_SSL_CTX? We'll keep ctx in SSL object by setting and freeing via SSL_free -> doesn't free ctx.
    // We'll store ctx in SSL ex_data? For simplicity: set as app data and free later.
    SSL_set_app_data(ssl, ctx);

    SSL_set_fd(ssl, fd);
    if (sni && *sni) SSL_set_tlsext_host_name(ssl, sni);

    // Handshake (socket timeouts already applied)
    int r = SSL_connect(ssl);
    if (r != 1) {
        unsigned long e = ERR_get_error();
        if (err) {
            if (e) snprintf(err, err_sz, "TLS handshake failed: %s", ERR_error_string(e, NULL));
            else snprintf(err, err_sz, "TLS handshake failed");
        }
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        return NULL;
    }

    if (show_cert) {
        X509 *cert = SSL_get_peer_certificate(ssl);
        if (cert) {
            X509_NAME_oneline(X509_get_subject_name(cert), subj, (int)subj_sz);
            X509_NAME_oneline(X509_get_issuer_name(cert), iss, (int)iss_sz);
            X509_free(cert);
        }
    }

    return ssl;
}

void bh_tls_close(SSL *ssl) {
    if (!ssl) return;
    SSL_CTX *ctx = (SSL_CTX*)SSL_get_app_data(ssl);
    SSL_shutdown(ssl);
    SSL_free(ssl);
    if (ctx) SSL_CTX_free(ctx);
}
