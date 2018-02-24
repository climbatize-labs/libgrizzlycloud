#include <gc.h>

int async_client_shutdown(struct conn_client_s *c)
{
    assert(c);

    ev_io_stop(c->base.loop, &c->base.read);
    ev_io_stop(c->base.loop, &c->base.write);

    //c->want_shutdown = 1;
    c->base.flags |= GC_WANT_SHUTDOWN;

    ringbuffer_send_pop_all(&c->base.rb);

    hm_log(LOG_TRACE, c->base.log, "Removing client [%.*s:%d] fd: [%d] alive since: [%s]",
                                   sn_p(c->base.net.ip), c->base.net.port,
                                   c->base.fd, c->base.date);

    int ret;
    ret = fd_close(c->base.fd);
    if(ret != GC_OK) {
        hm_log(LOG_TRACE, c->base.log, "File descriptor %d failed to closed",
                                  c->base.fd);
    }

    struct hm_pool_s *p = c->base.pool;
    hm_pfree(p, c);

    return GC_OK;
}

static void client_error(struct conn_client_s *c, enum clerr_e err)
{
    hm_log(LOG_TRACE, c->base.log, "Client error %d on fd %d",
                                   err, c->base.fd);
    async_client_shutdown(c);
}

inline static void recv_append(struct conn_client_s *c)
{
    int sz;
    char *buffer;

    assert(c);

    buffer = ringbuffer_recv_read(&c->base.rb, &sz);
    c->recv(c, buffer, sz);
    ringbuffer_recv_pop(&c->base.rb);
}

void async_handle_socket_errno(struct hm_log_s *l)
{
    if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
        hm_log(LOG_ERR, l, "Socket errno %d", errno);
        return;
    }

    if(errno == ECONNRESET) {
        hm_log(LOG_ERR, l, "Connection reset by peer");
    } else if(errno == ETIMEDOUT) {
        hm_log(LOG_ERR, l, "Connection to backend timed out");
    } else if(errno == EPIPE) {
        hm_log(LOG_ERR, l, "Broken pipe to backend (EPIPE)");
    } else {
        hm_log(LOG_ERR, l, "Errno: %d", errno);
    }
}

static void async_read(struct ev_loop *loop, ev_io *w, int revents)
{
    (void)revents;
    int sz;
    struct conn_client_s *c;
    int fd;

    assert(w);
    c = (struct conn_client_s *)w->data;
    fd = w->fd;

    assert(c);

    if(EQFLAG(c->base.flags, GC_WANT_SHUTDOWN)) {
        if(c->error_callback) {
            c->error_callback(c, CL_WANTSHUTDOWN_ERR);
        }
        return;
    }

    sz = recv(fd, c->base.rb.recv.tmp, RB_SLOT_SIZE, 0);

    if(sz > 0) {
        ringbuffer_recv_append(&c->base.rb, sz);

        if(ringbuffer_recv_is_full(&c->base.rb)) {
            ev_io_stop(c->base.loop, &c->base.read);
            if(c->error_callback) {
                c->error_callback(c, CL_READRBFULL_ERR);
            }
            return;
        }

        recv_append(c);

    } else if(sz == 0) {
        ev_io_stop(c->base.loop, &c->base.read);
        async_handle_socket_errno(c->base.log);
        if(c->error_callback) {
            c->error_callback(c, CL_READZERO_ERR);
        }
    } else {
        ev_io_stop(c->base.loop, &c->base.read);
        async_handle_socket_errno(c->base.log);
        if(c->error_callback) {
            c->error_callback(c, CL_READ_ERR);
        }
    }
}

static void async_write(struct ev_loop *loop, ev_io *w, int revents)
{
    (void)revents;
    struct conn_client_s *c;
    int fd;
    int sz;

    assert(w);
    c = (struct conn_client_s *)w->data;
    fd = w->fd;

    assert(c);

    if(ringbuffer_send_is_empty(&c->base.rb)) {
        ev_io_stop(loop, &c->base.write);
        return;
    }

    char *next = ringbuffer_send_next(&c->base.rb, &sz);

    if(sz == 0) {
        ev_io_stop(loop, &c->base.write);
        return;
    }

    sz = send(fd, next, sz, MSG_NOSIGNAL);
    if(sz > 0) {
        ringbuffer_send_skip(&c->base.rb, sz);
        if(ringbuffer_send_is_empty(&c->base.rb)) {
            ev_io_stop(loop, &c->base.write);
        }
    } else {
        async_handle_socket_errno(c->base.log);
        if(c->error_callback) {
            c->error_callback(c, CL_WRITE_ERR);
        }
    }
}

static int async_client_accept(struct conn_client_s *client)
{
    ev_io_init(&client->base.write, async_write, client->base.fd, EV_WRITE);
    ev_io_init(&client->base.read, async_read, client->base.fd, EV_READ);

    client->base.read.data = client;
    client->base.write.data = client;

    ev_io_start(client->base.loop, &client->base.read);

    timestring(client->base.date, sizeof(client->base.date));

    return GC_OK;
}

static void *connector_addclient(struct conn_server_s *cs, struct conn_client_s *cc)
{
    struct conn_client_holder_s *cch;

    cch = hm_palloc(cs->pool, sizeof(*cch));

    if(cch == NULL) {
        hm_log(LOG_ERR, cs->log, "{Connector}: Memory allocation failed");
        return NULL;
    }

    memset(cch, 0, sizeof(*cch));

    cch->client = cc;
    cch->next = cs->clients_head;

    cs->clients_head = cch;
    cs->clients++;

    cc->holder = cch;

    hm_log(LOG_NOTICE, cs->log, "{Connector}: adding client %.*s:%d:%d, total: %d",
                                sn_p(cc->base.net.ip), cc->base.fd, cc->base.net.port,
                                cs->clients);

    return cch;
}

static void server_async_client(struct ev_loop *loop, ev_io *w, int revents)
{
    (void) revents;
    struct sockaddr_storage addr;
    socklen_t sl = sizeof(addr);
    int client;
    struct conn_client_s *cc;
    struct conn_server_s *cs = w->data;

    assert(cs);

    client = accept(w->fd, (struct sockaddr *) &addr, &sl);
    if(client == -1) {
        switch (errno) {
            case EMFILE:
                hm_log(LOG_ERR, cs->log, "Accept() failed; too many open files for this process");
                break;

            case ENFILE:
                hm_log(LOG_ERR, cs->log, "Accept() failed; too many open files for this system");
                break;

            default:
                assert(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN);
                break;
        }
        return;
    }

    int flag = 1;
    int ret = setsockopt(client, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag));
    if(ret != GC_OK) {
        hm_log(LOG_ERR, cs->log, "Couldn't setsockopt on client (TCP_NODELAY)");
    }
#ifdef TCP_CWND
    int cwnd = 10;
    ret = setsockopt(client, IPPROTO_TCP, TCP_CWND, &cwnd, sizeof(cwnd));
    if(ret != GC_OK) {
        hm_log(LOG_ERR, cs->log, "Couldn't setsockopt on client (TCP_CWND)");
    }
#endif

    ret = fd_setnonblock(client);
    if(ret != GC_OK) {
        hm_log(LOG_TRACE, cs->log, "Failed to set noblock() on fd %d", client);
    }

    ret = fd_setkeepalive(client);
    if(ret != GC_OK) {
        hm_log(LOG_TRACE, cs->log, "Failed to set keepalive() on fd %d", client);
    }

#define PEER_NAME
#ifdef PEER_NAME
    socklen_t len;
    struct sockaddr_storage paddr;
    char ipstr[INET6_ADDRSTRLEN];
    int pport, peer;

    len = sizeof(paddr);
    peer = getpeername(client, (struct sockaddr*)&paddr, &len);

    // deal with both IPv4 and IPv6:
    if(paddr.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)&paddr;
        pport = ntohs(s->sin_port);
        inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof(ipstr));
    } else { // AF_INET6
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&paddr;
        pport = ntohs(s->sin6_port);
        inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof(ipstr));
    }

    if(peer == -1) {
        hm_log(LOG_WARNING, cs->log, "{Connector}: Couldn't retrieve peer name");
    }
#endif

    cc = hm_palloc(cs->pool, sizeof(struct conn_client_s));
    if(cc == NULL) {
        hm_log(LOG_WARNING, cs->log, "{Connector}: Memory allocation failed");
        return;
    }

    memset(cc, 0, sizeof(struct conn_client_s));

    cc->base.loop = loop;
    cc->base.fd = client;
    cc->base.pool = cs->pool;
    cc->base.log = cs->log;
    cc->base.read.data = cc;
    cc->base.write.data = cc;
    cc->parent = cs;
    cc->error_callback = client_error;
#ifdef PEER_NAME
    sn_initz(snip, ipstr);
    snb_cpy_ds(cc->base.net.ip, snip);
    cc->base.net.port = pport;
#endif

    if(connector_addclient(cs, cc) == NULL) {
        hm_pfree(cs->pool, cc);
        return;
    }

    cc->recv = cs->recv;
    async_client_accept(cc);
}

int async_server(struct conn_server_s *cs)
{
    struct addrinfo *ai, hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    const int gai_err = getaddrinfo(cs->host, cs->port, &hints, &ai);

    if(gai_err != GC_OK) {
        hm_log(LOG_CRIT, cs->log, "Server get address info failed with [%s]", gai_strerror(gai_err));
        return GC_ERROR;
    }

    cs->fd = socket(ai->ai_family, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if(cs->fd == -1) {
        hm_log(LOG_CRIT, cs->log, "Server socket() initialization failed");
        return GC_ERROR;
    }

#ifdef SO_REUSEADDR
    int t = 1;
    setsockopt(cs->fd, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(int));
#endif
#ifdef SO_REUSEPORT
    setsockopt(cs->fd, SOL_SOCKET, SO_REUSEPORT, &t, sizeof(int));
#endif

    if(bind(cs->fd, ai->ai_addr, ai->ai_addrlen)) {
        hm_log(LOG_CRIT, cs->log, "Server bind() failed");
        return GC_ERROR;
    }

#if TCP_DEFER_ACCEPT
    int timeout = 1;
    setsockopt(cs->fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &timeout, sizeof(int));
#endif

    freeaddrinfo(ai);
    listen(cs->fd, DEFAULT_BACKLOG);

    ev_io_init(&cs->listener, server_async_client, cs->fd, EV_READ);
    cs->listener.data = cs;
    ev_io_start(cs->loop, &cs->listener);

    hm_log(LOG_TRACE, cs->log, "Opening async server on %s:%s fd: %d %p",
                               cs->host, cs->port, cs->fd, cs);

    return GC_OK;
}

void async_server_shutdown(struct conn_server_s *s)
{
    assert(s);
    struct hm_pool_s *p = s->pool;

    ev_io_stop(s->loop, &s->listener);
    struct conn_client_holder_s *c;

    for(c = s->clients_head; c != NULL; c = c->next) {
        async_client_shutdown(c->client);
    }

    hm_pfree(p, s->clients_head);

    hm_pfree(p, s);
}
