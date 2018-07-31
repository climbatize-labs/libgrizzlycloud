#include <gc.h>

struct gc_module_s module_webcam;

static void client_data(struct gc_gen_client_s *client, char *buf, const int len)
{
    const char *tmpfile = "/tmp/gc_webcam";
    char cmd[128];

    snprintf(cmd, sizeof(cmd), "fswebcam -r 640x480 --jpeg 85 -D 1 %s", tmpfile);
    system(cmd);

    char *bin;
    int nbin = gc_fread(client->base.pool, &bin, tmpfile);

    if(nbin <= 0) return;

    char *buffer = hm_palloc(client->base.pool, sizeof(nbin) + nbin);
    assert(buffer);
    memcpy(buffer, &nbin, sizeof(nbin));
    memcpy(buffer + sizeof(nbin), bin, nbin);

    gc_gen_ev_send(client, buffer, sizeof(nbin) + nbin);

    hm_pfree(client->base.pool, bin);
    hm_pfree(client->base.pool, buffer);
}

static int start(struct gc_s *gc, struct gc_module_s *module)
{
    hm_log(LOG_TRACE, &gc->log, "Starting module [%s]", module->name);
    module->server = hm_palloc(gc->pool, sizeof(*(module->server)));
    if(!module->server) return GC_ERROR;

    memset(module->server, 0, sizeof(*(module->server)));

    module->server->loop = gc->loop;
    module->server->log  = &gc->log;
    module->server->pool = gc->pool;
    module->server->callback.data = client_data;
    module->server->host = "0.0.0.0";

    sn_itoa(port_local, module->port, 32);
    sn_to_char(port, port_local, 32);
    module->server->port = port;

    int ret;
    ret = async_server(module->server, gc, NULL);
    if(ret != GC_OK) {
        hm_pfree(gc->pool, module->server);
        module->server = NULL;
        return ret;
    }

    sn_setr(module->buffer, NULL, 0);

    return GC_OK;
}

static int response(struct gc_s *gc, struct proto_s *p, char **argv, int argc)
{
    gc_fwrite("/tmp/qq", "w", p->u.message_from.body.s, p->u.message_from.body.n);

    return GC_OK;
}

static int request(struct gc_s *gc, struct proto_s *p, char **argv, int argc)
{
    const char *tmpfile = "/tmp/gc_webcam";
    char cmd[128];

    snprintf(cmd, sizeof(cmd), "fswebcam -r 640x480 --jpeg 85 -D 1 %s", tmpfile);
    system(cmd);

    char *bin;
    int nbin = gc_fread(gc->pool, &bin, tmpfile);

    if(nbin < 0) return GC_ERROR;

    char header[64];
    snprintf(header, sizeof(header), "webcam_response");

    struct proto_s m = { .type = MESSAGE_TO };
    sn_set(m.u.message_to.to,      p->u.message_from.from_cloud);
    sn_set(m.u.message_to.address, p->u.message_from.from_address);
    sn_setr(m.u.message_to.body,   bin, nbin);
    sn_setr(m.u.message_to.tp,     header, strlen(header));

    gc_packet_send(gc, &m);

    return GC_OK;
}

static int stop(struct gc_s *gc, struct gc_module_s *module)
{
    if(module && module->server) {
        hm_log(LOG_TRACE, &gc->log, "Stopping module [%s]", module->name);
        async_server_shutdown(module->server);
        module->server = NULL;
    }
}

static int status()
{
    return GC_OK;
}

struct gc_module_s module_webcam = {
    .id      = MOD_WEBCAM,
    .name    = "mod_webcam",
    .start   = start,
    .status  = status,
    .stop    = stop,

    .port    = MOD_WEBCAM_PORT,
    .server  = NULL,
};
