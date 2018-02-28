/*
 *
 * GrizzlyCloud library - simplified VPN alternative for IoT
 * Copyright (C) 2016 - 2017 Filip Pancik
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

int async_client_shutdown(struct conn_client_s *c)
{
    assert(c);
    struct hm_pool_s *p = c->base.pool;

    ev_io_stop(c->base.loop, &c->base.read);
    ev_io_stop(c->base.loop, &c->base.write);

    c->base.flags |= GC_WANT_SHUTDOWN;

    ringbuffer_send_pop_all(&c->base.rb);

    hm_log(LOG_TRACE, c->base.log, "Removing client [%.*s:%d] fd: [%d] alive since: [%s]",
                                   sn_p(c->base.net.ip), c->base.net.port,
                                   c->base.fd, c->base.date);

    int ret;
    ret = gc_fd_close(c->base.fd);
    if(ret != GC_OK) {
        hm_log(LOG_TRACE, c->base.log, "File descriptor %d failed to closed",
                                       c->base.fd);
    }

    if(c->parent) {
        char key[16];
        snprintf(key, sizeof(key), "%d", c->base.fd);
        ht_rem(c->parent->ht, key, strlen(key), p);
    }

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
    c->callback_data(c, buffer, sz);
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

    if(gc_sigterm == 1) return;

    assert(w);
    c = (struct conn_client_s *)w->data;
    fd = w->fd;

    assert(c);

    if(EQFLAG(c->base.flags, GC_WANT_SHUTDOWN)) {
        if(c->callback_error) {
            c->callback_error(c, CL_WANTSHUTDOWN_ERR);
        }
        return;
    }

    sz = recv(fd, c->base.rb.recv.tmp, RB_SLOT_SIZE, 0);

    if(sz > 0) {
        ringbuffer_recv_append(&c->base.rb, sz);

        if(ringbuffer_recv_is_full(&c->base.rb)) {
            ev_io_stop(c->base.loop, &c->base.read);
            if(c->callback_error) {
                c->callback_error(c, CL_READRBFULL_ERR);
            }
            return;
        }

        recv_append(c);

    } else if(sz == 0) {
        ev_io_stop(c->base.loop, &c->base.read);
        async_handle_socket_errno(c->base.log);
        if(c->callback_error) {
            c->callback_error(c, CL_READZERO_ERR);
        }
    } else {
        ev_io_stop(c->base.loop, &c->base.read);
        async_handle_socket_errno(c->base.log);
        if(c->callback_error) {
            c->callback_error(c, CL_READ_ERR);
        }
    }
}

static void async_write(struct ev_loop *loop, ev_io *w, int revents)
{
    (void)revents;
    struct conn_client_s *c;
    int fd;
    int sz;

    if(gc_sigterm == 1) return;

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
        if(c->callback_error) {
            c->callback_error(c, CL_WRITE_ERR);
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

    gc_timestring(client->base.date, sizeof(client->base.date));

    return GC_OK;
}

static int connector_addclient(struct conn_server_s *cs, struct conn_client_s *cc)
{
    char key[8];
    snprintf(key, sizeof(key), "%d", cc->base.fd);
    if(HT_ADD_WA(cs->ht, key, strlen(key), cc, sizeof(cc), cs->pool) != GC_OK) {
        hm_log(LOG_ERR, cs->log, "{Connector}: Cannot add key [%s] to hashtable", key);
        return GC_ERROR;
    }

    hm_log(LOG_NOTICE, cs->log, "{Connector}: adding client %.*s:%d:%d",
                                sn_p(cc->base.net.ip), cc->base.fd, cc->base.net.port);

    return GC_OK;
}

static void server_async_client(struct ev_loop *loop, ev_io *w, int revents)
{
    (void) revents;
    struct sockaddr_storage addr;
    socklen_t sl = sizeof(addr);
    int client;
    struct conn_client_s *cc;
    struct conn_server_s *cs = w->data;

    if(gc_sigterm == 1) return;

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

    ret = gc_fd_setnonblock(client);
    if(ret != GC_OK) {
        hm_log(LOG_TRACE, cs->log, "Failed to set noblock() on fd %d", client);
    }

    ret = gc_fd_setkeepalive(client);
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
    cc->base.gc = cs->gc;
    cc->parent = cs;
    cc->callback_error = client_error;
#ifdef PEER_NAME
    sn_initz(snip, ipstr);
    snb_cpy_ds(cc->base.net.ip, snip);
    cc->base.net.port = pport;
#endif

    if(connector_addclient(cs, cc) != GC_OK) {
        hm_pfree(cs->pool, cc);
        return;
    }

    cc->callback_data = cs->callback_data;
    async_client_accept(cc);
}

int async_server(struct conn_server_s *cs, struct gc_s *gc)
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

    cs->ht = ht_init(cs->pool);
    if(!cs->ht) {
        hm_log(LOG_CRIT, cs->log, "Hashtable failed to initialize");
        return GC_ERROR;
    }

    hm_log(LOG_TRACE, cs->log, "Opening async server on %s:%s fd: %d %p",
                               cs->host, cs->port, cs->fd, cs);

    cs->gc = gc;

    return GC_OK;
}

void async_server_shutdown(struct conn_server_s *s)
{
    assert(s);
    struct hm_pool_s *p = s->pool;

    ev_io_stop(s->loop, &s->listener);
    struct conn_client_s *c;

    int i;
    for(i = 0; i < HT_MAX; i++) {
        if(!s->ht[i]) continue;
        c = (struct conn_client_s *)s->ht[i]->s;
        if(c) async_client_shutdown(c);
    }

    ht_free(s->ht, s->pool);

    hm_pfree(p, s);
}
