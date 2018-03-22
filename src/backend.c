/*
 *
 * GrizzlyCloud library - simplified VPN alternative for IoT
 * Copyright (C) 2017 - 2018 Filip Pancik
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <gc.h>

struct backend_s {
    const char          *ip;
    const char          *description;
    int                 ping[8];
    int                 nping;
    int                 idx;
    struct backend_s    *next;
};

struct backend_seed_s {
    const char          *ip;
    const char          *description;
};

static void parse_ping(char *buffer, int nbuffer, struct backend_s *bnd)
{
    const char *needle = "time=";
    int offset = 0;
    while(1 == 1) {
        char *found = strstr(buffer + offset, needle);
        if(!found) break;

        char *s, *start, *e;
        for(s = start = found + strlen(needle),
            e = buffer + nbuffer;
            s < e; s++) if(*s == '.' || *s == ' ') break;

        char nptr[64];
        snprintf(nptr, sizeof(nptr), "%.*s", (int)(s - start), start);

        bnd->ping[bnd->nping++] = atoi(nptr);

        offset = found - buffer + strlen(needle);
    }
}

static struct backend_s *backend_ping(struct gc_s *gcs,
                                      struct backend_seed_s *seed,
                                      int amount)
{
    int i;
    struct backend_s *gc, *head = NULL;

    for(i = 0; i < amount; i++) {
        char pcmd[128];

#ifdef __linux__
        snprintf(pcmd, sizeof(pcmd), "ping -c 3 %s", seed[i].ip);
#else
        snprintf(pcmd, sizeof(pcmd), "ping %s -n 3", seed[i].ip);
#endif

        hm_log(LOG_TRACE, &gcs->log, "Running ping command: [%s]", pcmd);

        FILE *t = popen(pcmd, "r");

        if(!t) {
            return head;
        }

        char buffer[2048];
        memset(buffer, 0, sizeof(buffer));
        fread(buffer, sizeof(char), sizeof(buffer), t);

        pclose(t);

        gc = hm_palloc(gcs->pool, sizeof(*gc));
        if(!gc) return NULL;

        memset(gc, 0, sizeof(*gc));
        gc->ip          = seed[i].ip;
        gc->description = seed[i].description;
        gc->idx         = i;

        parse_ping(buffer, sizeof(buffer), gc);

        gc->next = head;
        head = gc;
    }

    return head;
}

static void backend_dump(struct gc_s *gc, struct backend_s *bnd)
{
    int i;
    struct backend_s *host;

    for(host = bnd; host != NULL; host = host->next) {
        hm_log(LOG_TRACE, &gc->log, "IP: [%s]", host->ip);
        for(i = 0; i < host->nping; i++) {
            hm_log(LOG_TRACE, &gc->log, "Ping: [%d]", host->ping[i]);
        }
    }
}

static int backend_choose(struct backend_s *bnd)
{
    int i;
    struct backend_s *host;
    int lowest = 999;
    int lowest_idx = -1;

    for(host = bnd; host != NULL; host = host->next) {
        for(i = 0; i < host->nping; i++) {
            if(host->ping[i] < lowest) {
                lowest      = host->ping[i];
                lowest_idx  = host->idx;
            }
        }
    }

    return lowest_idx;
}

static void backend_free(struct hm_pool_s *pool, struct backend_s *bnd)
{
    struct backend_s *host, *del;

    for(host = bnd; host != NULL; ) {
        del = host;
        host = host->next;
        hm_pfree(pool, del);
    }
}

int gc_backend_init(struct gc_s *gc, snb *chosen)
{
    struct backend_s *bnd;
    struct backend_seed_s seeds[] = {
        { "93.185.107.138",  "cz01" },
        { "185.101.98.180",  "us01" },
        { "176.126.245.114", "uk01" }
    };
    /*
    struct backend_seed_s seeds[] = {
        { "localhost",  "local" },
    };
    */
    bnd = backend_ping(gc, seeds, COUNT(seeds));

    if(!bnd) {
        hm_log(LOG_TRACE, &gc->log, "No backend specified");
        return GC_ERROR;
    }

    backend_dump(gc, bnd);

    int sel_idx = backend_choose(bnd);
    if(sel_idx != -1) {
        hm_log(LOG_TRACE, &gc->log, "Selected backend: [%s %s]", seeds[sel_idx].ip,
                                                                 seeds[sel_idx].description);
        sn_initz(ip, (char *)seeds[sel_idx].ip);
        snb_cpy_d(chosen, ip);
    } else {
        hm_log(LOG_TRACE, &gc->log, "No backend selected");
        return GC_ERROR;
    }

    backend_free(gc->pool, bnd);

    return GC_OK;
}
