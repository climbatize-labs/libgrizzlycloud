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
#ifndef GC_ASYNCSERVER_H_
#define GC_ASYNCSERVER_H_

#define DEFAULT_BACKLOG 8

enum flags_e {
    GC_WANT_SHUTDOWN = (1 << 0),
    GC_HANDSHAKED    = (1 << 1),
};

enum clerr_e {
    CL_NOERROR = 1,
    CL_HANGTIMEOUT_ERR,
    CL_WANTSHUTDOWN_ERR,
    CL_READRBFULL_ERR,
    CL_READZERO_ERR,
    CL_READ_ERR,
    CL_WRITE_ERR,
    CL_BUFFERFULL_ERR,
    CL_PACKETLEN_ERR,
    CL_PACKETEXPECT_ERR,
    CL_SERVERSHUTDOWN_ERR,
    CL_SOCKET_ERR
};

enum gcerr_e {
    GC_NOERROR,
    GC_HANGTIMEOUT_ERR,
    GC_WANTSHUTDOWN_ERR,
    GC_READRBFULL_ERR,
    GC_READZERO_ERR,
    GC_READ_ERR,
    GC_WRITE_ERR,
    GC_BUFFERFULL_ERR,
    GC_PACKETLEN_ERR,
    GC_PACKETEXPECT_ERR,
    GC_SERVERSHUTDOWN_ERR,
    GC_SOCKET_ERR,
    GC_PONG_ERR,
};

struct client_ssl_s;
struct gc_tunnel_s *tunnel;
struct conn_client_s;

struct conn_server_s {
    struct ev_loop *loop;
    struct hm_pool_s *pool;
    struct hm_log_s *log;

    struct gc_tunnel_s *tunnel;

    struct ev_io listener;
    int fd;

    const char *host;
    const char *port;

    struct ht_s **ht;

    void (*callback_data)(struct conn_client_s *data, char *buf, const int len);
    void (*shutdown)();

    struct gc_s *gc;
};

struct client_s {
    struct ev_loop *loop;
    struct hm_pool_s *pool;
    struct hm_log_s *log;

    struct ev_io read;
    struct ev_io write;

    int fd;

    struct gc_ringbuffer_s rb;

    enum flags_e flags;

    struct {
        snb ip;
        int port;
    } net;

    char date[32];

    struct gc_s *gc;
};

struct conn_client_s {
    struct client_s base;

    char net_buf[RB_SLOT_SIZE];
    int net_nbuf;
    int net_expect;

    void (*callback_data)(struct conn_client_s *client, char *buf, int len);
    void (*callback_error)(struct conn_client_s *client, enum clerr_e error);

    struct conn_server_s *parent;
    struct conn_client_holder_s *holder;
};

struct client_ssl_s {
    struct client_s base;
    struct ev_timer ping, pong;

    struct sockaddr_in servaddr;

    char *net_buf;
    int net_nbuf;
    int net_expect;

    SSL *ssl;
    SSL_CTX *ctx;

    struct ev_io ev_w_connect;
    struct ev_io ev_r_handshake;
    struct ev_io ev_w_handshake;

    void (*callback_data)(struct gc_s *gc, const void *buffer, const int nbuffer);
    void (*callback_error)(struct client_ssl_s *client, enum gcerr_e error);

    void (*terminate_cb)(struct client_ssl_s *client, int error);

    void (*connected)(struct client_ssl_s *client);
};

int async_server(struct conn_server_s *cs, struct gc_s *gc);
void async_server_shutdown(struct conn_server_s *cs);

int async_client(struct conn_client_s *client);
int async_client_shutdown(struct conn_client_s *c);

int async_client_ssl_shutdown(struct client_ssl_s *c);
int async_client_ssl(struct gc_s *gc);

void gc_ev_send(struct client_ssl_s *client, char *buf, const int len);
void gc_gen_ev_send(struct conn_client_s *client, char *buf, const int len);

void async_handle_socket_errno(struct hm_log_s *l);

#endif
