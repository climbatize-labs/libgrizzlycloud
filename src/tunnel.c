#include <gc.h>

static struct tunnel_s    *tunnels  = NULL;

static struct conn_client_s *tunnel_client_find(sn port, sn fd)
{
    struct tunnel_s *t;
    struct conn_client_holder_s *holder;

    for(t = tunnels; t != NULL; t = t->next) {
        for(holder = t->server->clients_head; holder != NULL; holder = holder->next) {
            sn_itoa(client_fd, holder->client->base.fd, 8);

            if(sn_cmps(t->port_remote, port) &&
               sn_cmps(client_fd, fd)) {
                return holder->client;
            }
        }
    }

    return NULL;
}

int tunnel_response(struct gc_s *gc, struct proto_s *p, char **argv, int argc)
{
    if(argc != 3) {
        return GC_ERROR;
    }

    sn_initr(port, argv[1], strlen(argv[1]));
    sn_initr(fd,   argv[2], strlen(argv[2]));

    hm_log(LOG_TRACE, &gc->log, "Tunnel response [port:fd] [%.*s:%*s]",
                                sn_p(port),
                                sn_p(fd));

    struct conn_client_s *client = tunnel_client_find(port, fd);
    if(!client) {
        hm_log(LOG_TRACE, &gc->log, "Tunnel client not found");
        return GC_ERROR;
    }

    gen_ev_send(client,
                p->u.message_from.body.s,
                p->u.message_from.body.n);

    return GC_OK;
    /*

    sn_bytes_new(key, p->u.message_from.from_address.n + fd.n);
    sn_bytes_add(key, p->u.message_from.from_address);
    sn_bytes_add(key, fd);

    struct endpoint_s *ep = endpoint_find(&key);

    if(!ep) {
        sn_init(port, argv[1], strlen(argv[1]));
        sn_init(sd,   argv[2], strlen(argv[2]));
        int ret;
        ret = endpoint_add(key, sd, backend_port, remote_port, &ep);
        if(ret != GC_OK) return ret;
    }

    ep_send(ep->client,
            p->u.message_from.body.s,
            p->u.message_from.body.n);
    */
    //sn_bytes_delete(key);
}

static void client_data(struct conn_client_s *client, char *buf, const int len)
{
    struct tunnel_s *tunnel = client->parent->tunnel;

    assert(tunnel);

    // Payload
    sn_initr(payload, (char *)buf, len);

    // Client file descriptor
    char client_fd[8];
    snprintf(client_fd, sizeof(client_fd), "%d", client->base.fd);

    // Message header
    char header[64];
    snprintf(header, sizeof(header), "tunnel_request/%.*s/%s/%.*s",
                                     sn_p(tunnel->port_remote),
                                     client_fd,
                                     sn_p(tunnel->port_local));
    sn_initr(snheader, header, strlen(header));

    hm_log(LOG_DEBUG, client->base.log, "{Tunnel}: header [%.*s]",
                                        snheader.n, snheader.s);

    struct proto_s m = { .type = MESSAGE_TO };
    sn_set(m.u.message_to.to,      tunnel->device);
    sn_set(m.u.message_to.address, tunnel->pid);
    sn_set(m.u.message_to.body,    payload);
    sn_set(m.u.message_to.tp,      snheader);

    packet_send(gc, &m);
}

static int alloc_server(struct gc_s *gc, struct conn_server_s **c, sn port_local)
{
    *c = malloc(sizeof(**c));
    if(!*c) return GC_ERROR;

    memset(*c, 0, sizeof(**c));

    (*c)->loop = gc->loop;
    (*c)->log  = &gc->log;
    (*c)->pool = NULL;
    (*c)->recv = client_data;
    (*c)->host = "0.0.0.0";

    sn_to_char(port, port_local, 32);
    (*c)->port = port;

    int ret;
    ret = async_server(*c);
    if(ret != GC_OK) return ret;

    return GC_OK;
}

int tunnel_add(struct gc_s *gc, struct gc_device_pair_s *pair, sn type)
{
    struct conn_server_s *c = NULL;

    sn_initz(forced, "forced");
    if(!sn_cmps(type, forced)) {
        int ret;
        ret = alloc_server(gc, &c, pair->port_local);
        if(ret != GC_OK) return ret;
    }

    struct tunnel_s *t = malloc(sizeof(*t));
    if(!t) return GC_ERROR;

    memset(t, 0, sizeof(*t));

    snb_cpy_ds(t->cloud,       pair->cloud);
    snb_cpy_ds(t->pid,         pair->pid);
    snb_cpy_ds(t->device,      pair->device);
    snb_cpy_ds(t->port_local,  pair->port_local);
    snb_cpy_ds(t->port_remote, pair->port_remote);
    snb_cpy_ds(t->type,        pair->type);

    // Link tunnel and server
    t->server = c;
    if(c) c->tunnel = t;

    if(tunnels) tunnels->next = t;
    else        tunnels       = t;

    return GC_OK;
}

void tunnel_stop(sn pid)
{
    struct tunnel_s *t, *prev;
    for(t = tunnels; t != NULL; prev = t, t = t->next) {
        if(sn_cmps(t->pid, pid)) {

            if(prev) prev->next = t->next;
            else     tunnels = t->next;

            free(t);

            break;
        }
    }
}

void tunnel_force_stop_all()
{
    struct tunnel_s *t, *del;

    for(t = tunnels; t != NULL; ) {
        if(t->server) async_server_shutdown(t->server);
        del = t;
        t = t->next;
        free(del);
    }
}
