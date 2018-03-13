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

static void recv_append(struct gc_s *gc)
{
    struct gc_ringbuffer_s *rb;
    struct gc_gen_client_ssl_s *c = &gc->client;

    rb = &c->base.rb;

    if(rb->recv.target == 0 && rb->recv.len > (int)(2 * sizeof(int))) {
        rb->recv.target = *(int *)(rb->recv.buf);
        gc_swap_memory((void *)&rb->recv.target, sizeof(rb->recv.target));
        rb->recv.target += 4;

        if(rb->recv.target <= 0) {
            c->callback.error(c, GC_PACKETEXPECT_ERR);
            return;
        }
    }

    if(rb->recv.target != 0 && rb->recv.target <= rb->recv.len) {
        c->net.buf = hm_prealloc(gc->pool, c->net.buf, rb->recv.target);
        memcpy(c->net.buf, rb->recv.buf, rb->recv.target);
        c->net.n = rb->recv.target;

        gc_ringbuffer_recv_pop(gc->pool, rb);

        if(c->callback.data) {
            c->callback.data(gc, c->net.buf, c->net.n);
        }
    }
}

void async_client_ssl_shutdown(struct gc_gen_client_ssl_s *c)
{
    assert(c);

    ev_io_stop(c->base.loop, &c->base.read);
    ev_io_stop(c->base.loop, &c->base.write);
    ev_io_stop(c->base.loop, &c->ev_r_handshake);
    ev_io_stop(c->base.loop, &c->ev_w_handshake);
    ev_io_stop(c->base.loop, &c->ev_w_connect);

    c->base.flags |= GC_WANT_SHUTDOWN;

    if(c->ssl) SSL_free(c->ssl);
    if(c->ctx) SSL_CTX_free(c->ctx);

    if(c->net.buf) {
        hm_pfree(c->base.pool, c->net.buf);
    }

    gc_ringbuffer_send_pop_all(c->base.pool, &c->base.rb);

    hm_log(LOG_TRACE, c->base.log, "Removing client [%.*s:%d] fd: [%d] alive since: [%s]",
                                   sn_p(c->base.net.ip), c->base.net.port,
                                   c->base.fd, c->base.date);

    int ret;
    ret = gc_fd_close(c->base.fd);
    if(ret != GC_OK) {
        hm_log(LOG_TRACE, c->base.log, "File dscriptor %d failed to close",
                                       c->base.fd);
    }
}

static void async_read_ssl(struct ev_loop *loop, ev_io *w, int revents)
{
    (void) revents;
    int t = 0;
    struct gc_s *gc = (struct gc_s *)w->data;
    struct gc_gen_client_ssl_s *c = &gc->client;

    (void)revents;

    if(gc_sigterm == 1) return;

    if(EQFLAG(c->base.flags, GC_WANT_SHUTDOWN)) {
        if(c->callback.error) {
            c->callback.error(c, GC_WANTSHUTDOWN_ERR);
        }
        return;
    }

    t = SSL_read(c->ssl, c->base.rb.recv.tmp, RB_SLOT_SIZE);

    /*
       EAGAIN or EWOULDBLOCK The socket is marked nonblocking and the receive operation would block, or a receive timeout had been set and the timeout expired before data was received.
    if(t == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) goto again;
    */

    if(t > 0) {
        gc_ringbuffer_recv_append(c->base.pool, &c->base.rb, t);
        /*
        FIXME: add MAX so we don't spend all memory
        if(gc_ringbuffer_recv_is_full(&c->rb)) {
        ev_io_stop(c->loop, &c->read);
        if(c->callback_error) {
        c->callback_error(c, GC_READRBFULL_ERR);
        }
        return;
        }
        */

        recv_append(gc);

    } else if(t == 0) {
        async_handle_socket_errno(c->base.log);
        if(c->callback.error) {
            c->callback.error(c, GC_READZERO_ERR);
        }
     } else if(t == -1 &&
        (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
        hm_log(LOG_TRACE, c->base.log, "Socket read EAGAIN|EWOULDBLOCK|EINTR");
        async_handle_socket_errno(c->base.log);
    } else {
        async_handle_socket_errno(c->base.log);
        if(c->callback.error) {
            c->callback.error(c, GC_READ_ERR);
        }
    }
}

static void async_write_ssl(struct ev_loop *loop, ev_io *w, int revents)
{
    (void)revents;
    int t;
    struct gc_s *gc = (struct gc_s *)w->data;
    struct gc_gen_client_ssl_s *c = &gc->client;
    int sz;

    if(gc_sigterm == 1) return;

    if(EQFLAG(c->base.flags, GC_WANT_SHUTDOWN)) {
        if(c->callback.error) {
            c->callback.error(c, GC_WANTSHUTDOWN_ERR);
        }
        return;
    }

    char *next = gc_ringbuffer_send_next(&c->base.rb, &sz);

    if(sz == 0) {
        ev_io_stop(loop, &c->base.write);
        return;
    }

    t = SSL_write(c->ssl, next, sz);
    /*
       EAGAIN or EWOULDBLOCK The socket is marked nonblocking and the receive operation would block, or a receive timeout had been set and the timeout expired before data was received.
     */
    if(t == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) abort(); //goto again;

    if(t > 0) {
        gc_ringbuffer_send_skip(c->base.pool, &c->base.rb, t);
        if(gc_ringbuffer_send_is_empty(&c->base.rb)) {
            ev_io_stop(loop, &c->base.write);
            if(c->callback.terminate) {
                c->callback.terminate(c, 0);
                c->callback.terminate = NULL;
            }
        }
    } else {
        if(c->callback.terminate) {
            c->callback.terminate(c, GC_WRITE_ERR);
            c->callback.terminate = NULL;
        } else {
            async_handle_socket_errno(c->base.log);
            if(c->callback.error) {
                c->callback.error(c, GC_WRITE_ERR);
            }
        }
    }
}

SSL_CTX *make_ctx()
{
    SSL_CTX *ctx;

    long ssloptions = SSL_OP_NO_SSLv2 | SSL_OP_ALL |
        SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION;

#ifdef SSL_OP_NO_COMPRESSION
    ssloptions |= SSL_OP_NO_COMPRESSION;
#endif

    ctx = SSL_CTX_new(TLSv1_client_method());

    SSL_CTX_set_options(ctx, ssloptions);

    return ctx;
}

static void start_handshake(struct gc_s *gc, int err)
{
    struct gc_gen_client_ssl_s *c = &gc->client;
    ev_io_stop(c->base.loop, &c->base.read);
    ev_io_stop(c->base.loop, &c->base.write);

    c->base.flags &= ~GC_HANDSHAKED;

    if(err == SSL_ERROR_WANT_READ) {
        ev_io_start(c->base.loop, &c->ev_r_handshake);
    } else if(err == SSL_ERROR_WANT_WRITE) {
        ev_io_start(c->base.loop, &c->ev_w_handshake);
    }
}

static void handle_connect(struct ev_loop *loop, ev_io *w, int revents)
{
    (void) revents;
    int t;
    struct gc_s *gc = (struct gc_s *)w->data;
    struct gc_gen_client_ssl_s *c = &gc->client;
    t = connect(c->base.fd, (struct sockaddr *)&(c->servaddr), sizeof(c->servaddr));
    if(!t || errno == EISCONN || !errno) {
        ev_io_stop(loop, &c->ev_w_connect);

        start_handshake(gc, SSL_ERROR_WANT_WRITE);
    }
    else if(errno == EINPROGRESS || errno == EINTR || errno == EALREADY) {
        /* do nothing, we'll get phoned home again... */
    }
    else {
        printf("Error: {backend-connect}");
    }
}

static void end_handshake(struct gc_gen_client_ssl_s *c)
{
    ev_io_stop(c->base.loop, &c->ev_r_handshake);
    ev_io_stop(c->base.loop, &c->ev_w_handshake);

    /* Disable renegotiation (CVE-2009-3555) */
    if(c->ssl->s3) {
        c->ssl->s3->flags |= SSL3_FLAGS_NO_RENEGOTIATE_CIPHERS;
    }

    c->base.flags |= GC_HANDSHAKED;

    ev_io_start(c->base.loop, &c->base.read);

    if(!gc_ringbuffer_send_is_empty(&c->base.rb)) {
        ev_io_start(c->base.loop, &c->base.write);
    }

    struct gc_s *gc = c->base.gc;
    assert(gc);
    if(gc && gc->callback.state_changed)
        gc->callback.state_changed(gc, GC_HANDSHAKE_SUCCESS);
}

static void client_handshake(struct ev_loop *loop, ev_io *w, int revents)
{
    (void) revents;
    int t;
    struct gc_s *gc = (struct gc_s *)w->data;
    struct gc_gen_client_ssl_s *c = &gc->client;

    t = SSL_do_handshake(c->ssl);
    if(t == 1) {
        end_handshake(c);
    }
    else {
        int err = SSL_get_error(c->ssl, t);
        if(err == SSL_ERROR_WANT_READ) {
            ev_io_stop(loop, &c->ev_w_handshake);
            ev_io_start(loop, &c->ev_r_handshake);
        } else if(err == SSL_ERROR_WANT_WRITE) {
            ev_io_stop(loop, &c->ev_r_handshake);
            ev_io_start(loop, &c->ev_w_handshake);
        } else if(err == SSL_ERROR_ZERO_RETURN) {
            printf("Connection closed (in handshake)\n");
        } else {
            printf("Unexpected SSL error (in handshake): %d\n", err);
        }
    }
}

static int hostname_to_ip(char *hostname, char *ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    if((he = gethostbyname(hostname)) == NULL) {
        return 1;
    }

    addr_list = (struct in_addr **)he->h_addr_list;

    for(i = 0; addr_list[i] != NULL; i++) {
        strcpy(ip, inet_ntoa(*addr_list[i]));
        return 0;
    }

    return 1;
}

int async_client_ssl(struct gc_s *gc)
{
    char hostname[128];
    char ip[32];
    SSL_CTX *ctx;
    SSL *ssl;
    struct gc_gen_client_ssl_s *client = &gc->client;

    client->base.active = 0;

    ctx = make_ctx();

    ssl = SSL_new(ctx);

    snprintf(hostname, sizeof(hostname), "%.*s", sn_p(client->base.net.ip));

    hostname_to_ip(hostname, ip);

    memset(&client->servaddr, 0, sizeof(client->servaddr));
    client->servaddr.sin_family = AF_INET;
    client->servaddr.sin_addr.s_addr = inet_addr(ip);
    client->servaddr.sin_port = htons(client->base.net.port);

    client->base.fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if(client->base.fd == -1) {
        hm_log(LOG_CRIT, client->base.log, "Socket() ssl initialization failed");
        return GC_ERROR;
    }

    int ret = gc_fd_setkeepalive(client->base.fd);
    if(ret != GC_OK) {
        hm_log(LOG_TRACE, client->base.log, "Failed to set keepalive() on fd %d", client->base.fd);
    }

    long mode = SSL_MODE_ENABLE_PARTIAL_WRITE;
#ifdef SSL_MODE_RELEASE_BUFFERS
    mode |= SSL_MODE_RELEASE_BUFFERS;
#endif
    SSL_set_mode(ssl, mode);

    SSL_set_connect_state(ssl);
    SSL_set_fd(ssl, client->base.fd);

    client->ssl = ssl;
    client->ctx = ctx;

    client->base.gc = gc;
    client->base.read.data = gc;
    client->base.write.data = gc;
    client->ev_r_handshake.data = gc;
    client->ev_w_handshake.data = gc;
    client->ev_w_connect.data = gc;

    ev_io_init(&client->ev_r_handshake, client_handshake, client->base.fd, EV_READ);
    ev_io_init(&client->ev_w_handshake, client_handshake, client->base.fd, EV_WRITE);

    ev_io_init(&client->ev_w_connect, handle_connect, client->base.fd, EV_WRITE);

    ev_io_init(&client->base.write, async_write_ssl, client->base.fd, EV_WRITE);
    ev_io_init(&client->base.read, async_read_ssl, client->base.fd, EV_READ);

    gc_timestring(client->base.date, sizeof(client->base.date));

    ev_io_start(client->base.loop, &client->base.read);

    if(connect(client->base.fd, (struct sockaddr *)&client->servaddr, sizeof(client->servaddr)) != -1
       && errno != EINPROGRESS && errno != EINTR) {

        async_client_ssl_shutdown(client);
        printf("Connect() errno: %d\n", errno);
        return GC_ERROR;
    }

    ev_io_start(client->base.loop, &client->ev_w_connect);

    client->base.active = 1;

    return GC_OK;
}

static void recv_append_client(struct gc_gen_client_s *c)
{
    int sz;
    char *next;

    next = gc_ringbuffer_recv_read(&c->base.rb, &sz);
    c->callback.data(c, next, sz);
    gc_ringbuffer_recv_pop(c->base.pool, &c->base.rb);
}

static void async_read(struct ev_loop *loop, ev_io *w, int revents)
{
    (void) revents;
    int sz;
    struct gc_gen_client_s *c;
    int fd;

    if(gc_sigterm == 1) return;

    assert(w);
    c = (struct gc_gen_client_s *)w->data;
    fd = w->fd;

    assert(c);

    if(EQFLAG(c->base.flags, GC_WANT_SHUTDOWN)) {
        if(c->callback.error) {
            c->callback.error(c, GC_WANTSHUTDOWN_ERR);
        }

        return;
    }

    sz = recv(fd, c->base.rb.recv.tmp, RB_SLOT_SIZE, 0);

    hm_log(LOG_TRACE, c->base.log, "Received %d bytes from fd %d", sz, fd);

    if(sz > 0) {
        gc_ringbuffer_recv_append(c->base.pool, &c->base.rb, sz);

        if(gc_ringbuffer_recv_is_full(&c->base.rb)) {
            ev_io_stop(c->base.loop, &c->base.read);
            if(c->callback.error) {
                c->callback.error(c, GC_READRBFULL_ERR);
            }
            return;
        }

        recv_append_client(c);

    } else if(sz == 0) {
        async_handle_socket_errno(c->base.log);
        if(c->callback.error) {
            c->callback.error(c, GC_READZERO_ERR);
        }
     } else if(sz == -1 &&
        (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
        hm_log(LOG_TRACE, c->base.log, "Socket read EAGAIN|EWOULDBLOCK|EINTR");
        async_handle_socket_errno(c->base.log);
    } else {
        async_handle_socket_errno(c->base.log);
        if(c->callback.error) {
            c->callback.error(c, GC_READ_ERR);
        }
    }
}

static void async_write(struct ev_loop *loop, ev_io *w, int revents)
{
    (void)revents;
    struct gc_gen_client_s *c;
    int fd;
    int sz;

    if(gc_sigterm == 1) return;

    assert(w);
    c = (struct gc_gen_client_s *)w->data;
    fd = w->fd;

    assert(c);

    if(gc_ringbuffer_send_is_empty(&c->base.rb)) {
        ev_io_stop(loop, &c->base.write);
        return;
    }

    char *next = gc_ringbuffer_send_next(&c->base.rb, &sz);

    if(sz == 0) {
        ev_io_stop(loop, &c->base.write);
        return;
    }

    sz = send(fd, next, sz, MSG_NOSIGNAL);

    hm_log(LOG_TRACE, c->base.log, "%d bytes sent to fd %d", sz, fd);

    if(sz > 0) {
        gc_ringbuffer_send_skip(c->base.pool, &c->base.rb, sz);
        if(gc_ringbuffer_send_is_empty(&c->base.rb)) {
            ev_io_stop(loop, &c->base.write);
        }
    } else {
        async_handle_socket_errno(c->base.log);
        ev_io_stop(loop, &c->base.write);
        if(c->callback.error) {
            c->callback.error(c, GC_WRITE_ERR);
        }
    }
}

int async_client(struct gc_gen_client_s *client)
{
    struct sockaddr_in servaddr;
    char ip[32];

    assert(client);

    client->base.fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if(client->base.fd == -1) {
        hm_log(LOG_ERR, client->base.log, "{Connector}: client init socket error: %d", errno);
        client->callback.error(client, GC_SOCKET_ERR);
        return GC_ERROR;
    }

    snprintf(ip, sizeof(ip), "%.*s", sn_p(client->base.net.ip));

    int ret = gc_fd_setkeepalive(client->base.fd);
    if(ret != GC_OK) {
        hm_log(LOG_TRACE, client->base.log, "Failed to set keepalive() on fd %d", client->base.fd);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(client->base.net.port);

    ev_io_init(&client->base.write, async_write, client->base.fd, EV_WRITE);
    ev_io_init(&client->base.read, async_read, client->base.fd, EV_READ);

    client->base.read.data = client;
    client->base.write.data = client;
    gc_timestring(client->base.date, sizeof(client->base.date));

    ev_io_start(client->base.loop, &client->base.read);
    if(connect(client->base.fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != -1
       && errno != EINPROGRESS) {
        async_client_shutdown(client);
        hm_log(LOG_ERR, client->base.log, "{Connector}: connect() errno: %d", errno);
        return GC_ERROR;
    }

    hm_log(LOG_DEBUG, client->base.log, "Adding endpoint TCP client [%.*s:%d] fd: [%d]",
                                        sn_p(client->base.net.ip), client->base.net.port,
                                        client->base.fd);
    return GC_OK;
}
