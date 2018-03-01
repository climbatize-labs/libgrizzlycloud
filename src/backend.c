#include <gc.h>

static void parse_ping(char *buffer, int nbuffer, struct gc_backend_s *bnd)
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

static struct gc_backend_s *gc_backend_ping(struct gc_s *gcs,
                                            struct gc_backend_seed_s *seed,
                                            int amount)
{
    int i;
    struct gc_backend_s *gc, *head = NULL;

    for(i = 0; i < amount; i++) {
        char pcmd[128];
        snprintf(pcmd, sizeof(pcmd), "ping -c 3 %s", seed[i].ip);

        hm_log(LOG_TRACE, &gcs->log, "Running ping command: [%s]", pcmd);

        FILE *t = popen(pcmd, "r");

        if(!t) {
            return head;
        }

        char buffer[2048];
        memset(buffer, 0, sizeof(buffer));
        fread(buffer, sizeof(char), sizeof(buffer), t);

        pclose(t);

        gc = malloc(sizeof(*gc));
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

static void gc_backend_dump(struct gc_s *gc, struct gc_backend_s *bnd)
{
    int i;
    struct gc_backend_s *host;

    for(host = bnd; host != NULL; host = host->next) {
        hm_log(LOG_TRACE, &gc->log, "IP: [%s]", host->ip);
        for(i = 0; i < host->nping; i++) {
            hm_log(LOG_TRACE, &gc->log, "Ping: [%d]", host->ping[i]);
        }
    }
}

static int gc_backend_choose(struct gc_backend_s *bnd)
{
    int i;
    struct gc_backend_s *host;
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

static void gc_backend_free(struct gc_backend_s *bnd)
{
    struct gc_backend_s *host, *del;

    for(host = bnd; host != NULL; ) {
        del = host;
        host = host->next;
        free(del);
    }
}

int gc_backend_init(struct gc_s *gc, snb *chosen)
{
    struct gc_backend_s *bnd;
    struct gc_backend_seed_s seeds[] = {
        { "93.185.107.138",  "cz01" },
        { "185.101.98.180",  "us01" },
        { "176.126.245.114", "uk01" }
    };

    bnd = gc_backend_ping(gc, seeds, sizeof(seeds) / sizeof(seeds[0]));

    if(!bnd) {
        hm_log(LOG_TRACE, &gc->log, "No backend specified");
        return GC_ERROR;
    }

    gc_backend_dump(gc, bnd);

    int sel_idx = gc_backend_choose(bnd);
    if(sel_idx != -1) {
        hm_log(LOG_TRACE, &gc->log, "Selected backend: [%s %s]", seeds[sel_idx].ip,
                                                                 seeds[sel_idx].description);
        sn_initz(ip, (char *)seeds[sel_idx].ip);
        snb_cpy_d(chosen, ip);
    } else {
        hm_log(LOG_TRACE, &gc->log, "No backend selected");
    }

    gc_backend_free(bnd);

    return GC_OK;
}
