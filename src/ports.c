#include "bannerhunter.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static int clamp_int(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static int add_port(int p, int *out_ports, int *count, int max_ports) {
    if (p < 1 || p > 65535) return 0;
    // de-dup
    for (int i = 0; i < *count; i++) {
        if (out_ports[i] == p) return 1;
    }
    if (*count >= max_ports) return 0;
    out_ports[(*count)++] = p;
    return 1;
}

/*
 * Parse port spec like:
 *   "80,443,8080"
 *   "1-1024"
 *   "22,80,443,8000-8100"
 *
 * Safety: max ports is enforced by caller. Returns count or -1 on error.
 */
int bh_parse_ports(const char *spec, int *out_ports, int max_ports) {
    if (!spec || !*spec) return -1;

    int count = 0;
    const char *p = spec;

    while (*p) {
        while (*p && isspace((unsigned char)*p)) p++;

        // read start number
        char *end = NULL;
        long a = strtol(p, &end, 10);
        if (end == p) return -1;
        p = end;

        while (*p && isspace((unsigned char)*p)) p++;

        if (*p == '-') {
            // range
            p++;
            while (*p && isspace((unsigned char)*p)) p++;
            long b = strtol(p, &end, 10);
            if (end == p) return -1;
            p = end;

            a = clamp_int((int)a, 1, 65535);
            b = clamp_int((int)b, 1, 65535);
            if (b < a) {
                long tmp = a; a = b; b = tmp;
            }

            for (long x = a; x <= b; x++) {
                if (!add_port((int)x, out_ports, &count, max_ports)) return -1;
            }
        } else {
            if (!add_port((int)a, out_ports, &count, max_ports)) return -1;
        }

        while (*p && isspace((unsigned char)*p)) p++;

        if (*p == ',') {
            p++;
            continue;
        } else if (*p == '\0') {
            break;
        } else {
            return -1;
        }
    }

    return count;
}
