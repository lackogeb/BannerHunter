#include "bannerhunter.h"
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>

static const int default_ports[] = {21,22,23,25,53,80,110,143,443,465,587,993,995,3306,3389,5432,8080,8443};

static void usage(FILE *f) {
    fprintf(f,
"BannerHunter - safe, single-target banner & service probe (TCP + optional TLS)\n"
"\n"
"USAGE:\n"
"  bannerhunter --i-have-permission -h <host> [options]\n"
"\n"
"REQUIRED:\n"
"  --i-have-permission     Confirms you have explicit authorization to test the target\n"
"  -h, --host <host>       Target hostname or IP\n"
"\n"
"PORT SELECTION:\n"
"  -p, --port <n>          Scan a single port (repeatable)\n"
"  -P, --ports <spec>      Comma list and ranges (e.g. 80,443,8000-8100)\n"
"  --default-ports         Scan a curated safe default list\n"
"\n"
"PROBING:\n"
"  --tls                   TLS only\n"
"  --no-tls                Plain TCP only\n"
"  --probe-http            After connect, attempt HTTP HEAD (helps identify web services)\n"
"  --send-newline          Send CRLF right after connect (helps some services show a banner)\n"
"  --sni <name>            Override TLS SNI (default: host)\n"
"  --show-cert             Print TLS cert subject/issuer when available\n"
"\n"
"PERFORMANCE/SAFETY:\n"
"  -t, --timeout <sec>     Timeout per port (default 5)\n"
"  --delay-ms <ms>         Delay between ports (default 80)\n"
"  --max-ports <n>         Hard limit (default 128, max 256)\n"
"\n"
"OUTPUT:\n"
"  -o, --output <file>     Append results to a file\n"
"  --jsonl                 Emit JSON Lines (one JSON object per port)\n"
"  -q, --quiet             Reduce non-result output\n"
"\n"
"EXAMPLES:\n"
"  bannerhunter --i-have-permission -h example.com --default-ports --probe-http\n"
"  bannerhunter --i-have-permission -h 10.0.0.5 -P 22,80,443 --tls --show-cert\n"
"\n"
"ETHICS:\n"
"  This tool is intended for *authorized* security testing and troubleshooting.\n"
"  Do not scan systems you do not own or have explicit permission to test.\n"
    );
}

static int push_port(int p, bh_config_t *cfg) {
    if (cfg->port_count >= BH_MAX_PORTS) return 0;
    if (p < 1 || p > 65535) return 0;
    // de-dup
    for (int i = 0; i < cfg->port_count; i++) if (cfg->ports[i] == p) return 1;
    cfg->ports[cfg->port_count++] = p;
    return 1;
}

static void emit_text(const bh_config_t *cfg, const bh_result_t *r) {
    if (!r->tcp_ok) {
        if (!cfg->quiet) fprintf(stdout, "%s:%d  CLOSED (%s)\n", cfg->host, r->port, r->error[0] ? r->error : "error");
        if (cfg->out) fprintf(cfg->out, "%s:%d  CLOSED (%s)\n", cfg->host, r->port, r->error[0] ? r->error : "error");
        return;
    }

    const char *mode = r->used_tls ? "TLS" : "PLAIN";
    if (r->banner[0]) {
        fprintf(stdout, "%s:%d  OPEN  %s  %s\n", cfg->host, r->port, mode, r->banner);
        if (cfg->out) fprintf(cfg->out, "%s:%d  OPEN  %s  %s\n", cfg->host, r->port, mode, r->banner);
    } else {
        fprintf(stdout, "%s:%d  OPEN  %s\n", cfg->host, r->port, mode);
        if (cfg->out) fprintf(cfg->out, "%s:%d  OPEN  %s\n", cfg->host, r->port, mode);
    }

    if (cfg->show_cert && r->used_tls && r->cert_subject[0]) {
        fprintf(stdout, "  cert_subject: %s\n", r->cert_subject);
        fprintf(stdout, "  cert_issuer : %s\n", r->cert_issuer);
        if (cfg->out) {
            fprintf(cfg->out, "  cert_subject: %s\n", r->cert_subject);
            fprintf(cfg->out, "  cert_issuer : %s\n", r->cert_issuer);
        }
    }
}

static void json_escape(const char *in, char *out, size_t out_sz) {
    size_t o = 0;
    for (size_t i = 0; in && in[i] && o + 2 < out_sz; i++) {
        unsigned char c = (unsigned char)in[i];
        if (c == '\\' || c == '\"') { out[o++]='\\'; out[o++]=(char)c; }
        else if (c == '\n') { out[o++]='\\'; out[o++]='n'; }
        else if (c == '\r') { out[o++]='\\'; out[o++]='r'; }
        else if (c == '\t') { out[o++]='\\'; out[o++]='t'; }
        else if (c < 32) { /* skip */ }
        else out[o++]=(char)c;
    }
    out[o]='\0';
}

static void emit_jsonl(const bh_config_t *cfg, const bh_result_t *r) {
    char b[BH_BANNER_MAX*2];
    char es[1024], ei[1024], ee[512];
    json_escape(r->banner, b, sizeof(b));
    json_escape(r->cert_subject, es, sizeof(es));
    json_escape(r->cert_issuer, ei, sizeof(ei));
    json_escape(r->error, ee, sizeof(ee));

    fprintf(stdout,
        "{\"host\":\"%s\",\"port\":%d,\"tcp_ok\":%s,\"used_tls\":%s,\"tls_ok\":%s,"
        "\"banner\":\"%s\",\"cert_subject\":\"%s\",\"cert_issuer\":\"%s\",\"error\":\"%s\"}\n",
        cfg->host, r->port,
        r->tcp_ok ? "true":"false",
        r->used_tls ? "true":"false",
        r->tls_ok ? "true":"false",
        b, es, ei, ee
    );

    if (cfg->out) {
        fprintf(cfg->out,
            "{\"host\":\"%s\",\"port\":%d,\"tcp_ok\":%s,\"used_tls\":%s,\"tls_ok\":%s,"
            "\"banner\":\"%s\",\"cert_subject\":\"%s\",\"cert_issuer\":\"%s\",\"error\":\"%s\"}\n",
            cfg->host, r->port,
            r->tcp_ok ? "true":"false",
            r->used_tls ? "true":"false",
            r->tls_ok ? "true":"false",
            b, es, ei, ee
        );
    }
}

int main(int argc, char **argv) {
    bh_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.timeout_sec = 5;
    cfg.delay_ms = 80;
    cfg.mode = BH_MODE_AUTO;
    cfg.output = BH_OUT_TEXT;

    int have_permission = 0;
    int max_ports = 128;
    int use_default_ports = 0;

    static struct option long_opts[] = {
        {"i-have-permission", no_argument, 0, 1000},
        {"host", required_argument, 0, 'h'},
        {"port", required_argument, 0, 'p'},
        {"ports", required_argument, 0, 'P'},
        {"default-ports", no_argument, 0, 1001},
        {"timeout", required_argument, 0, 't'},
        {"delay-ms", required_argument, 0, 1002},
        {"max-ports", required_argument, 0, 1003},
        {"tls", no_argument, 0, 1004},
        {"no-tls", no_argument, 0, 1005},
        {"probe-http", no_argument, 0, 1006},
        {"send-newline", no_argument, 0, 1007},
        {"sni", required_argument, 0, 1008},
        {"show-cert", no_argument, 0, 1009},
        {"output", required_argument, 0, 'o'},
        {"jsonl", no_argument, 0, 1010},
        {"quiet", no_argument, 0, 'q'},
        {"help", no_argument, 0, 1011},
        {0,0,0,0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "h:p:P:t:o:q", long_opts, NULL)) != -1) {
        switch (opt) {
            case 1000: have_permission = 1; break;
            case 'h': strncpy(cfg.host, optarg, sizeof(cfg.host)-1); break;
            case 'p': push_port(atoi(optarg), &cfg); break;
            case 'P': {
                int tmp[BH_MAX_PORTS];
                int n = bh_parse_ports(optarg, tmp, BH_MAX_PORTS);
                if (n < 0) { fprintf(stderr, "Invalid port spec: %s\n", optarg); return 2; }
                for (int i = 0; i < n; i++) push_port(tmp[i], &cfg);
                break;
            }
            case 1001: use_default_ports = 1; break;
            case 't': cfg.timeout_sec = atoi(optarg); break;
            case 1002: cfg.delay_ms = atoi(optarg); break;
            case 1003: max_ports = atoi(optarg); if (max_ports < 1) max_ports = 1; if (max_ports > BH_MAX_PORTS) max_ports = BH_MAX_PORTS; break;
            case 1004: cfg.mode = BH_MODE_TLS_ONLY; break;
            case 1005: cfg.mode = BH_MODE_PLAIN_ONLY; break;
            case 1006: cfg.probe_http = 1; break;
            case 1007: cfg.send_newline = 1; break;
            case 1008: strncpy(cfg.sni, optarg, sizeof(cfg.sni)-1); break;
            case 1009: cfg.show_cert = 1; break;
            case 'o': cfg.out = fopen(optarg, "a"); if (!cfg.out) { perror("fopen"); return 2; } break;
            case 1010: cfg.output = BH_OUT_JSONL; break;
            case 'q': cfg.quiet = 1; break;
            case 1011: usage(stdout); return 0;
            default: usage(stderr); return 2;
        }
    }

    if (!have_permission) {
        fprintf(stderr, "Error: missing required flag --i-have-permission\n\n");
        usage(stderr);
        return 2;
    }
    if (cfg.host[0] == '\0') {
        fprintf(stderr, "Error: missing -h/--host\n\n");
        usage(stderr);
        return 2;
    }

    if (use_default_ports) {
        for (size_t i = 0; i < sizeof(default_ports)/sizeof(default_ports[0]); i++) {
            push_port(default_ports[i], &cfg);
        }
    }

    if (cfg.port_count == 0) {
        fprintf(stderr, "Error: no ports selected. Use -p/-P or --default-ports\n\n");
        usage(stderr);
        return 2;
    }

    if (cfg.port_count > max_ports) {
        fprintf(stderr, "Error: selected %d ports but max is %d (adjust with --max-ports)\n", cfg.port_count, max_ports);
        return 2;
    }

    if (!cfg.quiet) {
        fprintf(stdout, "Target: %s  Ports: %d  Mode: %s  Timeout: %ds  Delay: %dms\n",
                cfg.host, cfg.port_count,
                cfg.mode==BH_MODE_TLS_ONLY ? "TLS" : (cfg.mode==BH_MODE_PLAIN_ONLY ? "PLAIN" : "AUTO"),
                cfg.timeout_sec, cfg.delay_ms);
    }

    for (int i = 0; i < cfg.port_count; i++) {
        bh_result_t r;
        bh_scan_port(&cfg, cfg.ports[i], &r);
        if (cfg.output == BH_OUT_JSONL) emit_jsonl(&cfg, &r);
        else emit_text(&cfg, &r);
        fflush(stdout);
        if (cfg.out) fflush(cfg.out);
    }

    if (cfg.out) fclose(cfg.out);
    return 0;
}
